#include "GameManagerComponent.h"

#include "HighScoreScreenController.h"

void digger::GameManagerComponent::GameOver()
{
	m_ScreenState = GameScreenState::EnteringHighScore;

	ClearLevel();
	ClearHUD();
	ClearScreenUI();

	ShowHighScoreEntryScreen();
}

void digger::GameManagerComponent::ShowHighScoreEntryScreen()
{
	m_ScreenState = GameScreenState::EnteringHighScore;

	if (m_HighScoreScreenController)
		m_HighScoreScreenController->ShowEntryScreen(m_Score);
}

void digger::GameManagerComponent::ChangeHighScoreLetter(int delta)
{
	if (m_ScreenState != GameScreenState::EnteringHighScore)
		return;

	if (m_HighScoreScreenController)
		m_HighScoreScreenController->ChangeLetter(delta);
}

void digger::GameManagerComponent::MoveHighScoreCursor(int delta)
{
	if (m_ScreenState != GameScreenState::EnteringHighScore)
		return;

	if (m_HighScoreScreenController)
		m_HighScoreScreenController->MoveCursor(delta);
}

void digger::GameManagerComponent::ConfirmHighScoreLetter()
{
	if (m_ScreenState != GameScreenState::EnteringHighScore)
		return;

	if (!m_HighScoreScreenController)
		return;

	const HighScoreScreenResult result =
		m_HighScoreScreenController->Confirm();

	if (result == HighScoreScreenResult::Submitted)
		m_ScreenState = GameScreenState::ShowingHighScores;
}


void digger::GameManagerComponent::SubmitHighScore()
{
	if (!m_HighScoreScreenController)
		return;

	m_HighScoreScreenController->Confirm();
	m_ScreenState = GameScreenState::ShowingHighScores;
}

void digger::GameManagerComponent::ShowHighScoreList()
{
	m_ScreenState = GameScreenState::ShowingHighScores;

	if (m_HighScoreScreenController)
		m_HighScoreScreenController->ShowHighScoreList();
}

void digger::GameManagerComponent::ConfirmHighScoreScreen()
{
	if (m_ScreenState == GameScreenState::EnteringHighScore)
	{
		ConfirmHighScoreLetter();
		return;
	}

	if (m_ScreenState == GameScreenState::ShowingHighScores)
	{
		if (m_HighScoreScreenController)
			m_HighScoreScreenController->Clear();

		ShowStartMenu();
		return;
	}
}