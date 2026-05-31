#include "GameManagerComponent.h"

#include "LevelLoader.h"
#include "GridPositionComponent.h"
#include "DiamondComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "GameActorComponent.h"
#include "EventBus.h"
#include "ServiceLocator.h"
#include "GridMovementComponent.h"
#include "MiniginTime.h"
#include "FireballComponent.h"

#include <memory>
#include <iostream>

namespace
{
	enum GameSound : dae::SoundId
	{
		DiamondPickUp = 1
	};
}

digger::GameManagerComponent::GameManagerComponent(dae::GameObject* owner, dae::Scene* scene)
	: Component(owner)
	, m_Scene(scene)
{
}

void digger::GameManagerComponent::StartGame(GameMode mode)
{
	m_Mode = mode;
	m_Score = 0;
	m_Lives = 4;

	dae::ServiceLocator::GetSoundSystem().RegisterSound(
		DiamondPickUp,
		"Data/sounds/PickUpDiamond.wav");

	LoadLevel(0);
}

void digger::GameManagerComponent::LoadLevel(int index)
{
	m_CurrentLevel = index;

	if (m_CurrentLevel > 2)
	{
		std::cout << "Game finished!\n";
		return;
	}

	ClearLevel();

	const auto path =
		std::filesystem::path{ "Data/Levels/Level" + std::to_string(m_CurrentLevel + 1) + ".txt" };

	m_LevelData = LevelLoader::Load(path);
	SpawnLevel(m_LevelData);

	m_TotalEnemiesThisStage = 3 + m_CurrentLevel * 2;
	m_EnemiesRemainingToSpawn = m_TotalEnemiesThisStage;
	m_EnemiesAlive = 0;
	m_EnemySpawnTimer = 0.f;
}

void digger::GameManagerComponent::SkipLevel()
{
	LoadLevel(m_CurrentLevel + 1);
}

void digger::GameManagerComponent::Update()
{
	if (m_ShouldLoadNextLevel)
	{
		m_ShouldLoadNextLevel = false;
		LoadLevel(m_CurrentLevel + 1);
	}

	if (m_EnemiesRemainingToSpawn > 0)
	{
		m_EnemySpawnTimer += dae::MiniginTime::GetDeltaTime();

		if (m_EnemySpawnTimer >= m_EnemySpawnInterval)
		{
			m_EnemySpawnTimer = 0.f;
			SpawnEnemy();
		}
	}

	if (m_EnemiesRemainingToSpawn <= 0 && m_EnemiesAlive <= 0)
	{
		m_ShouldLoadNextLevel = true;
	}

	if (m_FireballCooldownTimer > 0.f)
		m_FireballCooldownTimer -= dae::MiniginTime::GetDeltaTime();

	CheckEnemyCrossings();
}

void digger::GameManagerComponent::ToggleMute()
{
	auto& sound = dae::ServiceLocator::GetSoundSystem();
	sound.SetMuted(!sound.IsMuted());
}

void digger::GameManagerComponent::CollectDiamond(dae::GameObject* diamond)
{
	if (!diamond)
		return;

	m_Score += 100;

	dae::ServiceLocator::GetSoundSystem().Play(DiamondPickUp, 1.0f);

	m_Scene->Remove(*diamond);

	m_Diamonds.erase(
		std::remove(m_Diamonds.begin(), m_Diamonds.end(), diamond),
		m_Diamonds.end());

	m_LevelObjects.erase(
		std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), diamond),
		m_LevelObjects.end());

	if (m_Diamonds.empty())
		m_ShouldLoadNextLevel = true;
}
void digger::GameManagerComponent::ClearLevel()
{
	if (m_PlayerActor)
	{
		dae::EventBus::GetInstance().GetSubject().RemoveObserver(m_PlayerActor);
		m_PlayerActor = nullptr;
	}

	for (auto* object : m_LevelObjects)
	{
		if (object)
			m_Scene->Remove(*object);
	}

	m_LevelObjects.clear();
	m_Diamonds.clear();
	m_Enemies.clear();
	m_DirtTiles.clear();
	m_Player = nullptr;
}

