#include "MenuConfirmCommand.h"
#include "GameManagerComponent.h"

void digger::MenuConfirmCommand::Execute()
{
	if (m_Manager)
		m_Manager->ConfirmMenuSelection();
}