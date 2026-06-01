#include "ShootFireballCommand.h"
#include "GameManagerComponent.h"

void digger::ShootFireballCommand::Execute()
{
	if (!m_Manager)
		return;

	m_Manager->ShootFireball(m_PlayerIndex);
}