void digger::GameManagerComponent::SpawnLevel(const LevelData& data)
{
	auto player = std::make_unique<dae::GameObject>();
	auto* playerPtr = player.get();

	player->AddComponent<dae::TransformComponent>(playerPtr);

	auto* movement = player->AddComponent<GridMovementComponent>(
		playerPtr,
		this,
		m_TileSize,
		120.f,
		m_MapOffset);

	movement->SetGridPosition(data.playerSpawn);

	
	player->AddComponent<dae::RenderComponent>(
		playerPtr,
		dae::ResourceManager::GetInstance().LoadTexture("Digger.png"));

	auto* actor = player->AddComponent<dae::GameActorComponent>(playerPtr, m_Lives, m_Score, 1);
	m_PlayerActor = actor;
	dae::EventBus::GetInstance().GetSubject().AddObserver(m_PlayerActor);

	m_Player = playerPtr;
	m_PlayerSpawn = data.playerSpawn;
	m_LevelObjects.push_back(playerPtr);
	m_Scene->Add(std::move(player));

	m_DirtTiles.clear();

	auto dirtTexture =
		dae::ResourceManager::GetInstance().LoadTexture("Dirt.png");

	for (int y = 0; y < data.height; ++y)
	{
		for (int x = 0; x < data.width; ++x)
		{
			if (data.tiles[y][x] != '#')
				continue;

			const glm::ivec2 gridPos{ x, y };
			const std::string key = MakeTileKey(gridPos);

			DirtTile dirtTile{};

			const int subdivisions = 16;
			const float pieceSize =
				static_cast<float>(m_TileSize) / static_cast<float>(subdivisions);

			for (int py = 0; py < subdivisions; ++py)
			{
				for (int px = 0; px < subdivisions; ++px)
				{
					const int pieceIndex = py * subdivisions + px;

					auto dirt = std::make_unique<dae::GameObject>();
					auto* dirtPtr = dirt.get();

					auto* transform =
						dirt->AddComponent<dae::TransformComponent>(dirtPtr);

					transform->SetLocalPosition(
						m_MapOffset.x + static_cast<float>(x * m_TileSize) + px * pieceSize,
						m_MapOffset.y + static_cast<float>(y * m_TileSize) + py * pieceSize);

					dirt->AddComponent<dae::RenderComponent>(
						dirtPtr,
						dirtTexture,
						pieceSize,
						pieceSize);

					dirtTile.pieces[pieceIndex] = dirtPtr;

					m_LevelObjects.push_back(dirtPtr);
					m_Scene->Add(std::move(dirt));
				}
			}

			m_DirtTiles[key] = dirtTile;
		}
	}


	for (const auto& diamondPos : data.diamonds)
	{
		auto diamond = std::make_unique<dae::GameObject>();
		auto* diamondPtr = diamond.get();

		diamond->AddComponent<dae::TransformComponent>(diamondPtr);
		diamond->AddComponent<GridPositionComponent>(diamondPtr, m_TileSize, m_MapOffset)
			->SetGridPosition(diamondPos);


		diamond->AddComponent<dae::RenderComponent>(
			diamondPtr,
			dae::ResourceManager::GetInstance().LoadTexture("Diamond.png"));

		diamond->AddComponent<DiamondComponent>(diamondPtr, this);

		m_Diamonds.push_back(diamondPtr);
		m_LevelObjects.push_back(diamondPtr);
		m_Scene->Add(std::move(diamond));
	}

	/*for (const auto& enemyPos : data.enemies)
	{
		auto enemy = std::make_unique<dae::GameObject>();
		auto* enemyPtr = enemy.get();

		enemy->AddComponent<dae::TransformComponent>(enemyPtr);

		enemy->AddComponent<dae::RenderComponent>(
			enemyPtr,
			dae::ResourceManager::GetInstance().LoadTexture("Nobbin.png"));

		auto* enemyComp = enemy->AddComponent<EnemyComponent>(enemyPtr, this);
		enemyComp->SetGridPosition(enemyPos);

		RegisterEnemy(enemyComp);
		m_LevelObjects.push_back(enemyPtr);
		m_Scene->Add(std::move(enemy));
	}
	*/

	m_EnemySpawn = data.enemySpawn;
}

