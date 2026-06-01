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
	const int touchedPlayer = enemy.GetTouchingPlayerIndex();

	if (touchedPlayer != -1)
	{
		enemy.GetManager()->DamagePlayer(touchedPlayer);
		return;
	}

	enemy.MoveTowardPlayer(false);
}