#include "NobbinState.h"
#include "EnemyComponent.h"
#include "GameManagerComponent.h"
#include "MiniginTime.h"
#include "EventBus.h"
#include "Event.h"

void digger::NobbinState::OnEnter(EnemyComponent& enemy)
{
	m_MoveTimer = 0.f;
	enemy.ResetCollisionCounter();
}

void digger::NobbinState::Update(EnemyComponent& enemy)
{
	m_MoveTimer += dae::MiniginTime::GetDeltaTime();

	if (enemy.IsOnPlayer())
	{
		enemy.GetManager()->DamagePlayer();
		return;
	}

	if (m_MoveTimer < m_MoveDelay)
		return;

	m_MoveTimer = 0.f;

	// Nobbins chase player, but cannot dig.
	enemy.MoveTowardPlayer(false);
}