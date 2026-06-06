#include "EnemyManager.h"

#include "GameManagerComponent.h"
#include "EnemyComponent.h"
#include "GridPositionComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "RenderComponent.h"

#include <algorithm>
#include <memory>

digger::EnemyManager::EnemyManager(dae::Scene* scene)
	: m_Scene(scene)
{
}

void digger::EnemyManager::BeginStage(GameManagerComponent& game)
{
	m_Enemies.clear();
	m_EnemiesAlive = 0;
	m_EnemySpawnTimer = 0.f;

	if (game.GetGameMode() == GameMode::Versus)
	{
		m_TotalEnemiesThisStage = 0;
		m_EnemiesRemainingToSpawn = 0;
		return;
	}

	m_TotalEnemiesThisStage = GetEnemyCountForStage(game);
	m_EnemiesRemainingToSpawn = m_TotalEnemiesThisStage;
}

bool digger::EnemyManager::Update(
	GameManagerComponent& game,
	float deltaTime)
{
	if (game.GetGameMode() == GameMode::Versus)
		return false;

	if (m_EnemiesRemainingToSpawn > 0)
	{
		m_EnemySpawnTimer += deltaTime;

		if (m_EnemySpawnTimer >= GetSpawnInterval(game))
		{
			m_EnemySpawnTimer = 0.f;
			SpawnEnemy(game);
		}
	}

	return m_EnemiesRemainingToSpawn <= 0 &&
		m_EnemiesAlive <= 0;
}

void digger::EnemyManager::Clear()
{
	m_Enemies.clear();

	m_TotalEnemiesThisStage = 0;
	m_EnemiesRemainingToSpawn = 0;
	m_EnemiesAlive = 0;
	m_EnemySpawnTimer = 0.f;

	m_EnemySpawn = {};
	m_HasEnemySpawn = false;
}

void digger::EnemyManager::SetSpawn(
	const glm::ivec2& spawn,
	bool hasSpawn)
{
	m_EnemySpawn = spawn;
	m_HasEnemySpawn = hasSpawn;
}

void digger::EnemyManager::SpawnEnemy(GameManagerComponent& game)
{
	if (game.GetGameMode() == GameMode::Versus)
		return;

	if (!m_HasEnemySpawn)
		return;

	if (m_EnemiesRemainingToSpawn <= 0)
		return;

	if (!m_Scene)
		return;

	auto enemy = std::make_unique<dae::GameObject>();
	auto* enemyPtr = enemy.get();

	enemy->AddComponent<dae::TransformComponent>(enemyPtr);


	enemy->AddComponent<dae::RenderComponent>(
		enemyPtr,
		dae::ResourceManager::GetInstance().LoadTexture("Nobbin.png"));

	auto* enemyComp =
		enemy->AddComponent<EnemyComponent>(enemyPtr, &game);

	enemyComp->SetGridPosition(m_EnemySpawn);

	RegisterEnemy(enemyComp);

	--m_EnemiesRemainingToSpawn;
	++m_EnemiesAlive;

	game.AddLevelObject(enemyPtr);

	m_Scene->Add(std::move(enemy));
}

void digger::EnemyManager::RegisterEnemy(EnemyComponent* enemy)
{
	if (!enemy)
		return;

	if (std::find(m_Enemies.begin(), m_Enemies.end(), enemy) != m_Enemies.end())
		return;

	m_Enemies.push_back(enemy);
}

void digger::EnemyManager::KillEnemy(
	GameManagerComponent& game,
	EnemyComponent* enemy)
{
	if (!enemy)
		return;

	auto* enemyObject = enemy->GetGameObject();
	if (!enemyObject)
		return;

	m_Enemies.erase(
		std::remove(m_Enemies.begin(), m_Enemies.end(), enemy),
		m_Enemies.end());

	if (m_EnemiesAlive > 0)
		--m_EnemiesAlive;

	game.RemoveLevelObject(enemyObject);

	if (m_Scene)
		m_Scene->Remove(*enemyObject);
}

