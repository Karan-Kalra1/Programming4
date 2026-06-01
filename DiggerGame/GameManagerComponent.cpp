#include "GameManagerComponent.h"

#include "LevelLoader.h"
#include "GridPositionComponent.h"
#include "DiamondComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "GameActorComponent.h"
#include "EventBus.h"
#include "ServiceLocator.h"
#include "GridMovementComponent.h"
#include "MiniginTime.h"
#include "FireballComponent.h"
#include "MoneyBagComponent.h"
#include "TextComponent.h"
#include "GameSounds.h"
#include <SDL3/SDL.h>

#include <memory>
#include <iostream>

namespace
{
	enum GameSound : dae::SoundId
	{
		DiamondPickUp = 1
	};
}

digger::GameManagerComponent::GameManagerComponent(dae::GameObject* owner, dae::Scene* scene)
	: Component(owner)
	, m_Scene(scene)
{
}

void digger::GameManagerComponent::StartGame(GameMode mode)
{
	ClearScreenUI();
	ClearHUD();
	ClearLevel();

	m_Mode = mode;
	m_Score = 0;
	m_Lives = 4;
	m_CurrentLevel = 0;

	m_ScreenState = GameScreenState::Playing;

	m_HighScores.Load();

	auto& sound = dae::ServiceLocator::GetSoundSystem();

	sound.RegisterSound(GameSound::DiamondPickUp, "Data/sounds/PickUpDiamond.wav");

	sound.RegisterSound(GameSound::BackgroundMusic, "Data/sounds/MainGameOST.wav");

	sound.RegisterSound(GameSound::MoneyBagWiggle, "Data/sounds/GoldBagWiggle.wav");
	sound.RegisterSound(GameSound::MoneyBagFalling, "Data/sounds/FallingGold.wav");

	sound.RegisterSound(GameSound::PlayerDeathSfx, "Data/sounds/DeathSound.wav");
	sound.RegisterSound(GameSound::PlayerDeathMusic, "Data/sounds/DeathMusic2.wav");

	sound.RegisterSound(GameSound::BulletTravel, "Data/sounds/BulletFiring.wav");
	sound.RegisterSound(GameSound::BulletHit, "Data/sounds/Hit.wav");

	sound.PlayLooping(GameSound::BackgroundMusic, 0.5f);

	LoadLevel(0);
	CreateHUD();
}

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

void digger::GameManagerComponent::CreateHUD()
{
	auto font =
		dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 24);

	
	// Score text

	{
		auto scoreObj = std::make_unique<dae::GameObject>();
		auto* obj = scoreObj.get();

		obj->AddComponent<dae::TransformComponent>(obj)
			->SetLocalPosition(20.f, 10.f);

		m_ScoreText = obj->AddComponent<dae::TextComponent>(
			obj,
			"Score: 0",
			font);

		m_HUDObjects.push_back(obj);
		m_Scene->Add(std::move(scoreObj));
	}

	
	// Life icons

	auto lifeTexture =
		dae::ResourceManager::GetInstance().LoadTexture("Digger.png");

	const float livesStartX = 150.f;  
	const float livesY = -10.f;          
	const float spacing = 55.f;        

	for (int i = 0; i < 4; ++i)
	{
		auto lifeObj = std::make_unique<dae::GameObject>();
		auto* obj = lifeObj.get();

		auto* tr = obj->AddComponent<dae::TransformComponent>(obj);
		tr->SetLocalPosition(
			livesStartX + static_cast<float>(i) * spacing,
			livesY);
		tr->SetLocalScale(0.70f, 0.70f);

		obj->AddComponent<dae::RenderComponent>(obj, lifeTexture);

		m_LifeIcons[i] = obj;
		m_HUDObjects.push_back(obj);
		m_Scene->Add(std::move(lifeObj));
	}

	UpdateHUD();
}

void digger::GameManagerComponent::UpdateHUD()
{
	if (m_ScoreText)
		m_ScoreText->SetText("Score: " + std::to_string(m_Score));

	const float livesStartX = 150.f;
	const float livesY = -10.f;
	const float spacing = 55.f;

	for (int i = 0; i < 4; ++i)
	{
		auto* icon = m_LifeIcons[i];
		if (!icon)
			continue;

		auto* tr = icon->GetComponent<dae::TransformComponent>();
		if (!tr)
			continue;

		if (i < m_Lives)
		{
			tr->SetLocalPosition(
				livesStartX + static_cast<float>(i) * spacing,
				livesY);
		}
		else
		{
			tr->SetLocalPosition(-5000.f, -5000.f);
		}
	}
}

