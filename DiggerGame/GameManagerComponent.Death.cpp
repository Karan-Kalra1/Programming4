#include "GameManagerComponent.h"

#include "DeathSequenceController.h"
#include "PlayerManager.h"



void digger::GameManagerComponent::StartDeathSequenceAt(
	const glm::vec2& position)
{
	if (m_DeathSequenceController)
		m_DeathSequenceController->Begin(position);
}

void digger::GameManagerComponent::FinishPlayerDeathSequence()
{
	if (m_PlayerManager)
		m_PlayerManager->FinishDeathSequence(*this);
}