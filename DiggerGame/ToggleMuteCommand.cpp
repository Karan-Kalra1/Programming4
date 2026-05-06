#include "ToggleMuteCommand.h"
#include "GameManagerComponent.h"

void digger::ToggleMuteCommand::Execute()
{
	if (m_Manager)
		m_Manager->ToggleMute();
}