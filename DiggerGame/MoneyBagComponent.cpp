#include "MoneyBagComponent.h"

#include "GameManagerComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "ResourceManager.h"
#include "MiniginTime.h"
#include "EnemyComponent.h"
#include "GridMovementComponent.h"
#include "EventBus.h"

#include <glm/geometric.hpp>

digger::MoneyBagComponent::MoneyBagComponent(
	dae::GameObject* owner,
	GameManagerComponent* manager,
	int tileSize,
	glm::vec2 offset)
	: Component(owner)
	, m_Manager(manager)
	, m_TileSize(tileSize)
	, m_Offset(offset)
{
}

void digger::MoneyBagComponent::Update()
{
	switch (m_State)
	{
	case MoneyBagState::Stable:
		UpdateStable();
		break;

	case MoneyBagState::WaitingToFall:
		UpdateWaitingToFall();
		break;

	case MoneyBagState::Falling:
		UpdateFalling();
		break;

	case MoneyBagState::BrokenGold:
		const auto playerPos = m_Manager->GetPlayerWorldPosition();

		auto* tr = GetOwner()->GetComponent<dae::TransformComponent>();
		const auto& pos = tr->GetWorldPosition();

		glm::vec2 goldCenter{ pos.x + 32.f, pos.y + 32.f };

		const float dx = goldCenter.x - playerPos.x;
		const float dy = goldCenter.y - playerPos.y;

		if ((dx * dx + dy * dy) < 32.f * 32.f)
			m_Manager->CollectGold(GetOwner());
		break;
	}
}

void digger::MoneyBagComponent::SetGridPosition(const glm::ivec2& pos)
{
	m_GridPosition = pos;
	m_WorldPosition = GridToWorld(pos);
	m_TargetWorldPosition = m_WorldPosition;

	if (auto* tr = GetOwner()->GetComponent<dae::TransformComponent>())
		tr->SetLocalPosition(m_WorldPosition.x, m_WorldPosition.y);
}

glm::vec2 digger::MoneyBagComponent::GridToWorld(const glm::ivec2& grid) const
{
	return {
		m_Offset.x + static_cast<float>(grid.x * m_TileSize),
		m_Offset.y + static_cast<float>(grid.y * m_TileSize)
	};
}

void digger::MoneyBagComponent::SetState(MoneyBagState state)
{
	m_State = state;
}

void digger::MoneyBagComponent::UpdateStable()
{
	if (!m_Manager)
		return;

	const glm::ivec2 below{ m_GridPosition.x, m_GridPosition.y + 1 };

	if (!m_Manager->HasDirtBelow(m_GridPosition) &&
		!m_Manager->IsPlayerAtGridPosition(below))
	{
		m_FallDelayTimer = 0.f;
		SetState(MoneyBagState::WaitingToFall);
	}
}

void digger::MoneyBagComponent::UpdateWaitingToFall()
{
	const glm::ivec2 below{ m_GridPosition.x, m_GridPosition.y + 1 };

	if (m_Manager &&
		(m_Manager->HasDirtBelow(m_GridPosition) ||
			m_Manager->IsPlayerAtGridPosition(below)))
	{
		m_FallDelayTimer = 0.f;
		SetState(MoneyBagState::Stable);
		return;
	}

	m_FallDelayTimer += dae::MiniginTime::GetDeltaTime();

	if (m_FallDelayTimer >= m_FallDelay)
	{
		m_FallDistance = 0;
		m_TargetWorldPosition = GridToWorld(m_GridPosition + glm::ivec2{ 0, 1 });
		SetState(MoneyBagState::Falling);
	}
}