void digger::GameManagerComponent::GameOver()
{
	m_ScreenState = GameScreenState::EnteringHighScore;

	ClearLevel();
	ClearHUD();
	ClearScreenUI();

	ShowHighScoreEntryScreen();
}

void digger::GameManagerComponent::QuitGame()
{
	SDL_Event quitEvent{};
	quitEvent.type = SDL_EVENT_QUIT;
	SDL_PushEvent(&quitEvent);
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


void digger::GameManagerComponent::LoadLevel(int index)
{
	m_CurrentLevel = index;

	if (m_CurrentLevel > 2)
	{
		std::cout << "Game finished!\n";
		return;
	}

	ClearLevel();

	const auto path =
		std::filesystem::path{ "Data/Levels/Level" + std::to_string(m_CurrentLevel + 1) + ".txt" };

	m_LevelData = LevelLoader::Load(path);
	SpawnLevel(m_LevelData);

	m_TotalEnemiesThisStage = 3 + m_CurrentLevel * 2;
	m_EnemiesRemainingToSpawn = m_TotalEnemiesThisStage;
	m_EnemiesAlive = 0;
	m_EnemySpawnTimer = 0.f;
}

void digger::GameManagerComponent::SkipLevel()
{
	LoadLevel(m_CurrentLevel + 1);
}

void digger::GameManagerComponent::Update()
{

	if (m_ScreenState == GameScreenState::StartMenu ||
		m_ScreenState == GameScreenState::ModeSelect ||
		m_ScreenState == GameScreenState::EnteringHighScore ||
		m_ScreenState == GameScreenState::ShowingHighScores)
	{
		return;
	}

	if (m_DamageCooldown > 0.f)
		m_DamageCooldown -= dae::MiniginTime::GetDeltaTime();

	if (m_DeathSequenceActive)
	{
		m_DeathSequenceTimer += dae::MiniginTime::GetDeltaTime();

		if (m_DeathSequenceTimer >= m_DeathSequenceDuration)
			FinishPlayerDeathSequence();

		return;
	}

	if (m_ShouldLoadNextLevel)
	{
		m_ShouldLoadNextLevel = false;
		LoadLevel(m_CurrentLevel + 1);
	}

	if (m_EnemiesRemainingToSpawn > 0)
	{
		m_EnemySpawnTimer += dae::MiniginTime::GetDeltaTime();

		if (m_EnemySpawnTimer >= m_EnemySpawnInterval)
		{
			m_EnemySpawnTimer = 0.f;
			SpawnEnemy();
		}
	}

	if (m_EnemiesRemainingToSpawn <= 0 && m_EnemiesAlive <= 0)
	{
		m_ShouldLoadNextLevel = true;
	}

	if (m_FireballCooldownTimer > 0.f)
		m_FireballCooldownTimer -= dae::MiniginTime::GetDeltaTime();

	CheckEnemyCrossings();
}

void digger::GameManagerComponent::ToggleMute()
{
	auto& sound = dae::ServiceLocator::GetSoundSystem();
	sound.SetMuted(!sound.IsMuted());
}

void digger::GameManagerComponent::CollectDiamond(dae::GameObject* diamond)
{
	if (!diamond)
		return;

	m_Score += 100;
	UpdateHUD();

	dae::ServiceLocator::GetSoundSystem().Play(DiamondPickUp, 1.0f);

	m_Scene->Remove(*diamond);

	m_Diamonds.erase(
		std::remove(m_Diamonds.begin(), m_Diamonds.end(), diamond),
		m_Diamonds.end());

	m_LevelObjects.erase(
		std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), diamond),
		m_LevelObjects.end());

	if (m_Diamonds.empty())
		m_ShouldLoadNextLevel = true;
}
void digger::GameManagerComponent::ClearLevel()
{
	if (m_PlayerActor)
	{
		dae::EventBus::GetInstance().GetSubject().RemoveObserver(m_PlayerActor);
		m_PlayerActor = nullptr;
	}

	RemoveAllFireballs();

	for (auto* object : m_LevelObjects)
	{
		if (object)
			m_Scene->Remove(*object);
	}

	m_LevelObjects.clear();
	m_Diamonds.clear();
	m_Enemies.clear();
	m_DirtTiles.clear();
	m_MoneyBags.clear();
	m_Player = nullptr;

	RemoveTombstone();

	dae::ServiceLocator::GetSoundSystem().Stop(GameSound::MoneyBagWiggle);
	dae::ServiceLocator::GetSoundSystem().Stop(GameSound::MoneyBagFalling);
}

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

