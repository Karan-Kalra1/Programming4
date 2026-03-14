#include "AddScoreCommand.h"
#include "GameActorComponent.h"

void dae::AddScoreCommand::Execute()
{
	if (m_pActor)
		m_pActor->AddScore(m_Amount);
}