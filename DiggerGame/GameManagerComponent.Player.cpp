#include "GameManagerComponent.h"

#include "PlayerManager.h"

const std::vector<digger::PlayerRuntime>&
digger::GameManagerComponent::GetPlayers() const
{
	static const std::vector<PlayerRuntime> empty{};

	if (!m_PlayerManager)
		return empty;

	return m_PlayerManager->GetPlayers();
}

const digger::PlayerRuntime*
digger::GameManagerComponent::GetPlayerRuntime(int playerIndex) const
{
	if (!m_PlayerManager)
		return nullptr;

	return m_PlayerManager->GetPlayerRuntime(playerIndex);
}

dae::GameObject* digger::GameManagerComponent::GetPlayer(
	int playerIndex) const
{
	if (!m_PlayerManager)
		return nullptr;

	return m_PlayerManager->GetPlayer(playerIndex);
}

glm::ivec2 digger::GameManagerComponent::GetPlayerGridPosition(
	int playerIndex) const
{
	if (!m_PlayerManager)
		return {};

	return m_PlayerManager->GetPlayerGridPosition(playerIndex);
}

glm::vec2 digger::GameManagerComponent::GetPlayerWorldPosition(
	int playerIndex) const
{
	if (!m_PlayerManager)
		return {};

	return m_PlayerManager->GetPlayerWorldPosition(playerIndex);
}

glm::ivec2 digger::GameManagerComponent::GetClosestAlivePlayerGridPosition(
	const glm::ivec2& fromGrid) const
{
	if (!m_PlayerManager)
		return fromGrid;

	return m_PlayerManager->GetClosestAlivePlayerGridPosition(fromGrid);
}

int digger::GameManagerComponent::GetPlayerIndexAtWorldPosition(
	const glm::vec2& worldPos,
	float radius) const
{
	if (!m_PlayerManager)
		return -1;

	return m_PlayerManager->GetPlayerIndexAtWorldPosition(worldPos, radius);
}

int digger::GameManagerComponent::GetDiggerPlayerIndexAtWorldPosition(
	const glm::vec2& worldPos,
	float radius) const
{
	if (!m_PlayerManager)
		return -1;

	return m_PlayerManager->GetDiggerPlayerIndexAtWorldPosition(worldPos, radius);
}

int digger::GameManagerComponent::GetPlayerIndexAtGridPosition(
	const glm::ivec2& pos) const
{
	if (!m_PlayerManager)
		return -1;

	return m_PlayerManager->GetPlayerIndexAtGridPosition(pos);
}

bool digger::GameManagerComponent::IsPlayerAlive(int playerIndex) const
{
	if (!m_PlayerManager)
		return false;

	return m_PlayerManager->IsPlayerAlive(playerIndex);
}

bool digger::GameManagerComponent::IsPlayerControllable(
	int playerIndex) const
{
	if (!m_PlayerManager)
		return false;

	return m_PlayerManager->IsPlayerControllable(*this, playerIndex);
}

bool digger::GameManagerComponent::CanPlayerShootFireball(
	int playerIndex) const
{
	if (!m_PlayerManager)
		return false;

	return m_PlayerManager->CanPlayerShootFireball(*this, playerIndex);
}

void digger::GameManagerComponent::StartPlayerFireballCooldown(
	int playerIndex,
	float cooldown)
{
	if (m_PlayerManager)
		m_PlayerManager->StartPlayerFireballCooldown(playerIndex, cooldown);
}

void digger::GameManagerComponent::SetPlayerFireballActive(
	int playerIndex,
	bool active)
{
	if (m_PlayerManager)
		m_PlayerManager->SetPlayerFireballActive(playerIndex, active);
}

void digger::GameManagerComponent::ClearAllPlayerFireballStates()
{
	if (m_PlayerManager)
		m_PlayerManager->ClearAllPlayerFireballStates();
}

void digger::GameManagerComponent::DamagePlayer(int playerIndex)
{
	if (m_ScreenState != GameScreenState::Playing)
		return;

	if (IsGameplayFrozen())
		return;

	if (m_PlayerManager)
		m_PlayerManager->DamagePlayer(*this, playerIndex);
}


bool digger::GameManagerComponent::AreAllPlayersDead() const
{
	if (!m_PlayerManager)
		return true;

	return m_PlayerManager->AreAllPlayersDead();
}

void digger::GameManagerComponent::CheckVersusCollision()
{
	if (m_PlayerManager)
		m_PlayerManager->CheckVersusCollision(*this);
}

int digger::GameManagerComponent::GetPlayerCount() const
{
	int count = 0;

	for (const auto& player : GetPlayers())
	{
		if (player.object)
			++count;
	}

	return count;
}

bool digger::GameManagerComponent::IsPlayerAtGridPosition(
	const glm::ivec2& pos) const
{
	return GetPlayerIndexAtGridPosition(pos) != -1;
}