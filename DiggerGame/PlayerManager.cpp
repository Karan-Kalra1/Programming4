#include "PlayerManager.h"

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

digger::PlayerManager::PlayerManager(dae::Scene* scene)
	: m_Scene(scene)
{
}

void digger::PlayerManager::ResetForNewGame(GameMode mode)
{
	m_PlayerLives = { 4, 4 };

	if (mode == GameMode::SinglePlayer)
		m_PlayerAlive = { true, false };
	else
		m_PlayerAlive = { true, true };

	m_DeathPlayerIndex = -1;

	m_Players.clear();
	m_Players.resize(2);
}

void digger::PlayerManager::ClearLevel()
{
	ReleaseAllDirections();
	RemovePlayerObservers();

	m_Players.clear();
	m_Players.resize(2);

	m_DeathPlayerIndex = -1;
}

void digger::PlayerManager::UpdateCooldowns(float deltaTime)
{
	for (auto& player : m_Players)
	{
		if (player.damageCooldown > 0.f)
			player.damageCooldown -= deltaTime;

		if (player.fireballCooldown > 0.f)
			player.fireballCooldown -= deltaTime;
	}
}

void digger::PlayerManager::SpawnPlayersForLevel(
	GameManagerComponent& game,
	const LevelData& data)
{
	RemovePlayerObservers();

	m_Players.clear();
	m_Players.resize(2);

	if (m_PlayerAlive[0])
	{
		SpawnDiggerPlayer(
			game,
			0,
			data.playerSpawn,
			"Digger.png",
			PlayerRole::Digger);
	}

	if (game.GetGameMode() == GameMode::Coop)
	{
		if (m_PlayerAlive[1])
		{
			const glm::ivec2 p2Spawn =
				data.hasPlayer2Spawn
				? data.player2Spawn
				: data.playerSpawn + glm::ivec2{ 1, 0 };

			SpawnDiggerPlayer(
				game,
				1,
				p2Spawn,
				"Digger.png",
				PlayerRole::Digger);
		}
	}
	else if (game.GetGameMode() == GameMode::Versus)
	{
		if (m_PlayerAlive[1])
		{
			const glm::ivec2 p2Spawn =
				data.hasPlayer2Spawn
				? data.player2Spawn
				: data.enemySpawn;

			SpawnDiggerPlayer(
				game,
				1,
				p2Spawn,
				"Nobbin.png",
				PlayerRole::VersusEnemy);
		}
	}
}

void digger::PlayerManager::SpawnDiggerPlayer(
	GameManagerComponent& game,
	int playerIndex,
	const glm::ivec2& spawn,
	const std::string& textureFile,
	PlayerRole role)
{
	if (playerIndex < 0 || playerIndex >= 2)
		return;

	if (!m_PlayerAlive[static_cast<size_t>(playerIndex)])
		return;

	if (!m_Scene)
		return;

	auto player = std::make_unique<dae::GameObject>();
	auto* playerPtr = player.get();

	player->AddComponent<dae::TransformComponent>(playerPtr);

	auto* movement =
		player->AddComponent<GridMovementComponent>(
			playerPtr,
			&game,
			game.GetTileSize(),
			160.f,
			game.GetMapOffset());

	movement->SetGridPosition(spawn);

	if (role == PlayerRole::VersusEnemy)
	{
		movement->SetCanDigDirt(false);
		movement->SetCanEnterDirt(false);
	}


	player->AddComponent<dae::RenderComponent>(
		playerPtr,
		dae::ResourceManager::GetInstance().LoadTexture(textureFile));

	auto* actor =
		player->AddComponent<dae::GameActorComponent>(
			playerPtr,
			m_PlayerLives[static_cast<size_t>(playerIndex)],
			0,
			playerIndex + 1);

	dae::EventBus::GetInstance().GetSubject().AddObserver(actor);

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
			false,
			role
	};

	game.AddLevelObject(playerPtr);
	m_Scene->Add(std::move(player));
}

void digger::PlayerManager::RemovePlayerObservers()
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

const digger::PlayerRuntime* digger::PlayerManager::GetPlayerRuntime(
	int playerIndex) const
{
	if (playerIndex < 0 ||
		playerIndex >= static_cast<int>(m_Players.size()))
		return nullptr;

	return &m_Players[static_cast<size_t>(playerIndex)];
}

