#include "GridMoveCommand.h"
#include "GameManagerComponent.h"
#include "GameObject.h"
#include "GridPositionComponent.h"

digger::GridMoveCommand::GridMoveCommand(
	GameManagerComponent* manager,
	const glm::ivec2& direction)
	: m_Manager(manager)
	, m_Direction(direction)
{
}

void digger::GridMoveCommand::Execute()
{
	if (!m_Manager)
		return;

	auto* player = m_Manager->GetPlayer();
	if (!player)
		return;

	auto* grid = player->GetComponent<GridPositionComponent>();
	if (!grid)
		return;

	const glm::ivec2 nextPos = grid->GetGridPosition() + m_Direction;

	m_Manager->DigTile(nextPos);
	grid->Move(m_Direction);
}