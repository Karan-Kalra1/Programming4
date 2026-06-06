#pragma once

#include <vector>

namespace dae
{
	class Scene;
	class GameObject;
	class TextComponent;
}

namespace digger
{
	enum class MenuResult
	{
		None,
		ShowModeSelect,
		Quit,
		StartSinglePlayer,
		StartCoop,
		StartVersus
	};

	class MenuController final
	{
	public:
		explicit MenuController(dae::Scene* scene);

		void ShowStartMenu();
		void ShowModeSelectMenu();

		void Navigate(int delta);
		MenuResult Confirm();

		void Clear();

	private:
		enum class MenuScreen
		{
			Start,
			ModeSelect
		};

		void RefreshStartMenuText();
		void RefreshModeSelectText();

		dae::Scene* m_Scene{};

		MenuScreen m_CurrentScreen{ MenuScreen::Start };

		std::vector<dae::GameObject*> m_Objects{};

		dae::TextComponent* m_StartMenuTitleText{};
		dae::TextComponent* m_StartMenuPlayText{};
		dae::TextComponent* m_StartMenuQuitText{};

		dae::TextComponent* m_ModeMenuTitleText{};
		dae::TextComponent* m_ModeSingleText{};
		dae::TextComponent* m_ModeCoopText{};
		dae::TextComponent* m_ModeVersusText{};

		int m_StartMenuIndex{};
		int m_ModeMenuIndex{};
	};
}