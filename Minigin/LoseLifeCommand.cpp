#include "LoseLifeCommand.h"
#include "GameActorComponent.h"

void dae::LoseLifeCommand::Execute()
{
	if (m_pActor)
		m_pActor->LoseLife();
}