std::string digger::GameManagerComponent::MakeTileKey(const glm::ivec2& pos) const
{
	return std::to_string(pos.x) + "," + std::to_string(pos.y);
}

bool digger::GameManagerComponent::IsDirt(const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return false;

	return m_LevelData.tiles[pos.y][pos.x] == '#';
}

bool digger::GameManagerComponent::CanEnemyMoveTo(const glm::ivec2& pos, bool canDig) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return false;

	const char tile = m_LevelData.tiles[pos.y][pos.x];

	if (tile == '#')
		return canDig;

	return true;
}

void digger::GameManagerComponent::DigTile(const glm::ivec2& pos)
{
	if (!IsDirt(pos))
		return;

	auto key = MakeTileKey(pos);
	auto it = m_DirtTiles.find(key);

	if (it != m_DirtTiles.end())
	{
		for (int i = 0; i < 4; ++i)
		{
			if (auto* obj = it->second.pieces[i])
			{
				if (auto* tr = obj->GetComponent<dae::TransformComponent>())
					tr->SetLocalPosition(-5000.f, -5000.f);
			}
		}

		m_DirtTiles.erase(it);
	}

	m_LevelData.tiles[pos.y][pos.x] = '.';
}



glm::ivec2 digger::GameManagerComponent::GetPlayerGridPosition() const
{
	
		if (!m_Player)
			return {};

		if (auto* movement = m_Player->GetComponent<GridMovementComponent>())
			return movement->GetGridPosition();

		if (auto* grid = m_Player->GetComponent<GridPositionComponent>())
			return grid->GetGridPosition();

		return {};
	
}

void digger::GameManagerComponent::DamagePlayer()
{
	if (!m_Player)
		return;

	dae::EventBus::GetInstance().GetSubject().Notify(
		dae::Event{ dae::EventType::DamageRequested, m_Player, 1 });

      auto* grid = m_Player->GetComponent<GridMovementComponent>();
		if (grid)
		 grid->SetGridPosition(m_PlayerSpawn);

		ResetEnemiesAfterPlayerDeath();
}

void digger::GameManagerComponent::RegisterEnemy(EnemyComponent* enemy)
{
	if (enemy)
		m_Enemies.push_back(enemy);
}

void digger::GameManagerComponent::CheckEnemyCrossings()
{
	for (size_t i = 0; i < m_Enemies.size(); ++i)
	{
		for (size_t j = i + 1; j < m_Enemies.size(); ++j)
		{
			if (!m_Enemies[i] || !m_Enemies[j])
				continue;

			if (m_Enemies[i]->GetGridPosition() == m_Enemies[j]->GetGridPosition())
			{
				m_Enemies[i]->RegisterEnemyCollision();
				m_Enemies[j]->RegisterEnemyCollision();
			}
		}
	}
}

bool digger::GameManagerComponent::CanPlayerMoveTo(const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return false;

	const char tile = m_LevelData.tiles[pos.y][pos.x];

	if (tile == 'X')
		return false;

	return true;
}

void digger::GameManagerComponent::DigAtWorldPosition(
	const glm::vec2& worldPos,
	const glm::ivec2&)
{
	const float r = m_DigRadius;

	DigAtPoint(worldPos);

	DigAtPoint(worldPos + glm::vec2{ r, 0.f });
	DigAtPoint(worldPos + glm::vec2{ -r, 0.f });
	DigAtPoint(worldPos + glm::vec2{ 0.f, r });
	DigAtPoint(worldPos + glm::vec2{ 0.f, -r });

	const float diagonal = r * 0.7071f;

	DigAtPoint(worldPos + glm::vec2{ diagonal, diagonal });
	DigAtPoint(worldPos + glm::vec2{ -diagonal, diagonal });
	DigAtPoint(worldPos + glm::vec2{ diagonal, -diagonal });
	DigAtPoint(worldPos + glm::vec2{ -diagonal, -diagonal });
}

glm::vec2 digger::GameManagerComponent::GetPlayerWorldPosition() const
{
	if (!m_Player)
		return {};

	if (auto* movement = m_Player->GetComponent<GridMovementComponent>())
		return movement->GetWorldPosition() + m_PlayerCenterOffset;

	if (auto* tr = m_Player->GetComponent<dae::TransformComponent>())
	{
		const auto& pos = tr->GetWorldPosition();
		return { pos.x + m_PlayerCenterOffset.x, pos.y + m_PlayerCenterOffset.y };
	}

	return {};
}


