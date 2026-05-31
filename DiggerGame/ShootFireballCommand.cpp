#include "ShootFireballCommand.h"
#include "GameManagerComponent.h"

void digger::ShootFireballCommand::Execute()
{
	if (m_Manager)
		m_Manager->ShootFireball();
}