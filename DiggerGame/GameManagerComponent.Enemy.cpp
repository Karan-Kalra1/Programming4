
#include "GameManagerComponent.h"

#include "EnemyManager.h"



//Enemy Spawning and Collision Logic

void digger::GameManagerComponent::SpawnEnemy()
{
	if (m_EnemyManager)
		m_EnemyManager->SpawnEnemy(*this);
}

void digger::GameManagerComponent::RegisterEnemy(
	EnemyComponent* enemy)
{
	if (m_EnemyManager)
		m_EnemyManager->RegisterEnemy(enemy);
}

void digger::GameManagerComponent::KillEnemy(
	EnemyComponent* enemy)
{
	if (m_EnemyManager)
		m_EnemyManager->KillEnemy(*this, enemy);
}

void digger::GameManagerComponent::ResetEnemiesAfterPlayerDeath()
{
	if (m_EnemyManager)
		m_EnemyManager->ResetEnemiesAfterPlayerDeath(*this);
}

void digger::GameManagerComponent::CheckEnemyCrossings()
{
	if (m_EnemyManager)
		m_EnemyManager->CheckEnemyCrossings();
}


//Enemy Location Getters

bool digger::GameManagerComponent::HasEnemyAt(
	const glm::ivec2& pos) const
{
	if (!m_EnemyManager)
		return false;

	return m_EnemyManager->HasEnemyAt(pos);
}

digger::EnemyComponent* digger::GameManagerComponent::GetEnemyAtWorldPosition(
	const glm::vec2& worldPos,
	float radius) const
{
	if (!m_EnemyManager)
		return nullptr;

	return m_EnemyManager->GetEnemyAtWorldPosition(worldPos, radius);
}

std::vector<digger::EnemyComponent*> digger::GameManagerComponent::GetEnemiesAtWorldPosition(
	const glm::vec2& worldPos,
	float radius) const
{
	if (!m_EnemyManager)
		return {};

	return m_EnemyManager->GetEnemiesAtWorldPosition(worldPos, radius);
}

const std::vector<digger::EnemyComponent*>& digger::GameManagerComponent::GetEnemies() const
{
	static const std::vector<EnemyComponent*> empty{};

	if (!m_EnemyManager)
		return empty;

	return m_EnemyManager->GetEnemies();
}