#include "HighScoreCursorCommand.h"
#include "GameManagerComponent.h"

void digger::HighScoreCursorCommand::Execute()
{
	if (m_Manager)
		m_Manager->MoveHighScoreCursor(m_Delta);
}