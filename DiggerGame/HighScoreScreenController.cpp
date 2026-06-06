#include "HighScoreScreenController.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "TextComponent.h"

#include <memory>
#include <string>

digger::HighScoreScreenController::HighScoreScreenController(
	dae::Scene* scene,
	std::filesystem::path highScoreFile)
	: m_Scene(scene)
	, m_HighScores(std::move(highScoreFile))
{
}

void digger::HighScoreScreenController::LoadScores()
{
	m_HighScores.Load();
}

void digger::HighScoreScreenController::ShowEntryScreen(int score)
{
	Clear();

	if (!m_Scene)
		return;

	m_ShowingList = false;
	m_CurrentScore = score;
	m_CurrentInitials = "AAA";
	m_InitialIndex = 0;

	auto fontBig =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);

	auto fontSmall =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 24);

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

	addText("GAME OVER", 320.f, 70.f, fontBig);

	addText(
		"Final Score: " + std::to_string(m_CurrentScore),
		300.f,
		130.f,
		fontSmall);

	addText("ENTER NAME", 280.f, 210.f, fontSmall);

	m_EntryText = addText("AAA", 355.f, 255.f, fontSmall);
	m_CursorText = addText("^", 355.f, 285.f, fontSmall);

	addText("UP/DOWN: Change Letter", 180.f, 340.f, fontSmall);
	addText("LEFT/RIGHT: Select Letter", 180.f, 375.f, fontSmall);
	addText("ENTER / A: Confirm", 180.f, 410.f, fontSmall);

	RefreshEntryText();
}

void digger::HighScoreScreenController::ShowHighScoreList()
{
	Clear();

	if (!m_Scene)
		return;

	m_ShowingList = true;

	auto fontTitle =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);

	auto fontHeader =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 24);

	auto fontRow =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 22);

	auto addText = [&](const std::string& text, float x, float y, auto font)
		{
			auto obj = std::make_unique<dae::GameObject>();
			auto* go = obj.get();

			go->AddComponent<dae::TransformComponent>(go)
				->SetLocalPosition(x, y);

			go->AddComponent<dae::TextComponent>(
				go,
				text,
				font);

			m_Objects.push_back(go);
			m_Scene->Add(std::move(obj));
		};

	addText("HIGH SCORES", 310.f, 50.f, fontTitle);

	addText("RANK", 220.f, 130.f, fontHeader);
	addText("NAME", 370.f, 130.f, fontHeader);
	addText("SCORE", 520.f, 130.f, fontHeader);

	const auto& scores = m_HighScores.GetScores();

	const float startY = 175.f;
	const float rowSpacing = 32.f;

	for (int i = 0; i < 10; ++i)
	{
		const float y = startY + static_cast<float>(i) * rowSpacing;

		std::string rankText = std::to_string(i + 1) + ".";
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

	addText("PRESS ENTER / A TO RETURN", 245.f, 520.f, fontHeader);
}

void digger::HighScoreScreenController::ChangeLetter(int delta)
{
	if (m_ShowingList)
		return;

	char& c = m_CurrentInitials[static_cast<size_t>(m_InitialIndex)];

	int index = c - 'A';
	index += delta;

	if (index < 0)
		index = 25;
	else if (index > 25)
		index = 0;

	c = static_cast<char>('A' + index);

	RefreshEntryText();
}

void digger::HighScoreScreenController::MoveCursor(int delta)
{
	if (m_ShowingList)
		return;

	m_InitialIndex += delta;

	if (m_InitialIndex < 0)
		m_InitialIndex = 2;
	else if (m_InitialIndex > 2)
		m_InitialIndex = 0;

	RefreshEntryText();
}

digger::HighScoreScreenResult digger::HighScoreScreenController::Confirm()
{
	if (m_ShowingList)
		return HighScoreScreenResult::ReturnToMenu;

	if (m_InitialIndex < 2)
	{
		++m_InitialIndex;
		RefreshEntryText();
		return HighScoreScreenResult::None;
	}

	SubmitCurrentScore();
	ShowHighScoreList();

	return HighScoreScreenResult::Submitted;
}

void digger::HighScoreScreenController::RefreshEntryText()
{
	if (m_EntryText)
		m_EntryText->SetText(m_CurrentInitials);

	if (m_CursorText)
	{
		std::string cursor = "   ";
		cursor[static_cast<size_t>(m_InitialIndex)] = '^';
		m_CursorText->SetText(cursor);
	}
}

void digger::HighScoreScreenController::SubmitCurrentScore()
{
	m_HighScores.AddScore(m_CurrentInitials, m_CurrentScore);
	m_HighScores.Save();
}

void digger::HighScoreScreenController::Clear()
{
	if (!m_Scene)
		return;

	for (auto* object : m_Objects)
	{
		if (object)
			m_Scene->Remove(*object);
	}

	m_Objects.clear();

	m_EntryText = nullptr;
	m_CursorText = nullptr;
}