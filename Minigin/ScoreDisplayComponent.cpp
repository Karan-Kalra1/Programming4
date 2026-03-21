#include "ScoreDisplayComponent.h"
#include "GameActorComponent.h"
#include "GameObject.h"
#include "TextComponent.h"

dae::ScoreDisplayComponent::ScoreDisplayComponent(GameObject* owner, GameActorComponent* actor, const std::string& prefix)
	: Component(owner)
	, m_pActor(actor)
	, m_Prefix(prefix)
{
	if (m_pActor)
	{
		m_pActor->GetSubject().AddObserver(this);
		RefreshText(m_pActor->GetScore());
	}
}


void dae::ScoreDisplayComponent::OnNotify(const Event& event)
{
	if (!m_pActor || event.actor != m_pActor->GetOwner())
		return;

	if (event.type == EventType::ScoreChanged)
	{
		RefreshText(m_pActor->GetScore());
	}
}

void dae::ScoreDisplayComponent::RefreshText(int score)
{
	auto* text = GetOwner()->GetComponent<TextComponent>();
	if (text)
		text->SetText(m_Prefix + std::to_string(score));
}