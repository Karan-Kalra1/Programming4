#include "MoneyBagManager.h"

#include "GameManagerComponent.h"
#include "MoneyBagComponent.h"
#include "Scene.h"
#include "GameObject.h"

#include <algorithm>

digger::MoneyBagManager::MoneyBagManager(dae::Scene* scene)
	: m_Scene(scene)
{
}

void digger::MoneyBagManager::Register(MoneyBagComponent* bag)
{
	if (!bag)
		return;

	if (std::find(m_MoneyBags.begin(), m_MoneyBags.end(), bag) != m_MoneyBags.end())
		return;

	m_MoneyBags.push_back(bag);
}

void digger::MoneyBagManager::Unregister(MoneyBagComponent* bag)
{
	m_MoneyBags.erase(
		std::remove(m_MoneyBags.begin(), m_MoneyBags.end(), bag),
		m_MoneyBags.end());
}

void digger::MoneyBagManager::Clear()
{
	m_MoneyBags.clear();
}

bool digger::MoneyBagManager::HasMoneyBagAt(const glm::ivec2& pos) const
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

bool digger::MoneyBagManager::CanMoveTo(
	const GameManagerComponent& game,
	const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.x >= game.GetLevelWidth() || pos.y >= game.GetLevelHeight())
		return false;

	if (game.IsDirt(pos))
		return false;

	if (game.IsSolidWall(pos))
		return false;

	if (game.GetPlayerIndexAtGridPosition(pos) != -1)
		return false;

	if (game.HasEnemyAt(pos))
		return false;

	if (HasMoneyBagAt(pos))
		return false;

	return true;
}

bool digger::MoneyBagManager::TryPushAt(
	GameManagerComponent& game,
	const glm::ivec2& bagPos,
	const glm::ivec2& direction)
{
	if (direction.y != 0)
		return false;

	const glm::ivec2 targetPos = bagPos + direction;

	if (!CanMoveTo(game, targetPos))
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

void digger::MoneyBagManager::CollectGold(
	GameManagerComponent& game,
	dae::GameObject* goldObject)
{
	if (!goldObject)
		return;

	game.AddScore(250);

	for (auto* bag : m_MoneyBags)
	{
		if (!bag)
			continue;

		if (bag->GetGameObject() == goldObject)
		{
			Unregister(bag);
			break;
		}
	}

	game.RemoveLevelObject(goldObject);

	if (m_Scene)
		m_Scene->Remove(*goldObject);
}