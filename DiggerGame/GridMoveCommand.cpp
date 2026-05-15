#include "GridMoveCommand.h"
#include "GameManagerComponent.h"
#include "GameObject.h"
#include "GridMovementComponent.h"

void digger::GridMoveCommand::Execute()
{
	if (!m_Manager)
		return;

	auto* player = m_Manager->GetPlayer();
	if (!player)
		return;

	auto* movement = player->GetComponent<GridMovementComponent>();
	if (!movement)
		return;

	if (m_Pressed)
		movement->PressDirection(m_Direction);
	else
		movement->ReleaseDirection(m_Direction);
}