#include "LevelSystem.h"

#include "GameManagerComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "DiamondComponent.h"
#include "MoneyBagComponent.h"
#include "ServiceLocator.h"
#include "GameSounds.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

digger::LevelSystem::LevelSystem(dae::Scene* scene)
	: m_Scene(scene)
{
}

void digger::LevelSystem::SetLevelData(const LevelData& data)
{
	m_LevelData = data;
}

void digger::LevelSystem::Clear()
{
	m_DirtTiles.clear();
	m_Diamonds.clear();
}

void digger::LevelSystem::SpawnStaticObjects(GameManagerComponent& game)
{
	SpawnDirtTiles(game);
	SpawnDiamonds(game);
	SpawnMoneyBags(game);
}


void digger::LevelSystem::SpawnDirtTiles(GameManagerComponent& game)
{

	m_DirtTiles.clear();

	auto dirtTexture =
		dae::ResourceManager::GetInstance().LoadTexture("Dirt.png");

	for (int y = 0; y < m_LevelData.height; ++y)
	{
		for (int x = 0; x < m_LevelData.width; ++x)
		{
			if (m_LevelData.tiles[y][x] != '#' && m_LevelData.tiles[y][x] != 'X')
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

					game.AddLevelObject(dirtPtr);
					m_Scene->Add(std::move(dirt));
				}
			}

			m_DirtTiles[key] = dirtTile;
		}
	}

}


void digger::LevelSystem::SpawnDiamonds(GameManagerComponent& game)
{
	for (const auto& diamondGrid : m_LevelData.diamonds)
	{
		auto diamond = std::make_unique<dae::GameObject>();
		auto* diamondPtr = diamond.get();

		auto* tr = diamond->AddComponent<dae::TransformComponent>(diamondPtr);

		tr->SetLocalPosition(
			m_MapOffset.x + static_cast<float>(diamondGrid.x * m_TileSize),
			m_MapOffset.y + static_cast<float>(diamondGrid.y * m_TileSize));


		diamond->AddComponent<dae::RenderComponent>(
			diamondPtr,
			dae::ResourceManager::GetInstance().LoadTexture("Diamond.png"));

		diamond->AddComponent<digger::DiamondComponent>(
			diamondPtr,
			&game);

		m_Diamonds.push_back(diamondPtr);

		game.AddLevelObject(diamondPtr);

		if (m_Scene)
			m_Scene->Add(std::move(diamond));
	}
}

void digger::LevelSystem::SpawnMoneyBags(GameManagerComponent& game)
{
	for (const auto& bagGrid : m_LevelData.moneyBags)
	{
		auto bag = std::make_unique<dae::GameObject>();
		auto* bagPtr = bag.get();

		auto* tr = bag->AddComponent<dae::TransformComponent>(bagPtr);

		tr->SetLocalPosition(
			m_MapOffset.x + static_cast<float>(bagGrid.x * m_TileSize),
			m_MapOffset.y + static_cast<float>(bagGrid.y * m_TileSize));


		bag->AddComponent<dae::RenderComponent>(
			bagPtr,
			dae::ResourceManager::GetInstance().LoadTexture("Money.PNG"));

		auto* moneyBag =
			bag->AddComponent<MoneyBagComponent>(
				bagPtr,
				&game,
				m_TileSize,
				m_MapOffset);

		moneyBag->SetGridPosition(bagGrid);

		game.RegisterMoneyBag(moneyBag);
		game.AddLevelObject(bagPtr);

		if (m_Scene)
			m_Scene->Add(std::move(bag));
	}
}


//Level Information

std::string digger::LevelSystem::MakeTileKey(
	const glm::ivec2& pos) const
{
	return std::to_string(pos.x) + "," + std::to_string(pos.y);
}

glm::ivec2 digger::LevelSystem::WorldToGrid(
	const glm::vec2& worldPos) const
{
	const int x = static_cast<int>(
		std::floor((worldPos.x - m_MapOffset.x) / static_cast<float>(m_TileSize)));

	const int y = static_cast<int>(
		std::floor((worldPos.y - m_MapOffset.y) / static_cast<float>(m_TileSize)));

	return { x, y };
}

bool digger::LevelSystem::IsDirt(const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return false;

	return m_LevelData.tiles[pos.y][pos.x] == '#';
}

