#include "GameManagerComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "GridMovementComponent.h"
#include "FireballComponent.h"
#include "EnemyComponent.h"
#include "ServiceLocator.h"
#include "GameSounds.h"

#include <algorithm>
#include <cmath>
#include <memory>


//Cooldown/Shooting

float digger::GameManagerComponent::GetFireballCooldown() const
{
	return 15.0f + static_cast<float>(m_CurrentLevel) * 5.0f;
}


void digger::GameManagerComponent::ShootFireball(int playerIndex)
{
	if (!IsPlayerControllable(playerIndex))
		return;

	if (playerIndex < 0 || playerIndex >= static_cast<int>(m_Players.size()))
		return;

	auto& playerData = m_Players[static_cast<size_t>(playerIndex)];

	if (playerData.role != PlayerRole::Digger)
		return;

	if (playerData.fireballCooldown > 0.f)
		return;


	auto* player = playerData.object;
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

	playerData.fireballCooldown = GetFireballCooldown();

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

	auto* fireTr = fireball->AddComponent<dae::TransformComponent>(fireballPtr);
	fireTr->SetLocalPosition(fireballPos.x, fireballPos.y);

	fireball->AddComponent<dae::RenderComponent>(
		fireballPtr,
		dae::ResourceManager::GetInstance().LoadTexture("Fireball.png"),
		fireballSize,
		fireballSize);

	fireball->AddComponent<FireballComponent>(
		fireballPtr,
		this,
		direction,
		300.f,
		playerIndex);

	m_Fireballs.push_back(fireballPtr);
	m_LevelObjects.push_back(fireballPtr);

	dae::ServiceLocator::GetSoundSystem().PlayLooping(
		GameSound::BulletTravel,
		0.7f);

	m_Scene->Add(std::move(fireball));
}





//Removing the fireball

void digger::GameManagerComponent::RemoveFireball(dae::GameObject* fireball)
{
	if (!fireball)
		return;

	dae::ServiceLocator::GetSoundSystem().Stop(GameSound::BulletTravel);
	dae::ServiceLocator::GetSoundSystem().Play(GameSound::BulletHit, 1.0f);

	m_Fireballs.erase(
		std::remove(m_Fireballs.begin(), m_Fireballs.end(), fireball),
		m_Fireballs.end());

	m_LevelObjects.erase(
		std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), fireball),
		m_LevelObjects.end());

	m_Scene->Remove(*fireball);
}

void digger::GameManagerComponent::RemoveAllFireballs()
{
	auto& sound = dae::ServiceLocator::GetSoundSystem();
	sound.Stop(GameSound::BulletTravel);

	for (auto* fireball : m_Fireballs)
	{
		if (fireball)
		{
			m_LevelObjects.erase(
				std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), fireball),
				m_LevelObjects.end());

			m_Scene->Remove(*fireball);
		}
	}

	m_Fireballs.clear();

}


//Collision Checks

void digger::GameManagerComponent::CheckFireballHit(dae::GameObject* fireball)
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

	const glm::ivec2 fireGrid{
		static_cast<int>(std::floor((fireCenter.x - m_MapOffset.x) / static_cast<float>(m_TileSize))),
		static_cast<int>(std::floor((fireCenter.y - m_MapOffset.y) / static_cast<float>(m_TileSize)))
	};

	// Hit dirt/wall/out-of-bounds
	if (IsBlockingTileForFireball(fireGrid))
	{
		RemoveFireball(fireball);
		return;
	}

	// Hit enemy
	for (auto* enemy : m_Enemies)
	{
		if (!enemy || enemy->IsDead())
			continue;

		auto* enemyTr =
			enemy->GetGameObject()->GetComponent<dae::TransformComponent>();

		if (!enemyTr)
			continue;

		const auto& enemyPos3 = enemyTr->GetWorldPosition();

		glm::vec2 enemyCenter{
			enemyPos3.x + 32.f,
			enemyPos3.y + 32.f
		};

		const float dx = enemyCenter.x - fireCenter.x;
		const float dy = enemyCenter.y - fireCenter.y;

		const float radius = 32.f;

		if ((dx * dx + dy * dy) <= radius * radius)
		{
			enemy->Kill();
			RemoveFireball(fireball);
			return;
		}
	}

	if (m_Mode == GameMode::Versus)
	{
		for (int i = 0; i < static_cast<int>(m_Players.size()); ++i)
		{
			auto& player = m_Players[static_cast<size_t>(i)];

			if (!player.alive || player.dying || !player.object)
				continue;

			if (player.role != PlayerRole::VersusEnemy)
				continue;

			auto* enemyTr =
				player.object->GetComponent<dae::TransformComponent>();

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
				DamagePlayer(i);
				RemoveFireball(fireball);
				return;
			}
		}
	}
}

bool digger::GameManagerComponent::IsBlockingTileForFireball(const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return true;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return true;

	const char tile = m_LevelData.tiles[pos.y][pos.x];

	// Dirt or solid wall blocks fireball
	return tile == '#' || tile == 'X';
}


