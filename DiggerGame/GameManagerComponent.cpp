#include "GameManagerComponent.h"

#include "LevelLoader.h"
#include "Scene.h"
#include "GameObject.h"
#include "ServiceLocator.h"
#include "GameSounds.h"
#include "MiniginTime.h"
#include "HudController.h"
#include "MenuController.h"
#include "HighScoreScreenController.h"
#include "DeathSequenceController.h"
#include "MoneyBagManager.h"
#include "FireballManager.h"
#include "EnemyManager.h"
//#include "PlayerManager.h"
//#include "LevelSystem.h"

#include <algorithm>
#include <filesystem>
#include <iostream>



digger::GameManagerComponent::~GameManagerComponent() = default;


digger::GameManagerComponent::GameManagerComponent(dae::GameObject* owner, dae::Scene* scene)
	: Component(owner)
	, m_Scene(scene)
{
	m_HudController = std::make_unique<HudController>(scene);
	m_MenuController = std::make_unique<MenuController>(scene);
	m_HighScoreScreenController =
		std::make_unique<HighScoreScreenController>(
			scene,
			"Data/Highscores.txt");
	m_DeathSequenceController =
		std::make_unique<DeathSequenceController>(
			scene,
			6.9f);
	m_MoneyBagManager = std::make_unique<MoneyBagManager>(scene);
	m_FireballManager = std::make_unique<FireballManager>(scene);
	m_EnemyManager = std::make_unique<EnemyManager>(scene);
	//m_PlayerManager = std::make_unique<PlayerManager>();
	//m_LevelSystem = std::make_unique<LevelSystem>();

	//RegisterSounds();
}


//Main Game Loop

void digger::GameManagerComponent::StartGame(GameMode mode)
{
	ClearScreenUI();
	ClearHUD();
	ClearLevel();

	m_Mode = mode;
	m_Score = 0;
	//m_Lives = 4;
	m_CurrentLevel = 0;
	m_PlayerLives = { 4, 4 };
	m_PlayerAlive = { true, true };

	m_ScreenState = GameScreenState::Playing;

	if (m_HighScoreScreenController)
		m_HighScoreScreenController->LoadScores();

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

	if (m_EnemyManager)
		m_EnemyManager->BeginStage(*this);
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

	for (auto& player : m_Players)
	{
		if (player.damageCooldown > 0.f)
			player.damageCooldown -= dae::MiniginTime::GetDeltaTime();

		if (player.fireballCooldown > 0.f)
			player.fireballCooldown -= dae::MiniginTime::GetDeltaTime();
	}

	if (m_DeathSequenceController &&
		m_DeathSequenceController->IsActive())
	{
		const bool finished =
			m_DeathSequenceController->Update(
				dae::MiniginTime::GetDeltaTime());

		if (finished)
			FinishPlayerDeathSequence();

		return;
	}

	if (m_ShouldLoadNextLevel)
	{
		m_ShouldLoadNextLevel = false;
		LoadLevel(m_CurrentLevel + 1);
	}

	if (m_EnemyManager)
	{
		const bool stageComplete =
			m_EnemyManager->Update(
				*this,
				dae::MiniginTime::GetDeltaTime());

		if (stageComplete)
		{
			m_ShouldLoadNextLevel = true;
			return;
		}

		m_EnemyManager->CheckEnemyCrossings();
	}


	if (m_Mode == GameMode::Versus)
		CheckVersusCollision();
}

void digger::GameManagerComponent::ToggleMute()
{
	auto& sound = dae::ServiceLocator::GetSoundSystem();
	sound.SetMuted(!sound.IsMuted());
}

bool digger::GameManagerComponent::IsGameplayFrozen() const
{
	return m_ScreenState != GameScreenState::Playing ||
		(m_DeathSequenceController && m_DeathSequenceController->IsActive());
}

void digger::GameManagerComponent::ClearLevel()
{
	RemovePlayerObservers();

	RemoveAllFireballs();

	for (auto* object : m_LevelObjects)
	{
		if (object)
			m_Scene->Remove(*object);
	}

	m_LevelObjects.clear();
	m_Diamonds.clear();
	m_DirtTiles.clear();

	if (m_EnemyManager)
		m_EnemyManager->Clear();

	if (m_MoneyBagManager)
		m_MoneyBagManager->Clear();

	m_Players.clear();

	//m_Player = nullptr;

	m_DeathSequenceController->Clear();

	dae::ServiceLocator::GetSoundSystem().Stop(GameSound::MoneyBagWiggle);
	dae::ServiceLocator::GetSoundSystem().Stop(GameSound::MoneyBagFalling);
}


//Removing and Adding LevelData
void digger::GameManagerComponent::AddLevelObject(dae::GameObject* object)
{
	if (!object)
		return;

	m_LevelObjects.push_back(object);
}

void digger::GameManagerComponent::RemoveLevelObject(dae::GameObject* object)
{
	m_LevelObjects.erase(
		std::remove(m_LevelObjects.begin(), m_LevelObjects.end(), object),
		m_LevelObjects.end());
}

//Adding Score
void digger::GameManagerComponent::AddScore(int amount)
{
	m_Score += amount;
	UpdateHUD();
}


//Diamond Collection

void digger::GameManagerComponent::CollectDiamond(dae::GameObject* diamond)
{
	if (!diamond)
		return;

	AddScore(100);

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







