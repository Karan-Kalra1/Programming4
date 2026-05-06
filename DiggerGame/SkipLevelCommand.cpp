#include "SkipLevelCommand.h"
#include "GameManagerComponent.h"

void digger::SkipLevelCommand::Execute()
{
	if (m_Manager)
		m_Manager->SkipLevel();
}