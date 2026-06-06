#include "GameManagerComponent.h"

#include "FireballManager.h"


//Cooldown/Shooting

float digger::GameManagerComponent::GetFireballCooldown() const
{
	if (!m_FireballManager)
		return 1.5f;

	return m_FireballManager->GetCooldown(*this);
}

void digger::GameManagerComponent::ShootFireball(int playerIndex)
{
	if (!m_FireballManager)
		return;

	m_FireballManager->Shoot(*this, playerIndex);
}



//Removing the fireball

void digger::GameManagerComponent::RemoveFireball(
	dae::GameObject* fireball)
{
	if (!m_FireballManager)
		return;

	m_FireballManager->Remove(*this, fireball);
}

void digger::GameManagerComponent::RemoveAllFireballs()
{
	if (!m_FireballManager)
		return;

	m_FireballManager->RemoveAll(*this);
}


//Collision Checks

void digger::GameManagerComponent::CheckFireballHit(
	dae::GameObject* fireball)
{
	if (!m_FireballManager)
		return;

	m_FireballManager->CheckHit(*this, fireball);
}

bool digger::GameManagerComponent::IsBlockingTileForFireball(
	const glm::ivec2& pos) const
{
	if (!m_FireballManager)
		return true;

	return m_FireballManager->IsBlockingTileForFireball(*this, pos);
}