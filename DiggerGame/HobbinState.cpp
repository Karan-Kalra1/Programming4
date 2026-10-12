#include "HobbinState.h"
#include "NobbinState.h"
#include "EnemyComponent.h"
#include "GameManagerComponent.h"
#include "MiniginTime.h"
#include "GameObject.h"
#include "AnimationComponent.h"

void digger::HobbinState::OnEnter(EnemyComponent& enemy)
{
	m_MoveTimer = 0.f;
	m_StateTimer = 0.f;

	auto* animation =
		enemy.GetGameObject()->GetComponent<dae::AnimationComponent>();

	if (animation)
	{
		animation->Play("HobbinWalk", true);
	}
}


void digger::HobbinState::Update(EnemyComponent& enemy)
{
	m_StateTimer += dae::MiniginTime::GetDeltaTime();

	const int touchedPlayer = enemy.GetTouchingPlayerIndex();

	if (touchedPlayer != -1)
	{
		enemy.GetManager()->DamagePlayer(touchedPlayer);
		return;
	}

	if (m_StateTimer >= m_HobbinDuration)
	{
		enemy.ChangeState(std::make_unique<NobbinState>());
		return;
	}

	enemy.MoveTowardPlayer(true);
}

void digger::HobbinState::OnExit(EnemyComponent&)
{
	
}