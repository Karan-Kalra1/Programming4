#include "LivesDisplayObserver.h"
#include "GameActorComponent.h"
#include "TextComponent.h"

dae::LivesDisplayObserver::LivesDisplayObserver(GameActorComponent* actor, TextComponent* text, const std::string& prefix)
	: m_pActor(actor)
	, m_pText(text)
	, m_Prefix(prefix)
{
}

void dae::LivesDisplayObserver::StartListening()
{
	if (m_pActor)
		m_pActor->GetSubject().AddObserver(this);

	Refresh();
}

void dae::LivesDisplayObserver::StopListening()
{
	if (m_pActor)
		m_pActor->GetSubject().RemoveObserver(this);
}

void dae::LivesDisplayObserver::OnNotify(const Event& event)
{
	if (!m_pActor || event.actor != m_pActor->GetOwner())
		return;

	if (event.type == EventType::LivesChanged || event.type == EventType::PlayerDied)
	{
		Refresh();
	}
}

void dae::LivesDisplayObserver::Refresh()
{
	if (m_pText && m_pActor)
	{
		m_pText->SetText(m_Prefix + std::to_string(m_pActor->GetLives()));
	}
}