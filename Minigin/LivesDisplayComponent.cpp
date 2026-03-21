#include "LivesDisplayComponent.h"
#include "GameActorComponent.h"
#include "GameObject.h"
#include "TextComponent.h"

dae::LivesDisplayComponent::LivesDisplayComponent(GameObject* owner, GameActorComponent* actor, const std::string& prefix)
	: Component(owner)
	, m_pActor(actor)
	, m_Prefix(prefix)
{
	if (m_pActor)
	{
		m_pActor->GetSubject().AddObserver(this);
		RefreshText(m_pActor->GetLives());
	}
}


void dae::LivesDisplayComponent::OnNotify(const Event& event)
{
	if (!m_pActor || event.actor != m_pActor->GetOwner())
		return;

	if (event.type == EventType::LivesChanged || event.type == EventType::PlayerDied)
	{
		RefreshText(m_pActor->GetLives());
	}
}

void dae::LivesDisplayComponent::RefreshText(int lives)
{
	auto* text = GetOwner()->GetComponent<TextComponent>();
	if (text)
		text->SetText(m_Prefix + std::to_string(lives));
}