void digger::GameManagerComponent::ClearHUD()
{
	for (auto* object : m_HUDObjects)
	{
		if (object)
			m_Scene->Remove(*object);
	}

	m_HUDObjects.clear();

	for (auto& icon : m_LifeIcons)
		icon = nullptr;

	m_ScoreText = nullptr;
}

void digger::GameManagerComponent::SpawnLevel(const LevelData& data)
{
	auto player = std::make_unique<dae::GameObject>();
	auto* playerPtr = player.get();

	player->AddComponent<dae::TransformComponent>(playerPtr);

	auto* movement = player->AddComponent<GridMovementComponent>(
		playerPtr,
		this,
		m_TileSize,
		120.f,
		m_MapOffset);

	movement->SetGridPosition(data.playerSpawn);

	
	player->AddComponent<dae::RenderComponent>(
		playerPtr,
		dae::ResourceManager::GetInstance().LoadTexture("Digger.png"));

	auto* actor = player->AddComponent<dae::GameActorComponent>(playerPtr, m_Lives, m_Score, 1);
	m_PlayerActor = actor;
	dae::EventBus::GetInstance().GetSubject().AddObserver(m_PlayerActor);

	m_Player = playerPtr;
	m_PlayerSpawn = data.playerSpawn;
	m_LevelObjects.push_back(playerPtr);
	m_Scene->Add(std::move(player));

	m_DirtTiles.clear();

	auto dirtTexture =
		dae::ResourceManager::GetInstance().LoadTexture("Dirt.png");

	for (int y = 0; y < data.height; ++y)
	{
		for (int x = 0; x < data.width; ++x)
		{
			if (data.tiles[y][x] != '#')
				continue;

			const glm::ivec2 gridPos{ x, y };
			const std::string key = MakeTileKey(gridPos);

			DirtTile dirtTile{};

			const int subdivisions = 16;
			const float pieceSize =
				static_cast<float>(m_TileSize) / static_cast<float>(subdivisions);

			for (int py = 0; py < subdivisions; ++py)
			{
				for (int px = 0; px < subdivisions; ++px)
				{
					const int pieceIndex = py * subdivisions + px;

					auto dirt = std::make_unique<dae::GameObject>();
					auto* dirtPtr = dirt.get();

					auto* transform =
						dirt->AddComponent<dae::TransformComponent>(dirtPtr);

					transform->SetLocalPosition(
						m_MapOffset.x + static_cast<float>(x * m_TileSize) + px * pieceSize,
						m_MapOffset.y + static_cast<float>(y * m_TileSize) + py * pieceSize);

					dirt->AddComponent<dae::RenderComponent>(
						dirtPtr,
						dirtTexture,
						pieceSize,
						pieceSize);

					dirtTile.pieces[pieceIndex] = dirtPtr;

					m_LevelObjects.push_back(dirtPtr);
					m_Scene->Add(std::move(dirt));
				}
			}

			m_DirtTiles[key] = dirtTile;
		}
	}


	for (const auto& diamondPos : data.diamonds)
	{
		auto diamond = std::make_unique<dae::GameObject>();
		auto* diamondPtr = diamond.get();

		diamond->AddComponent<dae::TransformComponent>(diamondPtr);
		diamond->AddComponent<GridPositionComponent>(diamondPtr, m_TileSize, m_MapOffset)
			->SetGridPosition(diamondPos);


		diamond->AddComponent<dae::RenderComponent>(
			diamondPtr,
			dae::ResourceManager::GetInstance().LoadTexture("Diamond.png"));

		diamond->AddComponent<DiamondComponent>(diamondPtr, this);

		m_Diamonds.push_back(diamondPtr);
		m_LevelObjects.push_back(diamondPtr);
		m_Scene->Add(std::move(diamond));
	}

	for (const auto& bagPos : data.moneyBags)
	{
		auto bag = std::make_unique<dae::GameObject>();
		auto* bagPtr = bag.get();

		bag->AddComponent<dae::TransformComponent>(bagPtr);
		

		bag->AddComponent<dae::RenderComponent>(
			bagPtr,
			dae::ResourceManager::GetInstance().LoadTexture("Money.PNG"));

		auto* bagComp = bag->AddComponent<MoneyBagComponent>(
			bagPtr,
			this,
			m_TileSize,
			m_MapOffset);

		bagComp->SetGridPosition(bagPos);

		RegisterMoneyBag(bagComp);

		m_LevelObjects.push_back(bagPtr);
		m_Scene->Add(std::move(bag));
	}

	/*for (const auto& enemyPos : data.enemies)
	{
		auto enemy = std::make_unique<dae::GameObject>();
		auto* enemyPtr = enemy.get();

		enemy->AddComponent<dae::TransformComponent>(enemyPtr);

		enemy->AddComponent<dae::RenderComponent>(
			enemyPtr,
			dae::ResourceManager::GetInstance().LoadTexture("Nobbin.png"));

		auto* enemyComp = enemy->AddComponent<EnemyComponent>(enemyPtr, this);
		enemyComp->SetGridPosition(enemyPos);

		RegisterEnemy(enemyComp);
		m_LevelObjects.push_back(enemyPtr);
		m_Scene->Add(std::move(enemy));
	}
	*/

	m_EnemySpawn = data.enemySpawn;
}

