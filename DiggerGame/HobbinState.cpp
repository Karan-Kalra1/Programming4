#include "HobbinState.h"
#include "NobbinState.h"
#include "EnemyComponent.h"
#include "GameManagerComponent.h"
#include "MiniginTime.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "ResourceManager.h"

void digger::HobbinState::OnEnter(EnemyComponent& enemy)
{
	m_MoveTimer = 0.f;
	m_StateTimer = 0.f;

	auto* render = enemy.GetGameObject()->GetComponent<dae::RenderComponent>();
	if (render)
	{
		render->SetTexture(
			dae::ResourceManager::GetInstance().LoadTexture("Hobbin.png"));
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

void digger::HobbinState::OnExit(EnemyComponent& enemy)
{
	auto* render = enemy.GetGameObject()->GetComponent<dae::RenderComponent>();
	if (render)
	{
		render->SetTexture(
			dae::ResourceManager::GetInstance().LoadTexture("Nobbin.png"));
	}
}