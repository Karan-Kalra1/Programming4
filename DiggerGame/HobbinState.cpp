#include "HobbinState.h"
#include "NobbinState.h"
#include "EnemyComponent.h"
#include "GameManagerComponent.h"
#include "MiniginTime.h"
#include "GameObject.h"
#include "TransformComponent.h"

void digger::HobbinState::OnEnter(EnemyComponent& enemy)
{
	m_MoveTimer = 0.f;
	m_StateTimer = 0.f;

	//make Hobbin slightly larger.
	if (auto* tr = enemy.GetGameObject()->GetComponent<dae::TransformComponent>())
		tr->SetLocalScale(1.05f, 1.05f);
}

void digger::HobbinState::Update(EnemyComponent& enemy)
{
	m_StateTimer += dae::MiniginTime::GetDeltaTime();

	if (enemy.IsOnPlayer())
	{
		enemy.GetManager()->DamagePlayer();
		return;
	}

	if (m_StateTimer >= m_HobbinDuration)
	{
		enemy.ChangeState(std::make_unique<NobbinState>());
		return;
	}

	enemy.MoveTowardPlayer(true);
}

void digger::HobbinState::OnExit(EnemyComponent& enemy)
{
	if (auto* tr = enemy.GetGameObject()->GetComponent<dae::TransformComponent>())
		tr->SetLocalScale(0.05f, 0.05f);
}