std::string digger::GameManagerComponent::MakeTileKey(const glm::ivec2& pos) const
{
	return std::to_string(pos.x) + "," + std::to_string(pos.y);
}

bool digger::GameManagerComponent::IsDirt(const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return false;

	return m_LevelData.tiles[pos.y][pos.x] == '#';
}

bool digger::GameManagerComponent::CanEnemyMoveTo(const glm::ivec2& pos, bool canDig) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return false;

	const char tile = m_LevelData.tiles[pos.y][pos.x];

	if (tile == '#')
		return canDig;

	return true;
}

void digger::GameManagerComponent::DigTile(const glm::ivec2& pos)
{
	if (!IsDirt(pos))
		return;

	auto key = MakeTileKey(pos);
	auto it = m_DirtTiles.find(key);

	if (it != m_DirtTiles.end())
	{
		for (int i = 0; i < 4; ++i)
		{
			if (auto* obj = it->second.pieces[i])
			{
				if (auto* tr = obj->GetComponent<dae::TransformComponent>())
					tr->SetLocalPosition(-5000.f, -5000.f);
			}
		}

		m_DirtTiles.erase(it);
	}

	m_LevelData.tiles[pos.y][pos.x] = '.';
}



glm::ivec2 digger::GameManagerComponent::GetPlayerGridPosition() const
{
	
		if (!m_Player)
			return {};

		if (auto* movement = m_Player->GetComponent<GridMovementComponent>())
			return movement->GetGridPosition();

		if (auto* grid = m_Player->GetComponent<GridPositionComponent>())
			return grid->GetGridPosition();

		return {};
	
}

void digger::GameManagerComponent::DamagePlayer()
{
	if (m_ScreenState != GameScreenState::Playing)
		return;

	if (!m_Player)
		return;

	if (m_DamageCooldown > 0.f)
		return;

	if (m_DeathSequenceActive)
		return;

	m_DamageCooldown = m_DamageCooldownDuration;

	--m_Lives;
	UpdateHUD();

	auto* tr = m_Player->GetComponent<dae::TransformComponent>();
	if (tr)
	{
		const auto& pos = tr->GetWorldPosition();
		m_PlayerDeathWorldPosition = { pos.x, pos.y };
	}
	else
	{
		m_PlayerDeathWorldPosition = {};
	}

	BeginPlayerDeathSequence();
}

void digger::GameManagerComponent::BeginPlayerDeathSequence()
{
	m_DeathSequenceActive = true;
	m_DeathSequenceTimer = 0.f;

	RemoveAllFireballs();

	auto& sound = dae::ServiceLocator::GetSoundSystem();

	sound.Stop(GameSound::BackgroundMusic);
	sound.Stop(GameSound::MoneyBagWiggle);
	sound.Stop(GameSound::MoneyBagFalling);

	sound.Play(GameSound::PlayerDeathMusic, 1.0f);

	SpawnTombstone(m_PlayerDeathWorldPosition);

	// Hide player while tombstone is visible
	if (m_Player)
	{
		if (auto* tr = m_Player->GetComponent<dae::TransformComponent>())
			tr->SetLocalPosition(-5000.f, -5000.f);
	}
}

