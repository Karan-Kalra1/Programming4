#include "MenuNavigateCommand.h"
#include "GameManagerComponent.h"

void digger::MenuNavigateCommand::Execute()
{
	if (m_Manager)
		m_Manager->NavigateMenu(m_Delta);
}