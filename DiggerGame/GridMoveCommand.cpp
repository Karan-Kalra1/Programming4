#include "GridMoveCommand.h"
#include "GameManagerComponent.h"
#include "GameObject.h"
#include "GridMovementComponent.h"

void digger::GridMoveCommand::Execute()
{
	if (!m_Manager)
		return;

	if (!m_Manager->IsPlayerControllable(m_PlayerIndex))
		return;

	auto* player = m_Manager->GetPlayer(m_PlayerIndex);
	if (!player)
		return;

	auto* movement = player->GetComponent<GridMovementComponent>();
	if (!movement)
		return;

	if (!m_Pressed)
	{
		movement->ReleaseDirection(m_Direction);
		return;
	}

	if (m_Pressed)
		movement->PressDirection(m_Direction);
	else
		movement->ReleaseDirection(m_Direction);
}