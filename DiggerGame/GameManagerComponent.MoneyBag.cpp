#include "GameManagerComponent.h"

#include "MoneyBagManager.h"

//Spawning/Despawning

void digger::GameManagerComponent::RegisterMoneyBag(
	MoneyBagComponent* bag)
{
	if (m_MoneyBagManager)
		m_MoneyBagManager->Register(bag);
}

void digger::GameManagerComponent::UnregisterMoneyBag(
	MoneyBagComponent* bag)
{
	if (m_MoneyBagManager)
		m_MoneyBagManager->Unregister(bag);
}

//Positions Checks
bool digger::GameManagerComponent::HasMoneyBagAt(
	const glm::ivec2& pos) const
{
	if (!m_MoneyBagManager)
		return false;

	return m_MoneyBagManager->HasMoneyBagAt(pos);
}

bool digger::GameManagerComponent::CanMoneyBagMoveTo(
	const glm::ivec2& pos) const
{
	if (!m_MoneyBagManager)
		return false;

	return m_MoneyBagManager->CanMoveTo(*this, pos);
}

//Collision Checks

bool digger::GameManagerComponent::TryPushMoneyBagAt(
	const glm::ivec2& bagPos,
	const glm::ivec2& direction)
{
	if (!m_MoneyBagManager)
		return false;

	return m_MoneyBagManager->TryPushAt(*this, bagPos, direction);
}


void digger::GameManagerComponent::CollectGold(
	dae::GameObject* goldObject)
{
	if (m_MoneyBagManager)
		m_MoneyBagManager->CollectGold(*this, goldObject);
}

