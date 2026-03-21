#include "GameActorComponent.h"
#include "Event.h"
#include "GameObject.h"

dae::GameActorComponent::GameActorComponent(GameObject* owner, int lives, int score, int playerIndex)
	: Component(owner)
	, m_Lives(lives)
	, m_Score(score)
	, m_PlayerIndex(playerIndex)
{
}

void dae::GameActorComponent::OnNotify(const Event& event)
{
	if (event.actor != GetOwner())
		return;

	switch (event.type)
	{
	case EventType::DamageRequested:
		LoseLife(event.value);
		break;

	case EventType::ScoreRequested:
		AddScore(event.value);
		break;

	default:
		break;
	}
}

void dae::GameActorComponent::LoseLife(int amount)
{
	m_Lives -= amount;
	if (m_Lives < 0)
		m_Lives = 0;

	m_Subject.Notify(Event{ EventType::LivesChanged, GetOwner(), m_Lives });

	if (m_Lives == 0)
	{
		m_Subject.Notify(Event{ EventType::PlayerDied, GetOwner(), 0 });
	}
}

void dae::GameActorComponent::AddScore(int amount)
{
	m_Score += amount;
	if (m_Score < 0)
		m_Score = 0;

	m_Subject.Notify(Event{ EventType::ScoreChanged, GetOwner(), m_Score });
}