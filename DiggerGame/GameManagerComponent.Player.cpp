
#include "GameManagerComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "GridMovementComponent.h"
#include "GameActorComponent.h"
#include "EventBus.h"
#include "ServiceLocator.h"
#include "GameSounds.h"

#include <algorithm>
#include <limits>
#include <memory>

//Player Spawning/Dying/Despawning

void digger::GameManagerComponent::SpawnDiggerPlayer(
	int playerIndex,
	const glm::ivec2& spawn,
	const std::string& textureFile,
	PlayerRole role)
{
	if (playerIndex < 0 || playerIndex >= 2)
		return;

	if (!m_PlayerAlive[static_cast<size_t>(playerIndex)])
		return;

	auto player = std::make_unique<dae::GameObject>();
	auto* playerPtr = player.get();

	player->AddComponent<dae::TransformComponent>(playerPtr);

	auto* movement = player->AddComponent<GridMovementComponent>(
		playerPtr,
		this,
		m_TileSize,
		160.f,
		m_MapOffset);

	movement->SetGridPosition(spawn);

	// Versus enemy should not dig dirt.
	if (role == PlayerRole::VersusEnemy)
	{
		movement->SetCanDigDirt(false);
		movement->SetCanEnterDirt(false);
	}

	player->AddComponent<dae::RenderComponent>(
		playerPtr,
		dae::ResourceManager::GetInstance().LoadTexture(textureFile));

	auto* actor = player->AddComponent<dae::GameActorComponent>(
		playerPtr,
		4,
		0,
		playerIndex + 1);

	dae::EventBus::GetInstance().GetSubject().AddObserver(actor);

	if (playerIndex >= static_cast<int>(m_Players.size()))
		m_Players.resize(static_cast<size_t>(playerIndex + 1));

	m_Players[static_cast<size_t>(playerIndex)] =
		PlayerRuntime{
			playerPtr,
			actor,
			spawn,
			m_PlayerLives[static_cast<size_t>(playerIndex)],
			m_PlayerAlive[static_cast<size_t>(playerIndex)],
			false,
			0.f,
			0.f,
			role
	};

	m_LevelObjects.push_back(playerPtr);
	m_Scene->Add(std::move(player));
}


void digger::GameManagerComponent::RemovePlayerObservers()
{
	for (auto& player : m_Players)
	{
		if (player.actor)
		{
			dae::EventBus::GetInstance().GetSubject().RemoveObserver(player.actor);
			player.actor = nullptr;
		}
	}
}





//Player Position Getters


dae::GameObject* digger::GameManagerComponent::GetPlayer(int index) const
{
	if (index < 0 || index >= static_cast<int>(m_Players.size()))
		return nullptr;

	return m_Players[static_cast<size_t>(index)].object;
}

glm::ivec2 digger::GameManagerComponent::GetPlayerGridPosition(int index) const
{
	auto* player = GetPlayer(index);
	if (!player)
		return {};

	if (auto* movement = player->GetComponent<GridMovementComponent>())
		return movement->GetGridPosition();

	return {};
}


bool digger::GameManagerComponent::IsPlayerAtGridPosition(const glm::ivec2& pos) const
{
	return GetPlayerGridPosition() == pos;
}

glm::ivec2 digger::GameManagerComponent::GetClosestAlivePlayerGridPosition(
	const glm::ivec2& fromGrid) const
{
	float bestDistanceSq = std::numeric_limits<float>::max();
	int bestIndex = -1;

	for (int i = 0; i < static_cast<int>(m_Players.size()); ++i)
	{
		const auto& player = m_Players[static_cast<size_t>(i)];

		if (!player.alive || player.dying || !player.object)
			continue;

		const glm::ivec2 playerGrid = GetPlayerGridPosition(i);
		const glm::ivec2 diff = playerGrid - fromGrid;

		const float distSq =
			static_cast<float>(diff.x * diff.x + diff.y * diff.y);

		if (distSq < bestDistanceSq)
		{
			bestDistanceSq = distSq;
			bestIndex = i;
		}
	}

	if (bestIndex == -1)
		return {};

	return GetPlayerGridPosition(bestIndex);
}

int digger::GameManagerComponent::GetPlayerIndexAtWorldPosition(
	const glm::vec2& worldPos,
	float radius,
	std::optional<PlayerRole> requiredRole) const
{
	const float radiusSq = radius * radius;

	for (int i = 0; i < static_cast<int>(m_Players.size()); ++i)
	{
		const auto& player = m_Players[static_cast<size_t>(i)];

		if (!player.alive || player.dying || !player.object)
			continue;

		if (requiredRole.has_value() && player.role != requiredRole.value())
			continue;

		const glm::vec2 playerPos = GetPlayerWorldPosition(i);
		const glm::vec2 diff = playerPos - worldPos;

		if ((diff.x * diff.x + diff.y * diff.y) <= radiusSq)
			return i;
	}

	return -1;
}

