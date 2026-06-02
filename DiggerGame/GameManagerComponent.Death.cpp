#include "GameManagerComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "GridMovementComponent.h"
#include "ServiceLocator.h"
#include "GameSounds.h"

#include <memory>

//Playing Death Sequence

void digger::GameManagerComponent::BeginPlayerDeathSequence()
{
	if (m_DeathSequenceActive)
		return;

	m_DeathSequenceActive = true;
	m_DeathSequenceTimer = 0.f;

	for (auto& player : m_Players)
	{
		if (!player.object)
			continue;

		if (auto* movement = player.object->GetComponent<GridMovementComponent>())
			movement->ReleaseAllDirections();
	}


	RemoveAllFireballs();

	auto& sound = dae::ServiceLocator::GetSoundSystem();

	sound.Stop(GameSound::BackgroundMusic);
	sound.Stop(GameSound::MoneyBagWiggle);
	sound.Stop(GameSound::MoneyBagFalling);
	sound.Stop(GameSound::BulletTravel);

	sound.Play(GameSound::PlayerDeathSfx, 1.0f);
	sound.Play(GameSound::PlayerDeathMusic, 1.0f);

	SpawnTombstone(m_PlayerDeathWorldPosition);

	if (m_DeathPlayerIndex >= 0 &&
		m_DeathPlayerIndex < static_cast<int>(m_Players.size()))
	{
		auto* player = m_Players[static_cast<size_t>(m_DeathPlayerIndex)].object;

		if (player)
		{
			if (auto* tr = player->GetComponent<dae::TransformComponent>())
				tr->SetLocalPosition(-5000.f, -5000.f);
		}
	}
}

void digger::GameManagerComponent::FinishPlayerDeathSequence()
{

	RemoveTombstone();

	if (m_DeathPlayerIndex < 0 ||
		m_DeathPlayerIndex >= static_cast<int>(m_Players.size()))
	{
		m_DeathPlayerIndex = -1;
		m_DeathSequenceActive = false;
		m_DeathSequenceTimer = 0.f;
		return;
	}

	auto& player = m_Players[static_cast<size_t>(m_DeathPlayerIndex)];

	// Reset enemies
	ResetEnemiesAfterPlayerDeath();

	if (player.lives <= 0)
	{
		player.alive = false;
		m_PlayerAlive[static_cast<size_t>(m_DeathPlayerIndex)] = false;
		player.dying = false;

		if (player.object)
		{
			if (auto* tr = player.object->GetComponent<dae::TransformComponent>())
				tr->SetLocalPosition(-5000.f, -5000.f);

			if (auto* movement = player.object->GetComponent<GridMovementComponent>())
				movement->ReleaseAllDirections();
		}

		if (m_Mode == GameMode::Versus)
		{
			GameOver();
			return;
		}


		if (AreAllPlayersDead())
		{
			m_DeathPlayerIndex = -1;
			m_DeathSequenceActive = false;
			m_DeathSequenceTimer = 0.f;

			GameOver();
			return;
		}
	}
	else
	{
		if (player.object)
		{
			if (auto* movement = player.object->GetComponent<GridMovementComponent>())
				movement->SetGridPosition(player.spawn);
		}

		// Small protection window after respawn
		player.damageCooldown = 1.0f;

		//player can be hit again
		player.dying = false;
	}

	m_DeathPlayerIndex = -1;

	m_DeathSequenceActive = false;
	m_DeathSequenceTimer = 0.f;

	dae::ServiceLocator::GetSoundSystem().PlayLooping(
		GameSound::BackgroundMusic,
		0.5f);
}



//Spawning/Despawning Tombstone

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



