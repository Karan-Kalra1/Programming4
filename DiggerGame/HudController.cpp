#include "HudController.h"

#include "GameManagerComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "Texture2D.h"

#include <memory>
#include <string>

digger::HudController::HudController(dae::Scene* scene)
	: m_Scene(scene)
{
}

void digger::HudController::Create(GameManagerComponent& game)
{
	Clear();

	if (!m_Scene)
		return;

	const auto& players = game.GetPlayers();

	auto font =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 22);

	const float screenWidth = 1056.f;

	const float hudY = 10.f;
	const float iconY = -4.f;

	const float iconScale = 0.6f;
	const float iconSpacing = 48.f;

	const float p1LabelX = 16.f;
	const float p1IconsStartX = 58.f;

	const float scoreX = screenWidth * 0.5f - 80.f;

	const float p2LabelX = screenWidth - 270.f;
	const float p2IconsStartX = screenWidth - 230.f;

	// =========================
	// P1 label
	// =========================
	if (players.size() >= 1 && players[0].alive)
	{
		auto labelObj = std::make_unique<dae::GameObject>();
		auto* obj = labelObj.get();

		obj->AddComponent<dae::TransformComponent>(obj)
			->SetLocalPosition(p1LabelX, hudY);

		auto* text = obj->AddComponent<dae::TextComponent>(
			obj,
			"P1",
			font);

		m_PlayerLifeLabels.push_back(text);
		m_HUDObjects.push_back(obj);
		m_Scene->Add(std::move(labelObj));
	}

	// =========================
	// Score text
	// =========================
	{
		auto scoreObj = std::make_unique<dae::GameObject>();
		auto* obj = scoreObj.get();

		obj->AddComponent<dae::TransformComponent>(obj)
			->SetLocalPosition(scoreX, hudY);

		m_ScoreText = obj->AddComponent<dae::TextComponent>(
			obj,
			"Score: 0",
			font);

		m_HUDObjects.push_back(obj);
		m_Scene->Add(std::move(scoreObj));
	}

	// =========================
	// P2 label
	// =========================
	if (players.size() >= 2 && players[1].alive)
	{
		auto labelObj = std::make_unique<dae::GameObject>();
		auto* obj = labelObj.get();

		obj->AddComponent<dae::TransformComponent>(obj)
			->SetLocalPosition(p2LabelX, hudY);

		auto* text = obj->AddComponent<dae::TextComponent>(
			obj,
			"P2",
			font);

		m_PlayerLifeLabels.push_back(text);
		m_HUDObjects.push_back(obj);
		m_Scene->Add(std::move(labelObj));
	}

	// =========================
	// Life icons
	// =========================
	m_PlayerLifeIcons.clear();
	m_PlayerLifeIcons.resize(players.size());

	for (int playerIndex = 0;
		playerIndex < static_cast<int>(players.size());
		++playerIndex)
	{
		const auto& player =
			players[static_cast<size_t>(playerIndex)];

		if (!player.alive)
			continue;

		std::shared_ptr<dae::Texture2D> lifeTexture{};

		if (player.role == PlayerRole::VersusEnemy)
		{
			lifeTexture =
				dae::ResourceManager::GetInstance().LoadTexture("Nobbin.png");
		}
		else
		{
			lifeTexture =
				dae::ResourceManager::GetInstance().LoadTexture("Digger.png");
		}

		std::array<dae::GameObject*, 4> icons{};

		const float iconsStartX =
			playerIndex == 0 ? p1IconsStartX : p2IconsStartX;

		for (int life = 0; life < 4; ++life)
		{
			auto lifeObj = std::make_unique<dae::GameObject>();
			auto* obj = lifeObj.get();

			auto* tr = obj->AddComponent<dae::TransformComponent>(obj);

			tr->SetLocalPosition(
				iconsStartX + static_cast<float>(life) * iconSpacing,
				iconY);

			tr->SetLocalScale(iconScale, iconScale);

			obj->AddComponent<dae::RenderComponent>(obj, lifeTexture);

			icons[static_cast<size_t>(life)] = obj;

			m_HUDObjects.push_back(obj);
			m_Scene->Add(std::move(lifeObj));
		}

		m_PlayerLifeIcons[static_cast<size_t>(playerIndex)] = icons;
	}

	Update(game);
}

void digger::HudController::Update(GameManagerComponent& game)
{
	const auto& players = game.GetPlayers();
	const int score = game.GetScore();

	if (m_ScoreText)
		m_ScoreText->SetText("Score: " + std::to_string(score));

	const float screenWidth = 1056.f;

	const float iconY = -4.f;
	const float iconSpacing = 48.f;

	const float p1IconsStartX = 58.f;
	const float p2IconsStartX = screenWidth - 230.f;

	for (int playerIndex = 0;
		playerIndex < static_cast<int>(m_PlayerLifeIcons.size());
		++playerIndex)
	{
		if (playerIndex >= static_cast<int>(players.size()))
			continue;

		const auto& player =
			players[static_cast<size_t>(playerIndex)];

		auto& icons =
			m_PlayerLifeIcons[static_cast<size_t>(playerIndex)];

		const float iconsStartX =
			playerIndex == 0 ? p1IconsStartX : p2IconsStartX;

		for (int life = 0; life < 4; ++life)
		{
			auto* icon = icons[static_cast<size_t>(life)];
			if (!icon)
				continue;

			auto* tr = icon->GetComponent<dae::TransformComponent>();
			if (!tr)
				continue;

			if (player.alive && life < player.lives)
			{
				tr->SetLocalPosition(
					iconsStartX + static_cast<float>(life) * iconSpacing,
					iconY);
			}
			else
			{
				tr->SetLocalPosition(-5000.f, -5000.f);
			}
		}
	}
}

void digger::HudController::Clear()
{
	if (!m_Scene)
		return;

	for (auto* object : m_HUDObjects)
	{
		if (object)
			m_Scene->Remove(*object);
	}

	m_HUDObjects.clear();

	m_PlayerLifeIcons.clear();
	m_PlayerLifeLabels.clear();

	m_ScoreText = nullptr;
}