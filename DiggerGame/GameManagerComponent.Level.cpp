#include "GameManagerComponent.h"

#include "LevelSystem.h"

#include <algorithm>

glm::vec2 digger::GameManagerComponent::GetMapOffset() const
{
	if (!m_LevelSystem)
		return {};

	return m_LevelSystem->GetMapOffset();
}

int digger::GameManagerComponent::GetTileSize() const
{
	if (!m_LevelSystem)
		return 64;

	return m_LevelSystem->GetTileSize();
}

float digger::GameManagerComponent::GetCollisionRadius() const
{
	return static_cast<float>(GetTileSize()) * 0.7f;
}

int digger::GameManagerComponent::GetLevelWidth() const
{
	if (!m_LevelSystem)
		return 0;

	return m_LevelSystem->GetWidth();
}

int digger::GameManagerComponent::GetLevelHeight() const
{
	if (!m_LevelSystem)
		return 0;

	return m_LevelSystem->GetHeight();
}

bool digger::GameManagerComponent::CanPlayerMoveTo(
	const glm::ivec2& pos) const
{
	return m_LevelSystem &&
		m_LevelSystem->CanPlayerMoveTo(pos);
}

bool digger::GameManagerComponent::CanEnemyMoveTo(
	const glm::ivec2& pos,
	bool canDig) const
{
	return m_LevelSystem &&
		m_LevelSystem->CanEnemyMoveTo(
			pos,
			canDig,
			HasDiamondAt(pos));
}

bool digger::GameManagerComponent::IsDirt(
	const glm::ivec2& pos) const
{
	return m_LevelSystem &&
		m_LevelSystem->IsDirt(pos);
}

bool digger::GameManagerComponent::IsSolidWall(
	const glm::ivec2& pos) const
{
	return !m_LevelSystem ||
		m_LevelSystem->IsSolidWall(pos);
}

bool digger::GameManagerComponent::HasDirtBelow(
	const glm::ivec2& pos) const
{
	return m_LevelSystem &&
		m_LevelSystem->HasDirtBelow(pos);
}

bool digger::GameManagerComponent::IsSolidForMoneyBag(
	const glm::ivec2& pos) const
{
	return !m_LevelSystem ||
		m_LevelSystem->IsSolidForMoneyBag(
			pos,
			HasMoneyBagAt(pos));
}

bool digger::GameManagerComponent::HasDiamondAt(
	const glm::ivec2& pos) const
{
	return m_LevelSystem &&
		m_LevelSystem->HasDiamondAt(pos);
}

void digger::GameManagerComponent::CollectDiamond(
	dae::GameObject* diamond)
{
	if (m_LevelSystem)
		m_LevelSystem->CollectDiamond(*this, diamond);
}

void digger::GameManagerComponent::DigTile(
	const glm::ivec2& pos)
{
	if (m_LevelSystem)
		m_LevelSystem->DigTile(pos);
}

void digger::GameManagerComponent::DigAtWorldPosition(
	const glm::vec2& worldPos,
	const glm::ivec2& direction)
{
	if (m_LevelSystem)
		m_LevelSystem->DigAtWorldPosition(
			*this,
			worldPos,
			direction);
}

void digger::GameManagerComponent::DigAtPoint(
	const glm::vec2& point)
{
	if (m_LevelSystem)
		m_LevelSystem->DigAtPoint(*this, point);
}

void digger::GameManagerComponent::AddLevelObject(
	dae::GameObject* object)
{
	if (!object)
		return;

	m_LevelObjects.push_back(object);
}

void digger::GameManagerComponent::RemoveLevelObject(
	dae::GameObject* object)
{
	m_LevelObjects.erase(
		std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), object),
		m_LevelObjects.end());
}