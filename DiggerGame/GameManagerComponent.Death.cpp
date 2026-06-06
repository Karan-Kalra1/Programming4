#include "GameManagerComponent.h"

#include "DeathSequenceController.h"
#include "GridMovementComponent.h"
#include "ServiceLocator.h"
#include "GameSounds.h"
#include "TransformComponent.h"
#include "GameObject.h"

#include <memory>

//Playing Death Sequence
void digger::GameManagerComponent::BeginPlayerDeathSequence()
{
	if (m_DeathSequenceController &&
		m_DeathSequenceController->IsActive())
	{
		return;
	}

	for (auto& player : m_Players)
	{
		if (!player.object)
			continue;

		if (auto* movement = player.object->GetComponent<GridMovementComponent>())
			movement->ReleaseAllDirections();
	}

	RemoveAllFireballs();

	if (m_DeathPlayerIndex >= 0 &&
		m_DeathPlayerIndex < static_cast<int>(m_Players.size()))
	{
		auto* playerObject =
			m_Players[static_cast<size_t>(m_DeathPlayerIndex)].object;

		if (playerObject)
		{
			if (auto* tr = playerObject->GetComponent<dae::TransformComponent>())
				tr->SetLocalPosition(-5000.f, -5000.f);
		}
	}

	if (m_DeathSequenceController)
		m_DeathSequenceController->Begin(m_LastDeathWorldPosition);
}

void digger::GameManagerComponent::FinishPlayerDeathSequence()
{
	if (m_DeathPlayerIndex < 0 ||
		m_DeathPlayerIndex >= static_cast<int>(m_Players.size()))
	{
		m_DeathPlayerIndex = -1;
		return;
	}

	auto& player = m_Players[static_cast<size_t>(m_DeathPlayerIndex)];

	ResetEnemiesAfterPlayerDeath();

	if (player.lives <= 0)
	{
		player.alive = false;
		player.dying = false;

		m_PlayerAlive[static_cast<size_t>(m_DeathPlayerIndex)] = false;

		if (player.object)
		{
			if (auto* tr = player.object->GetComponent<dae::TransformComponent>())
				tr->SetLocalPosition(-5000.f, -5000.f);

			if (auto* movement = player.object->GetComponent<GridMovementComponent>())
				movement->ReleaseAllDirections();
		}

		if (m_Mode == GameMode::Versus || AreAllPlayersDead())
		{
			m_DeathPlayerIndex = -1;
			GameOver();
			return;
		}
	}
	else
	{
		player.dying = false;

		if (player.object)
		{
			if (auto* movement = player.object->GetComponent<GridMovementComponent>())
				movement->SetGridPosition(player.spawn);
		}

		player.damageCooldown = 1.0f;
	}

	m_DeathPlayerIndex = -1;

	dae::ServiceLocator::GetSoundSystem().PlayLooping(
		GameSound::BackgroundMusic,
		0.5f);
}