void digger::GameManagerComponent::FinishPlayerDeathSequence()
{
	m_DeathSequenceActive = false;
	m_DeathSequenceTimer = 0.f;

	RemoveTombstone();

	if (m_Lives <= 0)
	{
		GameOver();
		return;
	}

	if (m_Player)
	{
		if (auto* movement = m_Player->GetComponent<GridMovementComponent>())
			movement->SetGridPosition(m_PlayerSpawn);
	}

	ResetEnemiesAfterPlayerDeath();

	dae::ServiceLocator::GetSoundSystem().PlayLooping(
		GameSound::BackgroundMusic,
		0.5f);
}

void digger::GameManagerComponent::SpawnTombstone(const glm::vec2& position)
{
	RemoveTombstone();

	auto tomb = std::make_unique<dae::GameObject>();
	auto* tombPtr = tomb.get();

	auto* tr = tomb->AddComponent<dae::TransformComponent>(tombPtr);
	tr->SetLocalPosition(position.x, position.y);
	tr->SetLocalScale(0.9f, 0.9f);

	tomb->AddComponent<dae::RenderComponent>(
		tombPtr,
		dae::ResourceManager::GetInstance().LoadTexture("Tomb.png"));

	m_Tombstone = tombPtr;
	m_Scene->Add(std::move(tomb));
}

void digger::GameManagerComponent::RemoveTombstone()
{
	if (!m_Tombstone)
		return;

	m_Scene->Remove(*m_Tombstone);
	m_Tombstone = nullptr;
}

void digger::GameManagerComponent::RegisterEnemy(EnemyComponent* enemy)
{
	if (enemy)
		m_Enemies.push_back(enemy);
}

void digger::GameManagerComponent::CheckEnemyCrossings()
{
	for (size_t i = 0; i < m_Enemies.size(); ++i)
	{
		for (size_t j = i + 1; j < m_Enemies.size(); ++j)
		{
			if (!m_Enemies[i] || !m_Enemies[j])
				continue;

			if (m_Enemies[i]->GetGridPosition() == m_Enemies[j]->GetGridPosition())
			{
				m_Enemies[i]->RegisterEnemyCollision();
				m_Enemies[j]->RegisterEnemyCollision();
			}
		}
	}
}

bool digger::GameManagerComponent::CanPlayerMoveTo(const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return false;

	const char tile = m_LevelData.tiles[pos.y][pos.x];

	if (tile == 'X')
		return false;

	return true;
}

void digger::GameManagerComponent::DigAtWorldPosition(
	const glm::vec2& worldPos,
	const glm::ivec2&)
{
	const float r = m_DigRadius;

	DigAtPoint(worldPos);

	DigAtPoint(worldPos + glm::vec2{ r, 0.f });
	DigAtPoint(worldPos + glm::vec2{ -r, 0.f });
	DigAtPoint(worldPos + glm::vec2{ 0.f, r });
	DigAtPoint(worldPos + glm::vec2{ 0.f, -r });

	const float diagonal = r * 0.7071f;

	DigAtPoint(worldPos + glm::vec2{ diagonal, diagonal });
	DigAtPoint(worldPos + glm::vec2{ -diagonal, diagonal });
	DigAtPoint(worldPos + glm::vec2{ diagonal, -diagonal });
	DigAtPoint(worldPos + glm::vec2{ -diagonal, -diagonal });
}

glm::vec2 digger::GameManagerComponent::GetPlayerWorldPosition() const
{
	if (!m_Player)
		return {};

	if (auto* movement = m_Player->GetComponent<GridMovementComponent>())
		return movement->GetWorldPosition() + m_PlayerCenterOffset;

	if (auto* tr = m_Player->GetComponent<dae::TransformComponent>())
	{
		const auto& pos = tr->GetWorldPosition();
		return { pos.x + m_PlayerCenterOffset.x, pos.y + m_PlayerCenterOffset.y };
	}

	return {};
}


