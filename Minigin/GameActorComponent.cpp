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

void dae::GameActorComponent::LoseLife()
{
	--m_Lives;
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

void dae::GameActorComponent::SetLives(int lives)
{
	m_Lives = lives;
	if (m_Lives < 0)
		m_Lives = 0;

	m_Subject.Notify(Event{ EventType::LivesChanged, GetOwner(), m_Lives });

	if (m_Lives == 0)
	{
		m_Subject.Notify(Event{ EventType::PlayerDied, GetOwner(), 0 });
	}
}

void dae::GameActorComponent::SetScore(int score)
{
	m_Score = score;
	if (m_Score < 0)
		m_Score = 0;

	m_Subject.Notify(Event{ EventType::ScoreChanged, GetOwner(), m_Score });
}