digger::PlayerRuntime* digger::PlayerManager::GetMutablePlayerRuntime(
	int playerIndex)
{
	if (playerIndex < 0 ||
		playerIndex >= static_cast<int>(m_Players.size()))
		return nullptr;

	return &m_Players[static_cast<size_t>(playerIndex)];
}

dae::GameObject* digger::PlayerManager::GetPlayer(
	int playerIndex) const
{
	const auto* player = GetPlayerRuntime(playerIndex);
	if (!player)
		return nullptr;

	return player->object;
}

glm::ivec2 digger::PlayerManager::GetPlayerGridPosition(
	int playerIndex) const
{
	auto* player = GetPlayer(playerIndex);
	if (!player)
		return {};

	if (auto* movement = player->GetComponent<GridMovementComponent>())
		return movement->GetGridPosition();

	return {};
}

glm::vec2 digger::PlayerManager::GetPlayerWorldPosition(
	int playerIndex) const
{
	auto* player = GetPlayer(playerIndex);
	if (!player)
		return {};

	if (auto* movement = player->GetComponent<GridMovementComponent>())
		return movement->GetWorldPosition() + glm::vec2{ 32.f, 32.f };

	if (auto* tr = player->GetComponent<dae::TransformComponent>())
	{
		const auto& pos = tr->GetWorldPosition();
		return { pos.x + 32.f, pos.y + 32.f };
	}

	return {};
}

glm::ivec2 digger::PlayerManager::GetClosestAlivePlayerGridPosition(
	const glm::ivec2& fromGrid) const
{
	float bestDistanceSq = std::numeric_limits<float>::max();
	int bestIndex = -1;

	for (int i = 0; i < static_cast<int>(m_Players.size()); ++i)
	{
		const auto& player = m_Players[static_cast<size_t>(i)];

		if (!player.alive || player.dying || !player.object)
			continue;

		if (player.role != PlayerRole::Digger)
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
		return fromGrid;

	return GetPlayerGridPosition(bestIndex);
}

int digger::PlayerManager::GetPlayerIndexAtWorldPosition(
	const glm::vec2& worldPos,
	float radius) const
{
	return GetPlayerIndexAtWorldPosition(
		worldPos,
		radius,
		std::nullopt);
}

int digger::PlayerManager::GetDiggerPlayerIndexAtWorldPosition(
	const glm::vec2& worldPos,
	float radius) const
{
	return GetPlayerIndexAtWorldPosition(
		worldPos,
		radius,
		PlayerRole::Digger);
}

int digger::PlayerManager::GetPlayerIndexAtWorldPosition(
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

		if (requiredRole.has_value() &&
			player.role != requiredRole.value())
			continue;

		const glm::vec2 playerPos = GetPlayerWorldPosition(i);
		const glm::vec2 diff = playerPos - worldPos;

		if ((diff.x * diff.x + diff.y * diff.y) <= radiusSq)
			return i;
	}

	return -1;
}