void digger::GameManagerComponent::DigAtPoint(const glm::vec2& point)
{
	const int gridX = static_cast<int>(
		std::floor((point.x - m_MapOffset.x) / static_cast<float>(m_TileSize)));

	const int gridY = static_cast<int>(
		std::floor((point.y - m_MapOffset.y) / static_cast<float>(m_TileSize)));

	const glm::ivec2 tilePos{ gridX, gridY };


	if (HasMoneyBagAt(tilePos))
		return;

	if (!IsDirt(tilePos))
		return;


	auto it = m_DirtTiles.find(MakeTileKey(tilePos));
	if (it == m_DirtTiles.end())
		return;

	auto& tile = it->second;

	constexpr int subdivisions = 16;
	const float pieceSize =
		static_cast<float>(m_TileSize) / static_cast<float>(subdivisions);

	const float tileWorldX = m_MapOffset.x + static_cast<float>(gridX * m_TileSize);
	const float tileWorldY = m_MapOffset.y + static_cast<float>(gridY * m_TileSize);

	const float digRadius = 16.0f;
	const float digRadiusSq = digRadius * digRadius;

	auto removePiece = [&](int index)
		{
			if (tile.removed[index])
				return;

			tile.removed[index] = true;

			if (auto* obj = tile.pieces[index])
			{
				if (auto* tr = obj->GetComponent<dae::TransformComponent>())
					tr->SetLocalPosition(-5000.f, -5000.f);
			}
		};

	for (int py = 0; py < subdivisions; ++py)
	{
		for (int px = 0; px < subdivisions; ++px)
		{
			const int index = py * subdivisions + px;

			const glm::vec2 pieceCenter{
				tileWorldX + px * pieceSize + pieceSize * 0.5f,
				tileWorldY + py * pieceSize + pieceSize * 0.5f
			};

			const glm::vec2 diff = pieceCenter - point;
			const float distSq = diff.x * diff.x + diff.y * diff.y;

			if (distSq <= digRadiusSq)
				removePiece(index);
		}
	}

	int removedCount = 0;

	for (bool removed : tile.removed)
	{
		if (removed)
			++removedCount;
	}

	const int requiredRemoved =
		static_cast<int>(DirtPieceCount * 0.95f);

	if (removedCount >= requiredRemoved)
	{
		for (int i = 0; i < DirtPieceCount; ++i)
		{
			if (!tile.removed[i])
			{
				tile.removed[i] = true;

				if (auto* obj = tile.pieces[i])
				{
					if (auto* tr = obj->GetComponent<dae::TransformComponent>())
						tr->SetLocalPosition(-5000.f, -5000.f);
				}
			}
		}

		m_LevelData.tiles[tilePos.y][tilePos.x] = '.';
	}
}


void digger::GameManagerComponent::SpawnEnemy()
{
	if (m_EnemiesRemainingToSpawn <= 0)
		return;

	auto enemy = std::make_unique<dae::GameObject>();
	auto* enemyPtr = enemy.get();

	enemy->AddComponent<dae::TransformComponent>(enemyPtr);

	enemy->AddComponent<dae::RenderComponent>(
		enemyPtr,
		dae::ResourceManager::GetInstance().LoadTexture("Nobbin.png"));

	auto* enemyComp = enemy->AddComponent<EnemyComponent>(enemyPtr, this);
	enemyComp->SetGridPosition(m_EnemySpawn);

	RegisterEnemy(enemyComp);

	m_LevelObjects.push_back(enemyPtr);
	m_EnemiesAlive++;
	m_EnemiesRemainingToSpawn--;

	m_Scene->Add(std::move(enemy));
}


void digger::GameManagerComponent::KillEnemy(EnemyComponent* enemy)
{
	if (!enemy)
		return;

	auto* enemyObject = enemy->GetGameObject();

	m_Enemies.erase(
		std::remove(m_Enemies.begin(), m_Enemies.end(), enemy),
		m_Enemies.end());

	m_LevelObjects.erase(
		std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), enemyObject),
		m_LevelObjects.end());

	m_Scene->Remove(*enemyObject);

	--m_EnemiesAlive;
	if (m_EnemiesAlive < 0)
		m_EnemiesAlive = 0;

	m_Score += 250;
	UpdateHUD();
}


void digger::GameManagerComponent::ResetEnemiesAfterPlayerDeath()
{
	for (auto* enemy : m_Enemies)
	{
		if (!enemy)
			continue;

		auto* enemyObject = enemy->GetGameObject();

		m_LevelObjects.erase(
			std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), enemyObject),
			m_LevelObjects.end());

		m_Scene->Remove(*enemyObject);
	}

	m_Enemies.clear();

	m_EnemiesRemainingToSpawn += m_EnemiesAlive;
	m_EnemiesAlive = 0;
	m_EnemySpawnTimer = 0.f;
}

