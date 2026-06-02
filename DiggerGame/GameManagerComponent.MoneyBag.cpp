#include "GameManagerComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "MoneyBagComponent.h"
#include "EnemyComponent.h"

#include <algorithm>

//Spawning/Despawning

void digger::GameManagerComponent::RegisterMoneyBag(MoneyBagComponent* bag)
{
	if (bag)
		m_MoneyBags.push_back(bag);
}


void digger::GameManagerComponent::UnregisterMoneyBag(MoneyBagComponent* bag)
{
	m_MoneyBags.erase(
		std::remove(m_MoneyBags.begin(), m_MoneyBags.end(), bag),
		m_MoneyBags.end());
}



//Positions Checks
bool digger::GameManagerComponent::CanMoneyBagMoveTo(const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return false;

	if (IsDirt(pos))
		return false;

	// Blocks P1, P2 co-op, and P2 versus enemy.
	if (GetPlayerIndexAtGridPosition(pos) != -1)
		return false;

	if (HasEnemyAt(pos))
		return false;

	if (HasMoneyBagAt(pos))
		return false;

	return true;
}

bool digger::GameManagerComponent::HasMoneyBagAt(const glm::ivec2& pos) const
{
	for (auto* bag : m_MoneyBags)
	{
		if (!bag || bag->IsBrokenGold())
			continue;

		if (bag->GetGridPosition() == pos)
			return true;
	}

	return false;
}



//Collision Checks

bool digger::GameManagerComponent::TryPushMoneyBagAt(
	const glm::ivec2& bagPos,
	const glm::ivec2& direction)
{
	const glm::ivec2 targetPos = bagPos + direction;

	if (GetPlayerIndexAtGridPosition(targetPos) != -1)
		return false;

	if (HasEnemyAt(targetPos))
		return false;

	if (HasMoneyBagAt(targetPos))
		return false;

	if (!CanMoneyBagMoveTo(targetPos))
		return false;

	for (auto* bag : m_MoneyBags)
	{
		if (!bag || bag->IsBrokenGold())
			continue;

		if (bag->GetGridPosition() == bagPos)
			return bag->TryPush(direction);
	}

	return false;
}


void digger::GameManagerComponent::CollectGold(dae::GameObject* gold)
{
	if (!gold)
		return;

	m_Score += 250;
	UpdateHUD();

	m_LevelObjects.erase(
		std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), gold),
		m_LevelObjects.end());

	m_Scene->Remove(*gold);
}