bool digger::LevelSystem::IsSolidWall(const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return true;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return true;

	return m_LevelData.tiles[pos.y][pos.x] == 'X';
}

bool digger::LevelSystem::CanPlayerMoveTo(
	const glm::ivec2& pos) const
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

bool digger::LevelSystem::CanEnemyMoveTo(
	const glm::ivec2& pos,
	bool canDig,
	bool hasDiamond) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return false;

	const char tile = m_LevelData.tiles[pos.y][pos.x];

	if (tile == 'X')
		return false;

	if (tile == '#')
	{
		if (hasDiamond)
			return false;

		return canDig;
	}

	return true;
}

bool digger::LevelSystem::HasDirtBelow(
	const glm::ivec2& pos) const
{
	const glm::ivec2 below{
		pos.x,
		pos.y + 1
	};

	return IsDirt(below) || IsSolidWall(below);
}

bool digger::LevelSystem::IsSolidForMoneyBag(
	const glm::ivec2& pos,
	bool hasMoneyBag) const
{
	if (pos.x < 0 || pos.y < 0)
		return true;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return true;

	const char tile = m_LevelData.tiles[pos.y][pos.x];

	if (tile == '#' || tile == 'X')
		return true;

	if (hasMoneyBag)
		return true;

	return false;
}



//Digging Logic

void digger::LevelSystem::DigAtWorldPosition(GameManagerComponent& game,
	const glm::vec2& worldPos,
	const glm::ivec2&)
{
	const float r = m_DigRadius;

	DigAtPoint(game,worldPos);

	DigAtPoint(game,worldPos + glm::vec2{ r, 0.f });
	DigAtPoint(game, worldPos + glm::vec2{ -r, 0.f });
	DigAtPoint(game, worldPos + glm::vec2{ 0.f, r });
	DigAtPoint(game, worldPos + glm::vec2{ 0.f, -r });

	const float diagonal = r * 0.7071f;

	DigAtPoint(game, worldPos + glm::vec2{ diagonal, diagonal });
	DigAtPoint(game, worldPos + glm::vec2{ -diagonal, diagonal });
	DigAtPoint(game, worldPos + glm::vec2{ diagonal, -diagonal });
	DigAtPoint(game, worldPos + glm::vec2{ -diagonal, -diagonal });
}



void digger::LevelSystem::DigAtPoint(GameManagerComponent& game, const glm::vec2& point)
{
	const int gridX = static_cast<int>(
		std::floor((point.x - m_MapOffset.x) / static_cast<float>(m_TileSize)));

	const int gridY = static_cast<int>(
		std::floor((point.y - m_MapOffset.y) / static_cast<float>(m_TileSize)));

	const glm::ivec2 tilePos{ gridX, gridY };


	if (game.HasMoneyBagAt(tilePos))
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


void digger::LevelSystem::DigTile(const glm::ivec2& pos)
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


//Diamond Logic

bool digger::LevelSystem::HasDiamondAt(
	const glm::ivec2& pos) const
{
	for (auto* diamond : m_Diamonds)
	{
		if (!diamond)
			continue;

		auto* tr = diamond->GetComponent<dae::TransformComponent>();
		if (!tr)
			continue;

		const auto& worldPos = tr->GetWorldPosition();

		const int x = static_cast<int>(
			std::floor((worldPos.x - m_MapOffset.x) / static_cast<float>(m_TileSize)));

		const int y = static_cast<int>(
			std::floor((worldPos.y - m_MapOffset.y) / static_cast<float>(m_TileSize)));

		if (glm::ivec2{ x, y } == pos)
			return true;
	}

	return false;
}

void digger::LevelSystem::CollectDiamond(
	GameManagerComponent& game,
	dae::GameObject* diamond)
{
	if (!diamond)
		return;

	m_Diamonds.erase(
		std::remove(m_Diamonds.begin(), m_Diamonds.end(), diamond),
		m_Diamonds.end());

	dae::ServiceLocator::GetSoundSystem().Play(
		GameSound::DiamondPickUp,
		1.0f);

	game.AddScore(100);
	game.RemoveLevelObject(diamond);

	if (m_Scene)
		m_Scene->Remove(*diamond);
}