float digger::GameManagerComponent::GetFireballCooldown() const
{
	return 0.5f + static_cast<float>(m_CurrentLevel) * 0.25f;
}

glm::ivec2 digger::GameManagerComponent::GetPlayerFacingDirection() const
{
	if (!m_Player)
		return { 1, 0 };

	auto* movement = m_Player->GetComponent<GridMovementComponent>();
	if (!movement)
		return { 1, 0 };

	return movement->GetFacingDirection();
}

void digger::GameManagerComponent::ShootFireball()
{
	if (m_FireballCooldownTimer > 0.f)
		return;

	if (!m_Player)
		return;

	m_FireballCooldownTimer = GetFireballCooldown();

	auto* tr = m_Player->GetComponent<dae::TransformComponent>();
	if (!tr)
		return;

	const auto& pos3 = tr->GetWorldPosition();

	glm::ivec2 facing = GetPlayerFacingDirection();
	glm::vec2 direction{
		static_cast<float>(facing.x),
		static_cast<float>(facing.y)
	};

	auto fireball = std::make_unique<dae::GameObject>();
	auto* fireballPtr = fireball.get();

	dae::ServiceLocator::GetSoundSystem().PlayLooping(
		GameSound::BulletTravel,
		0.7f);

	auto* fireTr = fireball->AddComponent<dae::TransformComponent>(fireballPtr);

	
	glm::vec2 fireballPos{
		pos3.x ,
		pos3.y 
	};

	fireballPos += glm::vec2{
		static_cast<float>(facing.x),
		static_cast<float>(facing.y)
	} * 28.f;

	fireTr->SetLocalPosition(fireballPos.x, fireballPos.y);
	
	fireball->AddComponent<dae::RenderComponent>(
		fireballPtr,
		dae::ResourceManager::GetInstance().LoadTexture("Fireball.png"));

	fireball->AddComponent<FireballComponent>(
		fireballPtr,
		this,
		direction,
		300.f);

	m_Fireballs.push_back(fireballPtr);
	m_LevelObjects.push_back(fireballPtr);

	m_Scene->Add(std::move(fireball));
}

bool digger::GameManagerComponent::IsBlockingTileForFireball(const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return true;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return true;

	const char tile = m_LevelData.tiles[pos.y][pos.x];

	// Dirt or solid wall blocks fireball
	return tile == '#' || tile == 'X';
}

void digger::GameManagerComponent::RemoveFireball(dae::GameObject* fireball)
{
	if (!fireball)
		return;

	dae::ServiceLocator::GetSoundSystem().Stop(GameSound::BulletTravel);
	dae::ServiceLocator::GetSoundSystem().Play(GameSound::BulletHit, 1.0f);

	m_Fireballs.erase(
		std::remove(m_Fireballs.begin(), m_Fireballs.end(), fireball),
		m_Fireballs.end());

	m_LevelObjects.erase(
		std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), fireball),
		m_LevelObjects.end());

	m_Scene->Remove(*fireball);
}


void digger::GameManagerComponent::CheckFireballHit(dae::GameObject* fireball)
{
	if (!fireball)
		return;

	auto* fireTr = fireball->GetComponent<dae::TransformComponent>();
	if (!fireTr)
		return;

	const auto& firePos3 = fireTr->GetWorldPosition();

	glm::vec2 fireCenter{
		firePos3.x + 12.f,
		firePos3.y + 12.f
	};

	const glm::ivec2 fireGrid{
		static_cast<int>(std::floor((fireCenter.x - m_MapOffset.x) / static_cast<float>(m_TileSize))),
		static_cast<int>(std::floor((fireCenter.y - m_MapOffset.y) / static_cast<float>(m_TileSize)))
	};

	// Hit dirt/wall/out-of-bounds
	if (IsBlockingTileForFireball(fireGrid))
	{
		RemoveFireball(fireball);
		return;
	}

	// Hit enemy
	for (auto* enemy : m_Enemies)
	{
		if (!enemy || enemy->IsDead())
			continue;

		auto* enemyTr =
			enemy->GetGameObject()->GetComponent<dae::TransformComponent>();

		if (!enemyTr)
			continue;

		const auto& enemyPos3 = enemyTr->GetWorldPosition();

		glm::vec2 enemyCenter{
			enemyPos3.x + 32.f,
			enemyPos3.y + 32.f
		};

		const float dx = enemyCenter.x - fireCenter.x;
		const float dy = enemyCenter.y - fireCenter.y;

		const float radius = 32.f;

		if ((dx * dx + dy * dy) <= radius * radius)
		{
			enemy->Kill();
			RemoveFireball(fireball);
			return;
		}
	}
}


