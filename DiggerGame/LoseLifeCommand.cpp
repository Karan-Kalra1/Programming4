#include "LoseLifeCommand.h"
#include "EventBus.h"
#include "Event.h"
#include "GameObject.h"

void dae::LoseLifeCommand::Execute()
{
	if (!m_pActor)
		return;

	dae::EventBus::GetInstance().GetSubject().Notify(
		dae::Event{ dae::EventType::DamageRequested, m_pActor, m_Amount });
}