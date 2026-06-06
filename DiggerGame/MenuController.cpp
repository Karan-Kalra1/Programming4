#include "MenuController.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "TextComponent.h"

#include <memory>
#include <string>

digger::MenuController::MenuController(dae::Scene* scene)
	: m_Scene(scene)
{
}

void digger::MenuController::ShowStartMenu()
{
	Clear();

	if (!m_Scene)
		return;

	m_CurrentScreen = MenuScreen::Start;
	m_StartMenuIndex = 0;

	auto fontTitle =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 48);

	auto fontMenu =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 32);

	auto addText = [&](const std::string& text, float x, float y, auto font)
		{
			auto obj = std::make_unique<dae::GameObject>();
			auto* go = obj.get();

			go->AddComponent<dae::TransformComponent>(go)
				->SetLocalPosition(x, y);

			auto* textComp =
				go->AddComponent<dae::TextComponent>(go, text, font);

			m_Objects.push_back(go);
			m_Scene->Add(std::move(obj));

			return textComp;
		};

	m_StartMenuTitleText = addText("DIGGER", 365.f, 100.f, fontTitle);
	m_StartMenuPlayText = addText("", 390.f, 230.f, fontMenu);
	m_StartMenuQuitText = addText("", 390.f, 285.f, fontMenu);

	RefreshStartMenuText();
}

void digger::MenuController::ShowModeSelectMenu()
{
	Clear();

	if (!m_Scene)
		return;

	m_CurrentScreen = MenuScreen::ModeSelect;
	m_ModeMenuIndex = 0;

	auto fontTitle =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 40);

	auto fontMenu =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 28);

	auto addText = [&](const std::string& text, float x, float y, auto font)
		{
			auto obj = std::make_unique<dae::GameObject>();
			auto* go = obj.get();

			go->AddComponent<dae::TransformComponent>(go)
				->SetLocalPosition(x, y);

			auto* textComp =
				go->AddComponent<dae::TextComponent>(go, text, font);

			m_Objects.push_back(go);
			m_Scene->Add(std::move(obj));

			return textComp;
		};

	m_ModeMenuTitleText = addText("SELECT MODE", 310.f, 100.f, fontTitle);
	m_ModeSingleText = addText("", 330.f, 210.f, fontMenu);
	m_ModeCoopText = addText("", 330.f, 260.f, fontMenu);
	m_ModeVersusText = addText("", 330.f, 310.f, fontMenu);

	RefreshModeSelectText();
}

void digger::MenuController::Navigate(int delta)
{
	if (m_CurrentScreen == MenuScreen::Start)
	{
		m_StartMenuIndex += delta;

		if (m_StartMenuIndex < 0)
			m_StartMenuIndex = 1;
		else if (m_StartMenuIndex > 1)
			m_StartMenuIndex = 0;

		RefreshStartMenuText();
		return;
	}

	if (m_CurrentScreen == MenuScreen::ModeSelect)
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

digger::MenuResult digger::MenuController::Confirm()
{
	if (m_CurrentScreen == MenuScreen::Start)
	{
		if (m_StartMenuIndex == 0)
			return MenuResult::ShowModeSelect;

		return MenuResult::Quit;
	}

	if (m_CurrentScreen == MenuScreen::ModeSelect)
	{
		if (m_ModeMenuIndex == 0)
			return MenuResult::StartSinglePlayer;

		if (m_ModeMenuIndex == 1)
			return MenuResult::StartCoop;

		if (m_ModeMenuIndex == 2)
			return MenuResult::StartVersus;
	}

	return MenuResult::None;
}

void digger::MenuController::RefreshStartMenuText()
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

void digger::MenuController::RefreshModeSelectText()
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

void digger::MenuController::Clear()
{
	if (!m_Scene)
		return;

	for (auto* object : m_Objects)
	{
		if (object)
			m_Scene->Remove(*object);
	}

	m_Objects.clear();

	m_StartMenuTitleText = nullptr;
	m_StartMenuPlayText = nullptr;
	m_StartMenuQuitText = nullptr;

	m_ModeMenuTitleText = nullptr;
	m_ModeSingleText = nullptr;
	m_ModeCoopText = nullptr;
	m_ModeVersusText = nullptr;
}