bool digger::GameManagerComponent::HasDirtBelow(const glm::ivec2& pos) const
{
	const glm::ivec2 below{ pos.x, pos.y + 1 };

	if (below.y >= m_LevelData.height)
		return true;

	return IsDirt(below);
}

bool digger::GameManagerComponent::CanMoneyBagMoveTo(const glm::ivec2& pos) const
{
	if (pos.x < 0 || pos.y < 0)
		return false;

	if (pos.y >= m_LevelData.height || pos.x >= m_LevelData.width)
		return false;

	if (IsDirt(pos))
		return false;

	if (IsPlayerAtGridPosition(pos))
		return false;

	if (HasEnemyAt(pos))
		return false;

	if (HasMoneyBagAt(pos))
		return false;

	return true;
}

void digger::GameManagerComponent::RegisterMoneyBag(MoneyBagComponent* bag)
{
	if (bag)
		m_MoneyBags.push_back(bag);
}

void digger::GameManagerComponent::UnregisterMoneyBag(MoneyBagComponent* bag)
{
	m_MoneyBags.erase(
		std::remove(m_MoneyBags.begin(), m_MoneyBags.end(), bag),
		m_MoneyBags.end());
}

bool digger::GameManagerComponent::TryPushMoneyBagAt(
	const glm::ivec2& bagPos,
	const glm::ivec2& direction)
{
	const glm::ivec2 targetPos = bagPos + direction;

	if (IsPlayerAtGridPosition(targetPos))
		return false;

	if (HasEnemyAt(targetPos))
		return false;

	if (HasMoneyBagAt(targetPos))
		return false;

	if (!CanMoneyBagMoveTo(targetPos))
		return false;

	for (auto* bag : m_MoneyBags)
	{
		if (!bag || bag->IsBrokenGold())
			continue;

		if (bag->GetGridPosition() == bagPos)
			return bag->TryPush(direction);
	}

	return false;
}

void digger::GameManagerComponent::CollectGold(dae::GameObject* gold)
{
	if (!gold)
		return;

	m_Score += 250;
	UpdateHUD();

	m_LevelObjects.erase(
		std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), gold),
		m_LevelObjects.end());

	m_Scene->Remove(*gold);
}

bool digger::GameManagerComponent::HasMoneyBagAt(const glm::ivec2& pos) const
{
	for (auto* bag : m_MoneyBags)
	{
		if (!bag || bag->IsBrokenGold())
			continue;

		if (bag->GetGridPosition() == pos)
			return true;
	}

	return false;
}

bool digger::GameManagerComponent::IsPlayerAtGridPosition(const glm::ivec2& pos) const
{
	return GetPlayerGridPosition() == pos;
}

bool digger::GameManagerComponent::HasEnemyAt(const glm::ivec2& pos) const
{
	for (auto* enemy : m_Enemies)
	{
		if (!enemy || enemy->IsDead())
			continue;

		if (enemy->GetGridPosition() == pos)
			return true;
	}

	return false;
}

digger::EnemyComponent* digger::GameManagerComponent::GetEnemyAtWorldPosition(
	const glm::vec2& worldPos,
	float radius) const
{
	const float radiusSq = radius * radius;

	for (auto* enemy : m_Enemies)
	{
		if (!enemy || enemy->IsDead())
			continue;

		auto* tr = enemy->GetGameObject()->GetComponent<dae::TransformComponent>();
		if (!tr)
			continue;

		const auto& pos3 = tr->GetWorldPosition();
		glm::vec2 enemyCenter{ pos3.x + 32.f, pos3.y + 32.f };

		const glm::vec2 diff = enemyCenter - worldPos;

		if ((diff.x * diff.x + diff.y * diff.y) <= radiusSq)
			return enemy;
	}

	return nullptr;
}

void digger::GameManagerComponent::RemoveAllFireballs()
{
	auto& sound = dae::ServiceLocator::GetSoundSystem();
	sound.Stop(GameSound::BulletTravel);

	for (auto* fireball : m_Fireballs)
	{
		if (fireball)
		{
			m_LevelObjects.erase(
				std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), fireball),
				m_LevelObjects.end());

			m_Scene->Remove(*fireball);
		}
	}

	m_Fireballs.clear();
	
}