void digger::GameManagerComponent::DigAtPoint(const glm::vec2& point)
{
	const int gridX = static_cast<int>(
		std::floor((point.x - m_MapOffset.x) / static_cast<float>(m_TileSize)));

	const int gridY = static_cast<int>(
		std::floor((point.y - m_MapOffset.y) / static_cast<float>(m_TileSize)));

	const glm::ivec2 tilePos{ gridX, gridY };

	if (!IsDirt(tilePos))
		return;

	auto it = m_DirtTiles.find(MakeTileKey(tilePos));
	if (it == m_DirtTiles.end())
		return;

	auto& tile = it->second;

	constexpr int subdivisions = 16;
	const float pieceSize =
		static_cast<float>(m_TileSize) / static_cast<float>(subdivisions);

	const float tileWorldX = m_MapOffset.x + static_cast<float>(gridX * m_TileSize);
	const float tileWorldY = m_MapOffset.y + static_cast<float>(gridY * m_TileSize);

	const float digRadius = 16.0f;
	const float digRadiusSq = digRadius * digRadius;

	auto removePiece = [&](int index)
		{
			if (tile.removed[index])
				return;

			tile.removed[index] = true;

			if (auto* obj = tile.pieces[index])
			{
				if (auto* tr = obj->GetComponent<dae::TransformComponent>())
					tr->SetLocalPosition(-5000.f, -5000.f);
			}
		};

	for (int py = 0; py < subdivisions; ++py)
	{
		for (int px = 0; px < subdivisions; ++px)
		{
			const int index = py * subdivisions + px;

			const glm::vec2 pieceCenter{
				tileWorldX + px * pieceSize + pieceSize * 0.5f,
				tileWorldY + py * pieceSize + pieceSize * 0.5f
			};

			const glm::vec2 diff = pieceCenter - point;
			const float distSq = diff.x * diff.x + diff.y * diff.y;

			if (distSq <= digRadiusSq)
				removePiece(index);
		}
	}

	int removedCount = 0;

	for (bool removed : tile.removed)
	{
		if (removed)
			++removedCount;
	}

	const int requiredRemoved =
		static_cast<int>(DirtPieceCount * 0.95f);

	if (removedCount >= requiredRemoved)
	{
		for (int i = 0; i < DirtPieceCount; ++i)
		{
			if (!tile.removed[i])
			{
				tile.removed[i] = true;

				if (auto* obj = tile.pieces[i])
				{
					if (auto* tr = obj->GetComponent<dae::TransformComponent>())
						tr->SetLocalPosition(-5000.f, -5000.f);
				}
			}
		}

		m_LevelData.tiles[tilePos.y][tilePos.x] = '.';
	}
}


void digger::GameManagerComponent::SpawnEnemy()
{
	if (m_EnemiesRemainingToSpawn <= 0)
		return;

	auto enemy = std::make_unique<dae::GameObject>();
	auto* enemyPtr = enemy.get();

	enemy->AddComponent<dae::TransformComponent>(enemyPtr);

	enemy->AddComponent<dae::RenderComponent>(
		enemyPtr,
		dae::ResourceManager::GetInstance().LoadTexture("Nobbin.png"));

	auto* enemyComp = enemy->AddComponent<EnemyComponent>(enemyPtr, this);
	enemyComp->SetGridPosition(m_EnemySpawn);

	RegisterEnemy(enemyComp);

	m_LevelObjects.push_back(enemyPtr);
	m_EnemiesAlive++;
	m_EnemiesRemainingToSpawn--;

	m_Scene->Add(std::move(enemy));
}


void digger::GameManagerComponent::KillEnemy(EnemyComponent* enemy)
{
	if (!enemy)
		return;

	auto* enemyObject = enemy->GetGameObject();

	m_Enemies.erase(
		std::remove(m_Enemies.begin(), m_Enemies.end(), enemy),
		m_Enemies.end());

	m_LevelObjects.erase(
		std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), enemyObject),
		m_LevelObjects.end());

	m_Scene->Remove(*enemyObject);

	--m_EnemiesAlive;
	if (m_EnemiesAlive < 0)
		m_EnemiesAlive = 0;
}


