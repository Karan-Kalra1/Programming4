#include "GameManagerComponent.h"

#include "LevelLoader.h"
#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "DiamondComponent.h"
#include "MoneyBagComponent.h"
#include "EnemyComponent.h"
#include "GridMovementComponent.h"
#include "GameActorComponent.h"
#include "EventBus.h"
#include "GridPositionComponent.h"
#include "EnemyManager.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <string>


//Spawning level

void digger::GameManagerComponent::SpawnLevel(const LevelData& data)
{
	m_Players.clear();

	if (m_Mode == GameMode::Coop)
		m_Players.resize(2);

	if (m_PlayerAlive[0])
	{
		SpawnDiggerPlayer(
			0,
			data.playerSpawn,
			"Digger.png",
			PlayerRole::Digger);
	}

	// P2 depends on mode
	if (m_Mode == GameMode::Coop)
	{
		if (m_PlayerAlive[1])
		{
			const glm::ivec2 p2Spawn =
				data.hasPlayer2Spawn
				? data.player2Spawn
				: data.playerSpawn + glm::ivec2{ 1, 0 };

			SpawnDiggerPlayer(
				1,
				p2Spawn,
				"Digger.png",
				PlayerRole::Digger);
		}
	}
	else if (m_Mode == GameMode::Versus)
	{
		if (m_PlayerAlive[1])
		{
			const glm::ivec2 p2Spawn =
				data.hasPlayer2Spawn
				? data.player2Spawn
				: data.enemySpawn;

			SpawnDiggerPlayer(
				1,
				p2Spawn,
				"Nobbin.png",
				PlayerRole::VersusEnemy);
		}
	}

	if (m_Mode == GameMode::Versus)
	{
		if (m_EnemyManager)
			m_EnemyManager->Clear();
	}

	m_DirtTiles.clear();

	auto dirtTexture =
		dae::ResourceManager::GetInstance().LoadTexture("Dirt.png");

	for (int y = 0; y < data.height; ++y)
	{
		for (int x = 0; x < data.width; ++x)
		{
			if (data.tiles[y][x] != '#' && data.tiles[y][x] != 'X')
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

	for (const auto& bagPos : data.moneyBags)
	{
		auto bag = std::make_unique<dae::GameObject>();
		auto* bagPtr = bag.get();

		bag->AddComponent<dae::TransformComponent>(bagPtr);


		bag->AddComponent<dae::RenderComponent>(
			bagPtr,
			dae::ResourceManager::GetInstance().LoadTexture("Money.PNG"));

		auto* bagComp = bag->AddComponent<MoneyBagComponent>(
			bagPtr,
			this,
			m_TileSize,
			m_MapOffset);

		bagComp->SetGridPosition(bagPos);

		RegisterMoneyBag(bagComp);

		m_LevelObjects.push_back(bagPtr);
		m_Scene->Add(std::move(bag));
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

	if (m_EnemyManager)
		m_EnemyManager->SetSpawn(data.enemySpawn, data.hasEnemySpawn);
}



std::string digger::GameManagerComponent::MakeTileKey(const glm::ivec2& pos) const
{
	return std::to_string(pos.x) + "," + std::to_string(pos.y);
}






//Player and Enemy Movement Checks

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



bool digger::GameManagerComponent::CanEnemyMoveTo(const glm::ivec2& pos, bool canDig) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return false;

	const char tile = m_LevelData.tiles[pos.y][pos.x];

	if (tile == 'X')
		return false;

	if (tile == '#')
		return canDig;

	return true;
}






//Digging Logic

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



void digger::GameManagerComponent::DigAtPoint(const glm::vec2& point)
{
	const int gridX = static_cast<int>(
		std::floor((point.x - m_MapOffset.x) / static_cast<float>(m_TileSize)));

	const int gridY = static_cast<int>(
		std::floor((point.y - m_MapOffset.y) / static_cast<float>(m_TileSize)));

	const glm::ivec2 tilePos{ gridX, gridY };


	if (HasMoneyBagAt(tilePos))
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
		static_cast<int>(DirtPieceCount * 0.90f);

	if (removedCount >= requiredRemoved)
	{

		m_LevelData.tiles[tilePos.y][tilePos.x] = '.';
	}
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



//Level Information Getters


bool digger::GameManagerComponent::IsDirt(const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return false;

	return m_LevelData.tiles[pos.y][pos.x] == '#';
}

bool digger::GameManagerComponent::IsSolidWall(const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return true;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return true;

	return m_LevelData.tiles[pos.y][pos.x] == 'X';
}


bool digger::GameManagerComponent::IsSolidForMoneyBag(
	const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return true;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return true;

	const char tile = m_LevelData.tiles[pos.y][pos.x];

	if (tile == '#' || tile == 'X')
		return true;

	if (HasMoneyBagAt(pos))
		return true;

	return false;
}


bool digger::GameManagerComponent::HasDirtBelow(const glm::ivec2& pos) const
{
	const glm::ivec2 below{ pos.x, pos.y + 1 };

	if (below.y >= m_LevelData.height)
		return true;

	return IsDirt(below);
}



