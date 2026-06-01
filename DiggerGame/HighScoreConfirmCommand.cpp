#include "HighScoreConfirmCommand.h"
#include "GameManagerComponent.h"

void digger::HighScoreConfirmCommand::Execute()
{
	if (m_Manager)
		m_Manager->ConfirmHighScoreScreen();
}