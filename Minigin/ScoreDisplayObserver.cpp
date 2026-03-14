#include "ScoreDisplayObserver.h"
#include "GameActorComponent.h"
#include "TextComponent.h"

dae::ScoreDisplayObserver::ScoreDisplayObserver(GameActorComponent* actor, TextComponent* text, const std::string& prefix)
	: m_pActor(actor)
	, m_pText(text)
	, m_Prefix(prefix)
{
}

void dae::ScoreDisplayObserver::StartListening()
{
	if (m_pActor)
		m_pActor->GetSubject().AddObserver(this);

	Refresh();
}

void dae::ScoreDisplayObserver::StopListening()
{
	if (m_pActor)
		m_pActor->GetSubject().RemoveObserver(this);
}

void dae::ScoreDisplayObserver::OnNotify(const Event& event)
{
	if (!m_pActor || event.actor != m_pActor->GetOwner())
		return;

	if (event.type == EventType::ScoreChanged)
	{
		Refresh();
	}
}

void dae::ScoreDisplayObserver::Refresh()
{
	if (m_pText && m_pActor)
	{
		m_pText->SetText(m_Prefix + std::to_string(m_pActor->GetScore()));
	}
}