#include "GameManagerComponent.h"

#include "MenuController.h"
#include "ServiceLocator.h"
#include "GameSounds.h"
#include "Scene.h"
#include "HighScoreScreenController.h"
#include "DeathSequenceController.h"
#include "EnemyManager.h"

#include <SDL3/SDL.h>

//Display the Menu
void digger::GameManagerComponent::ShowStartMenu()
{
	ClearLevel();
	ClearHUD();
	ClearScreenUI();

	if (m_DeathSequenceController)
		m_DeathSequenceController->Clear();

	if (m_EnemyManager)
		m_EnemyManager->Clear();

	auto& sound = dae::ServiceLocator::GetSoundSystem();
	sound.StopAll();
	sound.PlayLooping(GameSound::BackgroundMusic, 0.5f);

	m_ScreenState = GameScreenState::StartMenu;

	if (m_MenuController)
		m_MenuController->ShowStartMenu();
}

void digger::GameManagerComponent::ShowModeSelectMenu()
{
	ClearScreenUI();

	m_ScreenState = GameScreenState::ModeSelect;

	if (m_MenuController)
		m_MenuController->ShowModeSelectMenu();
}



//Navigate the Start Menu

void digger::GameManagerComponent::NavigateMenu(int delta)
{
	if (m_ScreenState != GameScreenState::StartMenu &&
		m_ScreenState != GameScreenState::ModeSelect)
	{
		return;
	}

	if (m_MenuController)
		m_MenuController->Navigate(delta);
}

void digger::GameManagerComponent::ConfirmMenuSelection()
{
	if (m_ScreenState != GameScreenState::StartMenu &&
		m_ScreenState != GameScreenState::ModeSelect)
	{
		return;
	}

	if (!m_MenuController)
		return;

	const MenuResult result = m_MenuController->Confirm();

	switch (result)
	{
	case MenuResult::ShowModeSelect:
		ShowModeSelectMenu();
		break;

	case MenuResult::Quit:
		QuitGame();
		break;

	case MenuResult::StartSinglePlayer:
		StartGame(GameMode::SinglePlayer);
		break;

	case MenuResult::StartCoop:
		StartGame(GameMode::Coop);
		break;

	case MenuResult::StartVersus:
		StartGame(GameMode::Versus);
		break;

	case MenuResult::None:
	default:
		break;
	}
}


//Quitting and clearing UI
void digger::GameManagerComponent::ClearScreenUI()
{
	if (m_MenuController)
		m_MenuController->Clear();

	if (m_HighScoreScreenController)
		m_HighScoreScreenController->Clear();

}

void digger::GameManagerComponent::QuitGame()
{
	SDL_Event quitEvent{};
	quitEvent.type = SDL_EVENT_QUIT;
	SDL_PushEvent(&quitEvent);
}