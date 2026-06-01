#include "HighScoreLetterCommand.h"
#include "GameManagerComponent.h"

void digger::HighScoreLetterCommand::Execute()
{
	if (m_Manager)
		m_Manager->ChangeHighScoreLetter(m_Delta);
}