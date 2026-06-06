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
#include "PlayerManager.h"
#include "LevelSystem.h"

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
	m_PlayerManager = std::make_unique<PlayerManager>(scene);
	m_LevelSystem = std::make_unique<LevelSystem>(scene);

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
	m_CurrentLevel = 0;

	if (m_PlayerManager)
		m_PlayerManager->ResetForNewGame(mode);

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
	m_IsLoadingLevel = true;
	m_LevelLoadFreezeTimer = m_LevelLoadFreezeDuration;

	ClearLevel();

	m_CurrentLevel = index;

	const auto levelPath =
		"Data/Levels/Level" + std::to_string(index + 1) + ".txt";

	const LevelData data = LevelLoader::Load(levelPath);

	if (m_LevelSystem)
	{
		m_LevelSystem->SetLevelData(data);
		m_LevelSystem->SpawnStaticObjects(*this);
	}

	if (m_PlayerManager)
		m_PlayerManager->SpawnPlayersForLevel(*this, data);

	if (m_EnemyManager)
	{
		m_EnemyManager->SetSpawn(data.enemySpawn, data.hasEnemySpawn);
		m_EnemyManager->BeginStage(*this);
	}

	m_ShouldLoadNextLevel = false;

}

void digger::GameManagerComponent::SkipLevel()
{
	if (m_CurrentLevel >= 2)
	{
		m_CurrentLevel = -1;
	}

	LoadLevel(m_CurrentLevel + 1);
}

void digger::GameManagerComponent::Update()
{

	if (m_IsLoadingLevel)
	{
		m_LevelLoadFreezeTimer -= dae::MiniginTime::GetDeltaTime();

		if (m_LevelLoadFreezeTimer <= 0.f)
		{
			m_IsLoadingLevel = false;
			m_LevelLoadFreezeTimer = 0.f;
		}

		return;
	}


	if (m_ScreenState == GameScreenState::StartMenu ||
		m_ScreenState == GameScreenState::ModeSelect ||
		m_ScreenState == GameScreenState::EnteringHighScore ||
		m_ScreenState == GameScreenState::ShowingHighScores)
	{
		return;
	}

	m_PlayerManager->UpdateCooldowns(
		dae::MiniginTime::GetDeltaTime());

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
	return m_IsLoadingLevel ||
		m_ScreenState != GameScreenState::Playing ||
		(m_DeathSequenceController &&
			m_DeathSequenceController->IsActive());
}

void digger::GameManagerComponent::ClearLevel()
{
	
	RemoveAllFireballs();

	if (m_DeathSequenceController)
		m_DeathSequenceController->Clear();

	for (auto* object : m_LevelObjects)
	{
		if (object)
			m_Scene->Remove(*object);
	}

	m_LevelObjects.clear();

	if (m_LevelSystem)
		m_LevelSystem->Clear();

	if (m_MoneyBagManager)
		m_MoneyBagManager->Clear();

	if (m_EnemyManager)
		m_EnemyManager->Clear();

	if (m_PlayerManager)
		m_PlayerManager->ClearLevel();

	dae::ServiceLocator::GetSoundSystem().Stop(GameSound::MoneyBagWiggle);
	dae::ServiceLocator::GetSoundSystem().Stop(GameSound::MoneyBagFalling);
}


//Adding Score
void digger::GameManagerComponent::AddScore(int amount)
{
	m_Score += amount;
	UpdateHUD();
}