void digger::GameManagerComponent::ResetEnemiesAfterPlayerDeath()
{
	for (auto* enemy : m_Enemies)
	{
		if (!enemy)
			continue;

		auto* enemyObject = enemy->GetGameObject();

		m_LevelObjects.erase(
			std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), enemyObject),
			m_LevelObjects.end());

		m_Scene->Remove(*enemyObject);
	}

	m_Enemies.clear();

	m_EnemiesRemainingToSpawn += m_EnemiesAlive;
	m_EnemiesAlive = 0;
	m_EnemySpawnTimer = 0.f;
}

float digger::GameManagerComponent::GetFireballCooldown() const
{
	return 0.5f + static_cast<float>(m_CurrentLevel) * 0.25f;
}

glm::ivec2 digger::GameManagerComponent::GetPlayerFacingDirection() const
{
	if (!m_Player)
		return { 1, 0 };

	auto* movement = m_Player->GetComponent<GridMovementComponent>();
	if (!movement)
		return { 1, 0 };

	return movement->GetFacingDirection();
}

void digger::GameManagerComponent::ShootFireball()
{
	if (m_FireballCooldownTimer > 0.f)
		return;

	if (!m_Player)
		return;

	m_FireballCooldownTimer = GetFireballCooldown();

	auto* tr = m_Player->GetComponent<dae::TransformComponent>();
	if (!tr)
		return;

	const auto& pos3 = tr->GetWorldPosition();

	glm::ivec2 facing = GetPlayerFacingDirection();
	glm::vec2 direction{
		static_cast<float>(facing.x),
		static_cast<float>(facing.y)
	};

	auto fireball = std::make_unique<dae::GameObject>();
	auto* fireballPtr = fireball.get();

	auto* fireTr = fireball->AddComponent<dae::TransformComponent>(fireballPtr);

	
	glm::vec2 fireballPos{
		pos3.x ,
		pos3.y 
	};

	fireballPos += glm::vec2{
		static_cast<float>(facing.x),
		static_cast<float>(facing.y)
	} * 28.f;

	fireTr->SetLocalPosition(fireballPos.x, fireballPos.y);
	
	fireball->AddComponent<dae::RenderComponent>(
		fireballPtr,
		dae::ResourceManager::GetInstance().LoadTexture("Fireball.png"));

	fireball->AddComponent<FireballComponent>(
		fireballPtr,
		this,
		direction,
		300.f);

	m_Fireballs.push_back(fireballPtr);
	m_LevelObjects.push_back(fireballPtr);

	m_Scene->Add(std::move(fireball));
}

void digger::GameManagerComponent::CheckFireballHit(dae::GameObject* fireball)
{
	if (!fireball)
		return;

	auto* fireTr = fireball->GetComponent<dae::TransformComponent>();
	if (!fireTr)
		return;

	const auto& firePos3 = fireTr->GetWorldPosition();
	glm::vec2 firePos{ firePos3.x, firePos3.y };

	for (auto* enemy : m_Enemies)
	{
		if (!enemy || enemy->IsDead())
			continue;

		auto* enemyTr = enemy->GetGameObject()->GetComponent<dae::TransformComponent>();
		if (!enemyTr)
			continue;

		const auto& enemyPos3 = enemyTr->GetWorldPosition();
		glm::vec2 enemyPos{ enemyPos3.x + 32.f, enemyPos3.y + 32.f };

		const float dx = enemyPos.x - firePos.x;
		const float dy = enemyPos.y - firePos.y;

		const float radius = 32.f;

		if ((dx * dx + dy * dy) <= radius * radius)
		{
			enemy->Kill();

			m_Fireballs.erase(
				std::remove(m_Fireballs.begin(), m_Fireballs.end(), fireball),
				m_Fireballs.end());

			m_LevelObjects.erase(
				std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), fireball),
				m_LevelObjects.end());

			m_Scene->Remove(*fireball);
			return;
		}
	}
}