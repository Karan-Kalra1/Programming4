
#include "GameManagerComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "EnemyComponent.h"
#include "MiniginTime.h"

#include <algorithm>
#include <memory>
#include <vector>

//Enemy Spawning and Collision Logic

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


void digger::GameManagerComponent::SpawnEnemy()
{
	if (m_Mode != GameMode::Versus)
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

	m_Score += 250;
	UpdateHUD();
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



//Enemy Location Getters


bool digger::GameManagerComponent::HasEnemyAt(const glm::ivec2& pos) const
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

digger::EnemyComponent* digger::GameManagerComponent::GetEnemyAtWorldPosition(
	const glm::vec2& worldPos,
	float radius) const
{
	const float radiusSq = radius * radius;

	for (auto* enemy : m_Enemies)
	{
		if (!enemy || enemy->IsDead())
			continue;

		auto* tr = enemy->GetGameObject()->GetComponent<dae::TransformComponent>();
		if (!tr)
			continue;

		const auto& pos3 = tr->GetWorldPosition();
		glm::vec2 enemyCenter{ pos3.x + 32.f, pos3.y + 32.f };

		const glm::vec2 diff = enemyCenter - worldPos;

		if ((diff.x * diff.x + diff.y * diff.y) <= radiusSq)
			return enemy;
	}

	return nullptr;
}


std::vector<digger::EnemyComponent*> digger::GameManagerComponent::GetEnemiesAtWorldPosition(
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