#include "FireballManager.h"

#include "GameManagerComponent.h"
#include "FireballComponent.h"
#include "EnemyComponent.h"
#include "GridMovementComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "ServiceLocator.h"
#include "GameSounds.h"

#include <algorithm>
#include <cmath>
#include <memory>

digger::FireballManager::FireballManager(dae::Scene* scene)
	: m_Scene(scene)
{
}

float digger::FireballManager::GetCooldown(
	const GameManagerComponent& game) const
{
	return 10.f + static_cast<float>(game.GetCurrentLevel()) * 2.5f;
}

void digger::FireballManager::Shoot(
	GameManagerComponent& game,
	int playerIndex)
{
	if (!game.CanPlayerShootFireball(playerIndex))
		return;

	const auto* playerData = game.GetPlayerRuntime(playerIndex);
	if (!playerData)
		return;

	auto* player = playerData->object;
	if (!player)
		return;

	auto* playerTr = player->GetComponent<dae::TransformComponent>();
	if (!playerTr)
		return;

	auto* movement = player->GetComponent<GridMovementComponent>();
	if (!movement)
		return;

	const glm::ivec2 facing = movement->GetFacingDirection();

	glm::vec2 direction{
		static_cast<float>(facing.x),
		static_cast<float>(facing.y)
	};

	if (direction == glm::vec2{})
		direction = { 1.f, 0.f };

	game.StartPlayerFireballCooldown(
		playerIndex,
		GetCooldown(game));


	const auto& pos3 = playerTr->GetWorldPosition();

	constexpr float playerCenter = 32.f;
	constexpr float fireballSize = 64.f;

	glm::vec2 fireballPos{
		pos3.x + playerCenter - fireballSize * 0.5f,
		pos3.y + playerCenter - fireballSize * 0.5f
	};

	fireballPos += direction * 28.f;

	auto fireball = std::make_unique<dae::GameObject>();
	auto* fireballPtr = fireball.get();

	auto* fireTr =
		fireball->AddComponent<dae::TransformComponent>(fireballPtr);

	fireTr->SetLocalPosition(fireballPos.x, fireballPos.y);

	fireball->AddComponent<dae::RenderComponent>(
		fireballPtr,
		dae::ResourceManager::GetInstance().LoadTexture("Fireball.png"),
		fireballSize,
		fireballSize);

	fireball->AddComponent<FireballComponent>(
		fireballPtr,
		&game,
		direction,
		300.f,
		playerIndex);

	m_Fireballs.push_back(fireballPtr);
	game.AddLevelObject(fireballPtr);

	if (!HasAnyFireballs())
	{
		dae::ServiceLocator::GetSoundSystem().PlayLooping(
			GameSound::BulletTravel,
			0.7f);
	}
	else
	{

		dae::ServiceLocator::GetSoundSystem().PlayLooping(
			GameSound::BulletTravel,
			0.7f);
	}

	if (m_Scene)
		m_Scene->Add(std::move(fireball));
}

