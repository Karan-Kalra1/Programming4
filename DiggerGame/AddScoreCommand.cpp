#include "AddScoreCommand.h"
#include "EventBus.h"
#include "Event.h"
#include "GameObject.h"

void dae::AddScoreCommand::Execute()
{
	if (!m_pActor)
		return;

	dae::EventBus::GetInstance().GetSubject().Notify(
		dae::Event{ dae::EventType::ScoreRequested, m_pActor, m_Amount });
}