void digger::EnemyManager::ResetEnemiesAfterPlayerDeath(
	GameManagerComponent& game)
{
	const int aliveEnemies = m_EnemiesAlive;

	for (auto* enemy : m_Enemies)
	{
		if (!enemy)
			continue;

		auto* enemyObject = enemy->GetGameObject();
		if (!enemyObject)
			continue;

		// Move away immediately so no same-frame collision can happen.
		if (auto* tr = enemyObject->GetComponent<dae::TransformComponent>())
			tr->SetLocalPosition(-5000.f, -5000.f);

		game.RemoveLevelObject(enemyObject);

		if (m_Scene)
			m_Scene->Remove(*enemyObject);
	}

	m_Enemies.clear();
	m_EnemiesAlive = 0;
	m_EnemySpawnTimer = 0.f;

	if (game.GetGameMode() != GameMode::Versus)
		m_EnemiesRemainingToSpawn += aliveEnemies;
}

void digger::EnemyManager::CheckEnemyCrossings()
{
	for (size_t i = 0; i < m_Enemies.size(); ++i)
	{
		for (size_t j = i + 1; j < m_Enemies.size(); ++j)
		{
			if (!m_Enemies[i] || !m_Enemies[j])
				continue;

			if (m_Enemies[i]->IsDead() || m_Enemies[j]->IsDead())
				continue;

			if (m_Enemies[i]->GetGridPosition() ==
				m_Enemies[j]->GetGridPosition())
			{
				m_Enemies[i]->RegisterEnemyCollision();
				m_Enemies[j]->RegisterEnemyCollision();
			}
		}
	}
}

bool digger::EnemyManager::HasEnemyAt(
	const glm::ivec2& pos) const
{
	for (auto* enemy : m_Enemies)
	{
		if (!enemy || enemy->IsDead())
			continue;

		if (enemy->GetGridPosition() == pos)
			return true;
	}

	return false;
}

digger::EnemyComponent* digger::EnemyManager::GetEnemyAtWorldPosition(
	const glm::vec2& worldPos,
	float radius) const
{
	const float radiusSq = radius * radius;

	for (auto* enemy : m_Enemies)
	{
		if (!enemy || enemy->IsDead())
			continue;

		auto* enemyObj = enemy->GetGameObject();
		if (!enemyObj)
			continue;

		auto* tr = enemyObj->GetComponent<dae::TransformComponent>();
		if (!tr)
			continue;

		const auto& pos3 = tr->GetWorldPosition();

		glm::vec2 enemyCenter{
			pos3.x + 32.f,
			pos3.y + 32.f
		};

		const glm::vec2 diff = enemyCenter - worldPos;

		if ((diff.x * diff.x + diff.y * diff.y) <= radiusSq)
			return enemy;
	}

	return nullptr;
}

std::vector<digger::EnemyComponent*> digger::EnemyManager::GetEnemiesAtWorldPosition(
	const glm::vec2& worldPos,
	float radius) const
{
	std::vector<EnemyComponent*> result{};

	const float radiusSq = radius * radius;

	for (auto* enemy : m_Enemies)
	{
		if (!enemy || enemy->IsDead())
			continue;

		auto* enemyObj = enemy->GetGameObject();
		if (!enemyObj)
			continue;

		auto* tr = enemyObj->GetComponent<dae::TransformComponent>();
		if (!tr)
			continue;

		const auto& pos3 = tr->GetWorldPosition();

		glm::vec2 enemyCenter{
			pos3.x + 32.f,
			pos3.y + 32.f
		};

		const glm::vec2 diff = enemyCenter - worldPos;

		if ((diff.x * diff.x + diff.y * diff.y) <= radiusSq)
			result.push_back(enemy);
	}

	return result;
}

float digger::EnemyManager::GetSpawnInterval(
	const GameManagerComponent& game) const
{
	const float baseInterval = 4.5f;
	const float levelReduction =
		static_cast<float>(game.GetCurrentLevel()) * 0.5f;

	const float interval = baseInterval - levelReduction;

	return std::max(2.5f, interval);
}

int digger::EnemyManager::GetEnemyCountForStage(
	const GameManagerComponent& game) const
{
	return 3 + game.GetCurrentLevel() * 2;
}