void digger::FireballManager::CheckHit(
	GameManagerComponent& game,
	dae::GameObject* fireball)
{
	if (!fireball)
		return;

	auto* fireTr = fireball->GetComponent<dae::TransformComponent>();
	if (!fireTr)
		return;

	const auto& firePos3 = fireTr->GetWorldPosition();

	glm::vec2 fireCenter{
		firePos3.x + 12.f,
		firePos3.y + 12.f
	};

	const glm::ivec2 fireGrid =
		WorldToGrid(game, fireCenter);

	if (IsBlockingTileForFireball(game, fireGrid))
	{
		Remove(game, fireball);
		return;
	}

	// Hit AI enemies
	for (auto* enemy : game.GetEnemies())
	{
		if (!enemy || enemy->IsDead())
			continue;

		auto* enemyObj = enemy->GetGameObject();
		if (!enemyObj)
			continue;

		auto* enemyTr =
			enemyObj->GetComponent<dae::TransformComponent>();

		if (!enemyTr)
			continue;

		const auto& enemyPos3 = enemyTr->GetWorldPosition();

		glm::vec2 enemyCenter{
			enemyPos3.x + 32.f,
			enemyPos3.y + 32.f
		};

		const glm::vec2 diff = enemyCenter - fireCenter;
		const float radius = 32.f;

		if ((diff.x * diff.x + diff.y * diff.y) <= radius * radius)
		{
			enemy->Kill();
			Remove(game, fireball);
			return;
		}
	}

	// Hit versus enemy player
	if (game.GetGameMode() == GameMode::Versus)
	{
		const auto& players = game.GetPlayers();

		for (int i = 0; i < static_cast<int>(players.size()); ++i)
		{
			const auto& player = players[static_cast<size_t>(i)];

			if (!player.alive || player.dying || !player.object)
				continue;

			if (player.role != PlayerRole::VersusEnemy)
				continue;

			auto* tr =
				player.object->GetComponent<dae::TransformComponent>();

			if (!tr)
				continue;

			const auto& pos3 = tr->GetWorldPosition();

			glm::vec2 center{
				pos3.x + 32.f,
				pos3.y + 32.f
			};

			const glm::vec2 diff = center - fireCenter;
			const float radius = 32.f;

			if ((diff.x * diff.x + diff.y * diff.y) <= radius * radius)
			{
				game.DamagePlayer(i);
				Remove(game, fireball);
				return;
			}
		}
	}
}

void digger::FireballManager::Remove(
	GameManagerComponent& game,
	dae::GameObject* fireball)
{
	if (!fireball)
		return;

	if (auto* fireballComp = fireball->GetComponent<FireballComponent>())
	{
		const int ownerIndex = fireballComp->GetOwnerPlayerIndex();
		game.SetPlayerFireballActive(ownerIndex, false);
	}

	m_Fireballs.erase(
		std::remove(m_Fireballs.begin(), m_Fireballs.end(), fireball),
		m_Fireballs.end());

	game.RemoveLevelObject(fireball);

	if (m_Scene)
		m_Scene->Remove(*fireball);

	auto& sound = dae::ServiceLocator::GetSoundSystem();

	if (m_Fireballs.empty())
		sound.Stop(GameSound::BulletTravel);

	sound.Play(GameSound::BulletHit, 1.0f);
}

void digger::FireballManager::RemoveAll(GameManagerComponent& game)
{
	auto& sound = dae::ServiceLocator::GetSoundSystem();
	sound.Stop(GameSound::BulletTravel);

	for (auto* fireball : m_Fireballs)
	{
		if (!fireball)
			continue;

		game.RemoveLevelObject(fireball);

		if (m_Scene)
			m_Scene->Remove(*fireball);
	}

	m_Fireballs.clear();

	game.ClearAllPlayerFireballStates();
}

bool digger::FireballManager::IsBlockingTileForFireball(
	const GameManagerComponent& game,
	const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return true;

	if (pos.y >= game.GetLevelHeight() ||
		pos.x >= game.GetLevelWidth())
	{
		return true;
	}

	if (game.IsDirt(pos))
		return true;

	if (game.IsSolidWall(pos))
		return true;

	return false;
}

glm::ivec2 digger::FireballManager::WorldToGrid(
	const GameManagerComponent& game,
	const glm::vec2& worldPos) const
{
	const glm::vec2 offset = game.GetMapOffset();
	const int tileSize = game.GetTileSize();

	const int x = static_cast<int>(
		std::floor((worldPos.x - offset.x) / static_cast<float>(tileSize)));

	const int y = static_cast<int>(
		std::floor((worldPos.y - offset.y) / static_cast<float>(tileSize)));

	return { x, y };
}

bool digger::FireballManager::HasAnyFireballs() const
{
	return !m_Fireballs.empty();
}