int digger::GameManagerComponent::GetPlayerIndexAtWorldPosition(
	const glm::vec2& worldPos,
	float radius) const
{
	return GetPlayerIndexAtWorldPosition(worldPos, radius, std::nullopt);
}

int digger::GameManagerComponent::GetDiggerPlayerIndexAtWorldPosition(
	const glm::vec2& worldPos,
	float radius) const
{
	return GetPlayerIndexAtWorldPosition(worldPos, radius, PlayerRole::Digger);
}


int digger::GameManagerComponent::GetPlayerIndexAtGridPosition(
	const glm::ivec2& pos) const
{
	for (int i = 0; i < static_cast<int>(m_Players.size()); ++i)
	{
		if (!IsPlayerAlive(i))
			continue;

		if (GetPlayerGridPosition(i) == pos)
			return i;
	}

	return -1;
}

glm::vec2 digger::GameManagerComponent::GetPlayerWorldPosition(int index) const
{
	auto* player = GetPlayer(index);
	if (!player)
		return {};

	if (auto* movement = player->GetComponent<GridMovementComponent>())
		return movement->GetWorldPosition() + m_PlayerCenterOffset;

	if (auto* tr = player->GetComponent<dae::TransformComponent>())
	{
		const auto& pos = tr->GetWorldPosition();
		return { pos.x + m_PlayerCenterOffset.x, pos.y + m_PlayerCenterOffset.y };
	}

	return {};
}





//Player Damage/Collisions/Health


void digger::GameManagerComponent::DamagePlayer(int playerIndex)
{
	if (m_ScreenState != GameScreenState::Playing)
		return;

	if (m_DeathSequenceActive)
		return;

	if (playerIndex < 0 ||
		playerIndex >= static_cast<int>(m_Players.size()))
	{
		return;
	}

	auto& player = m_Players[static_cast<size_t>(playerIndex)];

	if (!player.alive || player.dying || !player.object)
		return;

	if (player.damageCooldown > 0.f)
		return;

	player.damageCooldown = m_DamageCooldownDuration;

	//mark dying
	// This prevents a second enemy/moneybag collision from starting another death sequence
	player.dying = true;

	--player.lives;
	m_PlayerLives[static_cast<size_t>(playerIndex)] = player.lives;
	UpdateHUD();

	dae::EventBus::GetInstance().GetSubject().Notify(
		dae::Event{ dae::EventType::DamageRequested, player.object, 1 });

	if (auto* tr = player.object->GetComponent<dae::TransformComponent>())
	{
		const auto& pos = tr->GetWorldPosition();
		m_PlayerDeathWorldPosition = { pos.x, pos.y };
	}
	else
	{
		m_PlayerDeathWorldPosition = {};
	}

	m_DeathPlayerIndex = playerIndex;

	BeginPlayerDeathSequence();
}


bool digger::GameManagerComponent::AreAllPlayersDead() const
{
	if (m_Players.empty())
		return true;

	for (const auto& player : m_Players)
	{
		if (player.alive)
			return false;
	}

	return true;
}

bool digger::GameManagerComponent::IsPlayerControllable(int playerIndex) const
{
	if (IsGameplayFrozen())
		return false;

	return IsPlayerAlive(playerIndex);
}


bool digger::GameManagerComponent::IsPlayerAlive(int playerIndex) const
{
	if (playerIndex < 0 ||
		playerIndex >= static_cast<int>(m_Players.size()))
	{
		return false;
	}

	const auto& player = m_Players[static_cast<size_t>(playerIndex)];

	return player.alive && !player.dying && player.object;
}

void digger::GameManagerComponent::CheckVersusCollision()
{
	if (m_Mode != GameMode::Versus)
		return;

	if (m_Players.size() < 2)
		return;

	const auto& p1 = m_Players[0];
	const auto& p2 = m_Players[1];

	if (!p1.alive || p1.dying || !p1.object)
		return;

	if (!p2.alive || p2.dying || !p2.object)
		return;

	if (p1.role != PlayerRole::Digger)
		return;

	if (p2.role != PlayerRole::VersusEnemy)
		return;

	const glm::vec2 p1Pos = GetPlayerWorldPosition(0);
	const glm::vec2 p2Pos = GetPlayerWorldPosition(1);

	const glm::vec2 diff = p1Pos - p2Pos;

	const float radius = GetCollisionRadius();

	if ((diff.x * diff.x + diff.y * diff.y) <= radius * radius)
	{
		DamagePlayer(0);
	}
}




