#include "GameManagerComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "TextComponent.h"

#include <memory>
#include <string>


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
	auto fontBig =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);

	auto fontSmall =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 24);

	// GAME OVER
	{
		auto obj = std::make_unique<dae::GameObject>();
		auto* go = obj.get();

		go->AddComponent<dae::TransformComponent>(go)
			->SetLocalPosition(320.f, 70.f);

		go->AddComponent<dae::TextComponent>(
			go,
			"GAME OVER",
			fontBig);

		m_ScreenUIObjects.push_back(go);
		m_Scene->Add(std::move(obj));
	}

	// Final score
	{
		auto obj = std::make_unique<dae::GameObject>();
		auto* go = obj.get();

		go->AddComponent<dae::TransformComponent>(go)
			->SetLocalPosition(300.f, 130.f);

		go->AddComponent<dae::TextComponent>(
			go,
			"Final Score: " + std::to_string(m_Score),
			fontSmall);

		m_ScreenUIObjects.push_back(go);
		m_Scene->Add(std::move(obj));
	}

	// "Enter Name"
	{
		auto obj = std::make_unique<dae::GameObject>();
		auto* go = obj.get();

		go->AddComponent<dae::TransformComponent>(go)
			->SetLocalPosition(280.f, 210.f);

		go->AddComponent<dae::TextComponent>(
			go,
			"ENTER NAME",
			fontSmall);

		m_ScreenUIObjects.push_back(go);
		m_Scene->Add(std::move(obj));
	}

	// Initials line
	{
		auto obj = std::make_unique<dae::GameObject>();
		auto* go = obj.get();

		go->AddComponent<dae::TransformComponent>(go)
			->SetLocalPosition(355.f, 255.f);

		m_HighScoreEntryText =
			go->AddComponent<dae::TextComponent>(
				go,
				"AAA",
				fontSmall);

		m_ScreenUIObjects.push_back(go);
		m_Scene->Add(std::move(obj));
	}

	// Cursor line
	{
		auto obj = std::make_unique<dae::GameObject>();
		auto* go = obj.get();

		go->AddComponent<dae::TransformComponent>(go)
			->SetLocalPosition(355.f, 285.f);

		m_HighScoreCursorText =
			go->AddComponent<dae::TextComponent>(
				go,
				"^",
				fontSmall);

		m_ScreenUIObjects.push_back(go);
		m_Scene->Add(std::move(obj));
	}

	// Instructions line 1
	{
		auto obj = std::make_unique<dae::GameObject>();
		auto* go = obj.get();

		go->AddComponent<dae::TransformComponent>(go)
			->SetLocalPosition(180.f, 340.f);

		go->AddComponent<dae::TextComponent>(
			go,
			"UP/DOWN: Change Letter",
			fontSmall);

		m_ScreenUIObjects.push_back(go);
		m_Scene->Add(std::move(obj));
	}

	// Instructions line 2
	{
		auto obj = std::make_unique<dae::GameObject>();
		auto* go = obj.get();

		go->AddComponent<dae::TransformComponent>(go)
			->SetLocalPosition(180.f, 375.f);

		go->AddComponent<dae::TextComponent>(
			go,
			"LEFT/RIGHT: Select Letter",
			fontSmall);

		m_ScreenUIObjects.push_back(go);
		m_Scene->Add(std::move(obj));
	}

	// Instructions line 3
	{
		auto obj = std::make_unique<dae::GameObject>();
		auto* go = obj.get();

		go->AddComponent<dae::TransformComponent>(go)
			->SetLocalPosition(180.f, 410.f);

		go->AddComponent<dae::TextComponent>(
			go,
			"ENTER / A: Confirm",
			fontSmall);

		m_ScreenUIObjects.push_back(go);
		m_Scene->Add(std::move(obj));
	}

	m_CurrentInitials = "AAA";
	m_InitialIndex = 0;

	RefreshHighScoreEntryText();
}



void digger::GameManagerComponent::RefreshHighScoreEntryText()
{
	if (m_HighScoreEntryText)
		m_HighScoreEntryText->SetText(m_CurrentInitials);

	if (m_HighScoreCursorText)
	{
		std::string cursor = "   ";
		cursor[static_cast<size_t>(m_InitialIndex)] = '^';
		m_HighScoreCursorText->SetText(cursor);
	}
}

void digger::GameManagerComponent::ChangeHighScoreLetter(int delta)
{
	if (m_ScreenState != GameScreenState::EnteringHighScore)
		return;

	char& c = m_CurrentInitials[static_cast<size_t>(m_InitialIndex)];

	int index = c - 'A';
	index += delta;

	if (index < 0)
		index = 25;
	else if (index > 25)
		index = 0;

	c = static_cast<char>('A' + index);

	RefreshHighScoreEntryText();
}

void digger::GameManagerComponent::MoveHighScoreCursor(int delta)
{
	if (m_ScreenState != GameScreenState::EnteringHighScore)
		return;

	m_InitialIndex += delta;

	if (m_InitialIndex < 0)
		m_InitialIndex = 2;
	else if (m_InitialIndex > 2)
		m_InitialIndex = 0;

	RefreshHighScoreEntryText();
}

void digger::GameManagerComponent::ConfirmHighScoreLetter()
{
	if (m_ScreenState != GameScreenState::EnteringHighScore)
		return;

	if (m_InitialIndex < 2)
	{
		++m_InitialIndex;
		RefreshHighScoreEntryText();
		return;
	}

	SubmitHighScore();
}



void digger::GameManagerComponent::SubmitHighScore()
{
	m_HighScores.AddScore(m_CurrentInitials, m_Score);
	m_HighScores.Save();

	ShowHighScoreList();
}

void digger::GameManagerComponent::ShowHighScoreList()
{
	m_ScreenState = GameScreenState::ShowingHighScores;

	ClearHUD();
	ClearScreenUI();

	auto fontTitle =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);

	auto fontHeader =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 24);

	auto fontRow =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 22);

	auto addText = [&](const std::string& text, float x, float y, std::shared_ptr<dae::Font> font)
		{
			auto obj = std::make_unique<dae::GameObject>();
			auto* go = obj.get();

			go->AddComponent<dae::TransformComponent>(go)
				->SetLocalPosition(x, y);

			go->AddComponent<dae::TextComponent>(
				go,
				text,
				font);

			m_ScreenUIObjects.push_back(go);
			m_Scene->Add(std::move(obj));
		};

	// Title
	addText("HIGH SCORES", 310.f, 50.f, fontTitle);

	// Column headers
	addText("RANK", 220.f, 130.f, fontHeader);
	addText("NAME", 370.f, 130.f, fontHeader);
	addText("SCORE", 520.f, 130.f, fontHeader);

	const auto& scores = m_HighScores.GetScores();

	const float startY = 175.f;
	const float rowSpacing = 32.f;

	for (int i = 0; i < 10; ++i)
	{
		const float y = startY + static_cast<float>(i) * rowSpacing;

		const std::string rankText = std::to_string(i + 1) + ".";

		std::string nameText = "---";
		std::string scoreText = "0";

		if (i < static_cast<int>(scores.size()))
		{
			nameText = scores[static_cast<size_t>(i)].name;
			scoreText = std::to_string(scores[static_cast<size_t>(i)].score);
		}

		addText(rankText, 240.f, y, fontRow);
		addText(nameText, 385.f, y, fontRow);
		addText(scoreText, 535.f, y, fontRow);
	}

	// Restart instruction
	addText("PRESS ENTER / A TO RESTART", 245.f, 520.f, fontHeader);
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
		ShowStartMenu();
		return;
	}
}