int digger::PlayerManager::GetPlayerIndexAtGridPosition(
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

bool digger::PlayerManager::IsPlayerAlive(int playerIndex) const
{
	const auto* player = GetPlayerRuntime(playerIndex);
	if (!player)
		return false;

	return player->alive &&
		!player->dying &&
		player->object;
}

bool digger::PlayerManager::IsPlayerControllable(
	const GameManagerComponent& game,
	int playerIndex) const
{
	if (game.IsGameplayFrozen())
		return false;

	return IsPlayerAlive(playerIndex);
}

bool digger::PlayerManager::CanPlayerShootFireball(
	const GameManagerComponent& game,
	int playerIndex) const
{
	if (!IsPlayerControllable(game, playerIndex))
		return false;

	const auto* player = GetPlayerRuntime(playerIndex);
	if (!player)
		return false;

	if (player->role != PlayerRole::Digger)
		return false;

	if (player->fireballCooldown > 0.f)
		return false;

	if (player->fireballActive)
		return false;

	return true;
}

void digger::PlayerManager::StartPlayerFireballCooldown(
	int playerIndex,
	float cooldown)
{
	auto* player = GetMutablePlayerRuntime(playerIndex);
	if (!player)
		return;

	player->fireballCooldown = cooldown;
	player->fireballActive = true;
}

void digger::PlayerManager::SetPlayerFireballActive(
	int playerIndex,
	bool active)
{
	auto* player = GetMutablePlayerRuntime(playerIndex);
	if (!player)
		return;

	player->fireballActive = active;
}

void digger::PlayerManager::ClearAllPlayerFireballStates()
{
	for (auto& player : m_Players)
		player.fireballActive = false;
}

void digger::PlayerManager::DamagePlayer(
	GameManagerComponent& game,
	int playerIndex)
{
	if (playerIndex < 0 ||
		playerIndex >= static_cast<int>(m_Players.size()))
		return;

	auto& player = m_Players[static_cast<size_t>(playerIndex)];

	if (!player.alive || player.dying || !player.object)
		return;

	if (player.damageCooldown > 0.f)
		return;

	player.damageCooldown = 1.0f;
	player.dying = true;

	--player.lives;
	m_PlayerLives[static_cast<size_t>(playerIndex)] = player.lives;

	game.UpdateHUD();

	dae::EventBus::GetInstance().GetSubject().Notify(
		dae::Event{ dae::EventType::DamageRequested, player.object, 1 });

	glm::vec2 deathPos{};

	if (auto* tr = player.object->GetComponent<dae::TransformComponent>())
	{
		const auto& pos = tr->GetWorldPosition();
		deathPos = { pos.x, pos.y };
	}

	m_DeathPlayerIndex = playerIndex;

	ReleaseAllDirections();
	game.RemoveAllFireballs();
	HidePlayer(playerIndex);

	game.StartDeathSequenceAt(deathPos);
}

void digger::PlayerManager::ReleaseAllDirections()
{
	for (auto& player : m_Players)
	{
		if (!player.object)
			continue;

		if (auto* movement = player.object->GetComponent<GridMovementComponent>())
			movement->ReleaseAllDirections();
	}
}

void digger::PlayerManager::HidePlayer(int playerIndex)
{
	auto* player = GetPlayer(playerIndex);
	if (!player)
		return;

	if (auto* tr = player->GetComponent<dae::TransformComponent>())
		tr->SetLocalPosition(-5000.f, -5000.f);
}

void digger::PlayerManager::FinishDeathSequence(
	GameManagerComponent& game)
{
	if (m_DeathPlayerIndex < 0 ||
		m_DeathPlayerIndex >= static_cast<int>(m_Players.size()))
	{
		m_DeathPlayerIndex = -1;
		return;
	}

	auto& player = m_Players[static_cast<size_t>(m_DeathPlayerIndex)];

	game.ResetEnemiesAfterPlayerDeath();

	if (player.lives <= 0)
	{
		player.alive = false;
		player.dying = false;

		m_PlayerAlive[static_cast<size_t>(m_DeathPlayerIndex)] = false;

		HidePlayer(m_DeathPlayerIndex);

		if (auto* movement =
			player.object
			? player.object->GetComponent<GridMovementComponent>()
			: nullptr)
		{
			movement->ReleaseAllDirections();
		}

		const bool gameShouldEnd =
			game.GetGameMode() == GameMode::Versus ||
			AreAllPlayersDead();

		m_DeathPlayerIndex = -1;

		if (gameShouldEnd)
		{
			game.GameOver();
			return;
		}
	}
	else
	{
		player.dying = false;
		player.damageCooldown = 1.0f;

		if (player.object)
		{
			if (auto* movement = player.object->GetComponent<GridMovementComponent>())
				movement->SetGridPosition(player.spawn);
		}

		m_DeathPlayerIndex = -1;
	}

	dae::ServiceLocator::GetSoundSystem().PlayLooping(
		GameSound::BackgroundMusic,
		0.5f);

	game.UpdateHUD();
}

bool digger::PlayerManager::AreAllPlayersDead() const
{
	for (const auto& player : m_Players)
	{
		if (player.alive && player.object)
			return false;
	}

	return true;
}

void digger::PlayerManager::CheckVersusCollision(
	GameManagerComponent& game)
{
	if (game.GetGameMode() != GameMode::Versus)
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

	const float radius = game.GetCollisionRadius();

	if ((diff.x * diff.x + diff.y * diff.y) <= radius * radius)
		game.DamagePlayer(0);
}