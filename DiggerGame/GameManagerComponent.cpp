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
	player->AddComponent<GridPositionComponent>(playerPtr, m_TileSize)
		->SetGridPosition(data.playerSpawn);

	auto* playerTr = playerPtr->GetComponent<dae::TransformComponent>();
	playerTr->SetLocalScale(0.07f, 0.07f);

	player->AddComponent<dae::RenderComponent>(
		playerPtr,
		dae::ResourceManager::GetInstance().LoadTexture("Digger.png"));

	auto* actor = player->AddComponent<dae::GameActorComponent>(playerPtr, m_Lives, m_Score, 1);
	dae::EventBus::GetInstance().GetSubject().AddObserver(actor);

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

			auto dirt = std::make_unique<dae::GameObject>();
			auto* dirtPtr = dirt.get();

			auto* transform =
				dirt->AddComponent<dae::TransformComponent>(dirtPtr);

			transform->SetLocalPosition(
				static_cast<float>(x * m_TileSize),
				static_cast<float>(y * m_TileSize));

			transform->SetLocalScale(.065f, .065f);

			dirt->AddComponent<dae::RenderComponent>(dirtPtr, dirtTexture);

			m_DirtTiles[MakeTileKey({ x, y })] = dirtPtr;
			m_LevelObjects.push_back(dirtPtr);
			m_Scene->Add(std::move(dirt));
		}
	}


	for (const auto& diamondPos : data.diamonds)
	{
		auto diamond = std::make_unique<dae::GameObject>();
		auto* diamondPtr = diamond.get();

		diamond->AddComponent<dae::TransformComponent>(diamondPtr);
		diamond->AddComponent<GridPositionComponent>(diamondPtr, m_TileSize)
			->SetGridPosition(diamondPos);

		auto* tr = diamondPtr->GetComponent<dae::TransformComponent>();
		tr->SetLocalScale(0.055f, 0.055f);

		diamond->AddComponent<dae::RenderComponent>(
			diamondPtr,
			dae::ResourceManager::GetInstance().LoadTexture("Diamond.png"));

		diamond->AddComponent<DiamondComponent>(diamondPtr, this);

		m_Diamonds.push_back(diamondPtr);
		m_LevelObjects.push_back(diamondPtr);
		m_Scene->Add(std::move(diamond));
	}

	for (const auto& enemyPos : data.enemies)
	{
		auto enemy = std::make_unique<dae::GameObject>();
		auto* enemyPtr = enemy.get();

		enemy->AddComponent<dae::TransformComponent>(enemyPtr);

		enemy->AddComponent<GridPositionComponent>(enemyPtr, m_TileSize)
			->SetGridPosition(enemyPos);

		if (auto* tr = enemyPtr->GetComponent<dae::TransformComponent>())
			tr->SetLocalScale(0.05f, 0.05f);

		enemy->AddComponent<dae::RenderComponent>(
			enemyPtr,
			dae::ResourceManager::GetInstance().LoadTexture("Nobbin.png"));

		auto* enemyComp = enemy->AddComponent<EnemyComponent>(enemyPtr, this);
		RegisterEnemy(enemyComp);
		m_LevelObjects.push_back(enemyPtr);
		m_Scene->Add(std::move(enemy));
	}
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

	m_LevelData.tiles[pos.y][pos.x] = '.';

	const auto key = MakeTileKey(pos);
	auto it = m_DirtTiles.find(key);

	if (it != m_DirtTiles.end())
	{
		auto* dirtObject = it->second;

		if (auto* transform = dirtObject->GetComponent<dae::TransformComponent>())
			transform->SetLocalPosition(-1000.f, -1000.f);

		m_DirtTiles.erase(it);
	}
}

glm::ivec2 digger::GameManagerComponent::GetPlayerGridPosition() const
{
	if (!m_Player)
		return {};

	auto* grid = m_Player->GetComponent<GridPositionComponent>();
	if (!grid)
		return {};

	return grid->GetGridPosition();
}

void digger::GameManagerComponent::DamagePlayer()
{
	if (!m_Player)
		return;

	dae::EventBus::GetInstance().GetSubject().Notify(
		dae::Event{ dae::EventType::DamageRequested, m_Player, 1 });

	auto* grid = m_Player->GetComponent<GridPositionComponent>();
	if (grid)
		grid->SetGridPosition(m_PlayerSpawn);
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