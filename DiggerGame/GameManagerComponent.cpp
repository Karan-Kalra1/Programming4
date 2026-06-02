#include "GameManagerComponent.h"

#include "LevelLoader.h"
#include "Scene.h"
#include "GameObject.h"
#include "ServiceLocator.h"
#include "GameSounds.h"
#include "MiniginTime.h"

#include <algorithm>
#include <filesystem>
#include <iostream>



digger::GameManagerComponent::GameManagerComponent(dae::GameObject* owner, dae::Scene* scene)
	: Component(owner)
	, m_Scene(scene)
{
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

	for (auto& player : m_Players)
	{
		if (player.damageCooldown > 0.f)
			player.damageCooldown -= dae::MiniginTime::GetDeltaTime();

		if (player.fireballCooldown > 0.f)
			player.fireballCooldown -= dae::MiniginTime::GetDeltaTime();
	}

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

	if (m_Mode == GameMode::Versus)
		CheckVersusCollision();

	CheckEnemyCrossings();
}

void digger::GameManagerComponent::ToggleMute()
{
	auto& sound = dae::ServiceLocator::GetSoundSystem();
	sound.SetMuted(!sound.IsMuted());
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
	m_Enemies.clear();
	m_DirtTiles.clear();
	m_MoneyBags.clear();

	m_Players.clear();

	//m_Player = nullptr;

	RemoveTombstone();

	dae::ServiceLocator::GetSoundSystem().Stop(GameSound::MoneyBagWiggle);
	dae::ServiceLocator::GetSoundSystem().Stop(GameSound::MoneyBagFalling);
}




//Diamond Collection

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







