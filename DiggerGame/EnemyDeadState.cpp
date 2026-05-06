#include "EnemyDeadState.h"
#include "EnemyComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"

void digger::EnemyDeadState::OnEnter(EnemyComponent& enemy)
{
	if (auto* tr = enemy.GetGameObject()->GetComponent<dae::TransformComponent>())
		tr->SetLocalPosition(-1000.f, -1000.f);
}