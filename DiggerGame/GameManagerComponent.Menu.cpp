#include "GameManagerComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "TextComponent.h"

#include <SDL3/SDL.h>

#include <memory>

//Display the Menu
void digger::GameManagerComponent::ShowStartMenu()
{
	ClearLevel();
	ClearHUD();
	ClearScreenUI();

	m_ScreenState = GameScreenState::StartMenu;
	m_StartMenuIndex = 0;

	auto fontTitle =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 48);

	auto fontMenu =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 32);

	auto addText = [&](const std::string& text, float x, float y, std::shared_ptr<dae::Font> font)
		{
			auto obj = std::make_unique<dae::GameObject>();
			auto* go = obj.get();

			go->AddComponent<dae::TransformComponent>(go)
				->SetLocalPosition(x, y);

			auto* textComp =
				go->AddComponent<dae::TextComponent>(go, text, font);

			m_ScreenUIObjects.push_back(go);
			m_Scene->Add(std::move(obj));

			return textComp;
		};

	m_StartMenuTitleText = addText("DIGGER", 365.f, 100.f, fontTitle);
	m_StartMenuPlayText = addText("", 390.f, 230.f, fontMenu);
	m_StartMenuQuitText = addText("", 390.f, 285.f, fontMenu);

	RefreshStartMenuText();
}

void digger::GameManagerComponent::ShowModeSelectMenu()
{
	ClearScreenUI();

	m_ScreenState = GameScreenState::ModeSelect;
	m_ModeMenuIndex = 0;

	auto fontTitle =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 40);

	auto fontMenu =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 28);

	auto addText = [&](const std::string& text, float x, float y, std::shared_ptr<dae::Font> font)
		{
			auto obj = std::make_unique<dae::GameObject>();
			auto* go = obj.get();

			go->AddComponent<dae::TransformComponent>(go)
				->SetLocalPosition(x, y);

			auto* textComp =
				go->AddComponent<dae::TextComponent>(go, text, font);

			m_ScreenUIObjects.push_back(go);
			m_Scene->Add(std::move(obj));

			return textComp;
		};

	m_ModeMenuTitleText = addText("SELECT MODE", 310.f, 100.f, fontTitle);
	m_ModeSingleText = addText("", 330.f, 210.f, fontMenu);
	m_ModeCoopText = addText("", 330.f, 260.f, fontMenu);
	m_ModeVersusText = addText("", 330.f, 310.f, fontMenu);

	RefreshModeSelectText();
}




//Refresh the Menu UI

void digger::GameManagerComponent::RefreshModeSelectText()
{
	if (m_ModeSingleText)
	{
		m_ModeSingleText->SetText(
			m_ModeMenuIndex == 0 ? "> SINGLE PLAYER" : "  SINGLE PLAYER");
	}

	if (m_ModeCoopText)
	{
		m_ModeCoopText->SetText(
			m_ModeMenuIndex == 1 ? "> COOP" : "  COOP");
	}

	if (m_ModeVersusText)
	{
		m_ModeVersusText->SetText(
			m_ModeMenuIndex == 2 ? "> VERSUS" : "  VERSUS");
	}
}


void digger::GameManagerComponent::RefreshStartMenuText()
{
	if (m_StartMenuPlayText)
	{
		m_StartMenuPlayText->SetText(
			m_StartMenuIndex == 0 ? "> PLAY" : "  PLAY");
	}

	if (m_StartMenuQuitText)
	{
		m_StartMenuQuitText->SetText(
			m_StartMenuIndex == 1 ? "> QUIT" : "  QUIT");
	}
}




//Navigate the Start Menu

void digger::GameManagerComponent::NavigateMenu(int delta)
{
	if (m_ScreenState == GameScreenState::StartMenu)
	{
		m_StartMenuIndex += delta;

		if (m_StartMenuIndex < 0)
			m_StartMenuIndex = 1;
		else if (m_StartMenuIndex > 1)
			m_StartMenuIndex = 0;

		RefreshStartMenuText();
		return;
	}

	if (m_ScreenState == GameScreenState::ModeSelect)
	{
		m_ModeMenuIndex += delta;

		if (m_ModeMenuIndex < 0)
			m_ModeMenuIndex = 2;
		else if (m_ModeMenuIndex > 2)
			m_ModeMenuIndex = 0;

		RefreshModeSelectText();
		return;
	}
}

void digger::GameManagerComponent::ConfirmMenuSelection()
{
	if (m_ScreenState == GameScreenState::StartMenu)
	{
		if (m_StartMenuIndex == 0)
		{
			ShowModeSelectMenu();
		}
		else
		{
			QuitGame();
		}

		return;
	}

	if (m_ScreenState == GameScreenState::ModeSelect)
	{
		if (m_ModeMenuIndex == 0)
			StartGame(GameMode::SinglePlayer);
		else if (m_ModeMenuIndex == 1)
			StartGame(GameMode::Coop);
		else if (m_ModeMenuIndex == 2)
			StartGame(GameMode::Versus);

		return;
	}
}




//Quitting and clearing UI
void digger::GameManagerComponent::ClearScreenUI()
{
	for (auto* object : m_ScreenUIObjects)
	{
		if (object)
			m_Scene->Remove(*object);
	}

	m_ScreenUIObjects.clear();

	m_StartMenuTitleText = nullptr;
	m_StartMenuPlayText = nullptr;
	m_StartMenuQuitText = nullptr;

	m_ModeMenuTitleText = nullptr;
	m_ModeSingleText = nullptr;
	m_ModeCoopText = nullptr;
	m_ModeVersusText = nullptr;

	m_HighScoreEntryText = nullptr;
	m_HighScoreCursorText = nullptr;
	m_HighScoreListText = nullptr;
}

void digger::GameManagerComponent::QuitGame()
{
	SDL_Event quitEvent{};
	quitEvent.type = SDL_EVENT_QUIT;
	SDL_PushEvent(&quitEvent);
}
