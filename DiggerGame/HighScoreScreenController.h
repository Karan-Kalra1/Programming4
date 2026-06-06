#pragma once

#include "HighScoreManager.h"

#include <string>
#include <vector>
#include <filesystem>

namespace dae
{
	class Scene;
	class GameObject;
	class TextComponent;
}

namespace digger
{
	enum class HighScoreScreenResult
	{
		None,
		Submitted,
		ReturnToMenu
	};

	class HighScoreScreenController final
	{
	public:
		explicit HighScoreScreenController(
			dae::Scene* scene,
			std::filesystem::path highScoreFile);

		void LoadScores();

		void ShowEntryScreen(int score);
		void ShowHighScoreList();

		void ChangeLetter(int delta);
		void MoveCursor(int delta);
		HighScoreScreenResult Confirm();

		void Clear();

	private:
		void RefreshEntryText();
		void SubmitCurrentScore();

		dae::Scene* m_Scene{};

		HighScoreManager m_HighScores;

		std::vector<dae::GameObject*> m_Objects{};

		dae::TextComponent* m_EntryText{};
		dae::TextComponent* m_CursorText{};

		std::string m_CurrentInitials{ "AAA" };
		int m_InitialIndex{};
		int m_CurrentScore{};
		bool m_ShowingList{};
	};
}