void digger::MoneyBagComponent::UpdateFalling()
{
	const float dt = dae::MiniginTime::GetDeltaTime();

	glm::vec2 toTarget = m_TargetWorldPosition - m_WorldPosition;
	const float distance = glm::length(toTarget);
	const float moveDistance = m_FallSpeed * dt;

	if (distance <= moveDistance)
	{
		m_WorldPosition = m_TargetWorldPosition;
		m_GridPosition.y += 1;
		++m_FallDistance;
		m_HasFallen = true;

		if (!m_Manager || m_Manager->HasDirtBelow(m_GridPosition))
		{
			if (m_FallDistance >= 2)
			{
				BreakIntoGold();
				KillDraggedTargets();
			}
				
			else
				SetState(MoneyBagState::Stable);
		}
		else
		{
			m_TargetWorldPosition = GridToWorld(m_GridPosition + glm::ivec2{ 0, 1 });
		}
	}
	else
	{
		m_WorldPosition += glm::normalize(toTarget) * moveDistance;
	}

	if (auto* tr = GetOwner()->GetComponent<dae::TransformComponent>())
		tr->SetLocalPosition(m_WorldPosition.x, m_WorldPosition.y);

	CheckCrushTargets();
	DragTargetsDown();
}

bool digger::MoneyBagComponent::TryPush(const glm::ivec2& direction)
{
	if (m_State != MoneyBagState::Stable)
		return false;

	// Cannot push vertically
	if (direction.y != 0)
		return false;

	// Cannot push while supported by dirt
	if (!m_HasFallen && m_Manager && m_Manager->HasDirtBelow(m_GridPosition))
		return false;

	const glm::ivec2 target = m_GridPosition + direction;

	if (!m_Manager || !m_Manager->CanMoneyBagMoveTo(target))
		return false;

	SetGridPosition(target);
	return true;
}

void digger::MoneyBagComponent::BreakIntoGold()
{
	SetState(MoneyBagState::BrokenGold);

	auto* render = GetOwner()->GetComponent<dae::RenderComponent>();
	if (render)
	{
		render->SetTexture(
			dae::ResourceManager::GetInstance().LoadTexture("Gold.png"));
	}

}

void digger::MoneyBagComponent::UpdateVisual()
{
}

void digger::MoneyBagComponent::CheckCrushTargets()
{
	if (!m_Manager)
		return;

	auto* bagTr = GetOwner()->GetComponent<dae::TransformComponent>();
	if (!bagTr)
		return;

	const auto& bagPos3 = bagTr->GetWorldPosition();
	glm::vec2 bagCenter{ bagPos3.x + 32.f, bagPos3.y + 32.f };

	if (!m_DraggedPlayer)
	{
		glm::vec2 playerCenter = m_Manager->GetPlayerWorldPosition();

		const float dx = bagCenter.x - playerCenter.x;
		const float dy = bagCenter.y - playerCenter.y;

		if ((dx * dx + dy * dy) <= 30.f * 30.f)
			m_DraggedPlayer = m_Manager->GetPlayer();
	}

	if (!m_DraggedEnemy)
	{
		m_DraggedEnemy = m_Manager->GetEnemyAtWorldPosition(bagCenter, 30.f);
	}
}

void digger::MoneyBagComponent::DragTargetsDown()
{
	auto* bagTr = GetOwner()->GetComponent<dae::TransformComponent>();
	if (!bagTr)
		return;

	const auto& bagPos3 = bagTr->GetWorldPosition();

	if (m_DraggedPlayer)
	{
		if (auto* tr = m_DraggedPlayer->GetComponent<dae::TransformComponent>())
			tr->SetLocalPosition(bagPos3.x, bagPos3.y);
	}

	if (m_DraggedEnemy)
	{
		if (auto* tr = m_DraggedEnemy->GetGameObject()->GetComponent<dae::TransformComponent>())
			tr->SetLocalPosition(bagPos3.x, bagPos3.y);
	}
}

void digger::MoneyBagComponent::KillDraggedTargets()
{
	if (m_DraggedPlayer)
	{
		m_Manager->DamagePlayer();
		m_DraggedPlayer = nullptr;
	}

	if (m_DraggedEnemy)
	{
		m_DraggedEnemy->Kill();
		m_DraggedEnemy = nullptr;
	}
}
