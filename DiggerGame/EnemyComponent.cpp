#include "EnemyComponent.h"
#include "EnemyState.h"
#include "NobbinState.h"
#include "HobbinState.h"
#include "GridPositionComponent.h"
#include "GameManagerComponent.h"
#include "GameObject.h"

#include <cmath>
#include <array>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include "MiniginTime.h"
#include "MoneyBagComponent.h"



glm::ivec2 digger::EnemyComponent::FindRoamTarget(bool canDig) const
{
	static constexpr glm::ivec2 dirs[4]
	{
		{ 1, 0 },
		{ -1, 0 },
		{ 0, 1 },
		{ 0, -1 }
	};

	const auto start = GetGridPosition();

	for (const auto& dir : dirs)
	{
		const auto next = start + dir;

		if (m_Manager && m_Manager->HasMoneyBagAt(next))
		{
			if (dir.y == 0 && m_Manager->TryPushMoneyBagAt(next, dir))
				return next;

			continue;
		}

		if (CanMoveTo(next, canDig))
			return next;
	}

	return start;
}

namespace
{
	struct Vec2Hash
	{
		size_t operator()(const glm::ivec2& v) const
		{
			return std::hash<int>()(v.x * 73856093 ^ v.y * 19349663);
		}
	};
}

glm::vec2 GridToWorld(
	const glm::ivec2& grid,
	int tileSize,
	const glm::vec2& offset)
{
	return {
		offset.x + static_cast<float>(grid.x * tileSize),
		offset.y + static_cast<float>(grid.y * tileSize)
	};
}


digger::EnemyComponent::EnemyComponent(dae::GameObject* owner, GameManagerComponent* manager)
	: Component(owner)
	, m_Manager(manager)
{
	ChangeState(std::make_unique<NobbinState>());
}

digger::EnemyComponent::~EnemyComponent() = default;

void digger::EnemyComponent::Update()
{
	if (m_Manager && m_Manager->IsGameplayFrozen())
		return;

	if (m_CrushedByBag)
		return;

	UpdateSmoothMovement();

	if (m_State)
		m_State->Update(*this);
}

void digger::EnemyComponent::ChangeState(std::unique_ptr<EnemyState> newState)
{
	if (m_State)
		m_State->OnExit(*this);

	m_State = std::move(newState);

	if (m_State)
		m_State->OnEnter(*this);
}

glm::ivec2 digger::EnemyComponent::GetGridPosition() const
{
	return m_CurrentGrid;
}


glm::ivec2 digger::EnemyComponent::GetPlayerGridPosition() const
{
	if (!m_Manager)
		return {};

	return m_Manager->GetClosestAlivePlayerGridPosition(GetGridPosition());
}

int digger::EnemyComponent::GetTouchingPlayerIndex() const
{
	if (!m_Manager)
		return -1;

	auto* myTransform = GetGameObject()->GetComponent<dae::TransformComponent>();
	if (!myTransform)
		return -1;

	const auto& myPos3 = myTransform->GetWorldPosition();

	glm::vec2 enemyCenter{
		myPos3.x + 32.f,
		myPos3.y + 32.f
	};

	return m_Manager->GetPlayerIndexAtWorldPosition(
		enemyCenter,
		m_Manager->GetCollisionRadius());
}

void digger::EnemyComponent::SetGridPosition(const glm::ivec2& pos)
{
	m_CurrentGrid = pos;
	m_TargetGrid = pos;

	m_WorldPosition = GridToWorld(pos, m_Manager->GetTileSize(), m_Manager->GetMapOffset());
	m_TargetWorldPosition = m_WorldPosition;

	if (auto* tr = GetGameObject()->GetComponent<dae::TransformComponent>())
		tr->SetLocalPosition(m_WorldPosition.x, m_WorldPosition.y);
}

void digger::EnemyComponent::UpdateSmoothMovement()
{
	if (!m_IsMoving)
		return;

	const float dt = dae::MiniginTime::GetDeltaTime();

	glm::vec2 toTarget = m_TargetWorldPosition - m_WorldPosition;
	const float distance = glm::length(toTarget);
	const float moveDistance = m_Speed * dt;

	if (distance <= moveDistance)
	{
		m_WorldPosition = m_TargetWorldPosition;
		m_CurrentGrid = m_TargetGrid;
		m_IsMoving = false;
	}
	else
	{
		m_WorldPosition += glm::normalize(toTarget) * moveDistance;
	}

	if (m_CanDigCurrentMove && m_Manager)
	{
		const glm::ivec2 direction = m_TargetGrid - m_CurrentGrid;

		m_Manager->DigAtWorldPosition(
			m_WorldPosition + glm::vec2{ 32.f, 32.f },
			direction);
	}

	if (auto* tr = GetGameObject()->GetComponent<dae::TransformComponent>())
		tr->SetLocalPosition(m_WorldPosition.x, m_WorldPosition.y);
}

bool digger::EnemyComponent::CanMoveTo(const glm::ivec2& pos, bool canDig) const
{
	
	return m_Manager && m_Manager->CanEnemyMoveTo(pos, canDig);
	
}

void digger::EnemyComponent::DigTile(const glm::ivec2& pos)
{
	if (!m_Manager)
		return;

	if (m_Manager->HasDiamondAt(pos))
		return;

	m_Manager->DigTile(pos);
}

bool digger::EnemyComponent::IsOnPlayer() const
{
	
		if (!m_Manager)
			return false;

		auto* myTransform = GetGameObject()->GetComponent<dae::TransformComponent>();
		if (!myTransform)
			return false;

		const auto& myPos3 = myTransform->GetWorldPosition();
		glm::vec2 enemyPos{ myPos3.x + 32.f, myPos3.y + 32.f };

		glm::vec2 playerPos = m_Manager->GetPlayerWorldPosition();

		const float dx = enemyPos.x - playerPos.x;
		const float dy = enemyPos.y - playerPos.y;

		const float radius = m_Manager->GetCollisionRadius();

		return (dx * dx + dy * dy) <= radius * radius;
	
}

void digger::EnemyComponent::MoveTowardPlayer(bool canDig)
{
	if (m_IsMoving)
		return;


	const glm::ivec2 start = GetGridPosition();
	const glm::ivec2 goal = GetPlayerGridPosition();

	if (start == goal)
		return;

	static constexpr glm::ivec2 dirs[4] =
	{
		{ 1, 0 },
		{ -1, 0 },
		{ 0, 1 },
		{ 0, -1 }
	};

	std::queue<glm::ivec2> frontier;
	std::unordered_map<glm::ivec2, glm::ivec2, Vec2Hash> cameFrom;

	frontier.push(start);
	cameFrom[start] = start;

	bool found = false;

	while (!frontier.empty())
	{
		glm::ivec2 current = frontier.front();
		frontier.pop();

		if (current == goal)
		{
			found = true;
			break;
		}

		for (const auto& dir : dirs)
		{
			glm::ivec2 next = current + dir;

			// Money bags are blockers during path search
			if (m_Manager && m_Manager->HasMoneyBagAt(next))
				continue;

			if (!CanMoveTo(next, canDig))
				continue;

			if (cameFrom.contains(next))
				continue;

			frontier.push(next);
			cameFrom[next] = current;
		}
	}

	if (!found)
	{
		const glm::ivec2 roamTarget = FindRoamTarget(canDig);

		if (roamTarget == start)
			return;

		m_TargetGrid = roamTarget;
		m_TargetWorldPosition = GridToWorld(
			m_TargetGrid,
			m_Manager->GetTileSize(),
			m_Manager->GetMapOffset());
		
		m_CanDigCurrentMove = canDig;
		m_IsMoving = true;
		return;
	}

	glm::ivec2 current = goal;

	while (cameFrom[current] != start)
	{
		current = cameFrom[current];
	}



	m_TargetGrid = current;

	const glm::ivec2 moveDir = m_TargetGrid - start;

	if (m_Manager && m_Manager->HasMoneyBagAt(m_TargetGrid))
	{
		if (moveDir.y != 0)
			return;

		if (!m_Manager->TryPushMoneyBagAt(m_TargetGrid, moveDir))
			return;
	}
	else if (!CanMoveTo(m_TargetGrid, canDig))
	{
		return;
	}

	m_TargetWorldPosition = GridToWorld(
		m_TargetGrid,
		m_Manager->GetTileSize(),
		m_Manager->GetMapOffset());

	
	m_CanDigCurrentMove = canDig;
	m_IsMoving = true;
}

dae::GameObject* digger::EnemyComponent::GetGameObject() const
{
	return GetOwner();
}

void digger::EnemyComponent::RegisterEnemyCollision()
{
	++m_CrossCounter;

	if (m_CrossCounter >= 6)
	{
		m_CrossCounter = 0;
		ChangeState(std::make_unique<HobbinState>());
	}
}

void digger::EnemyComponent::ResetCollisionCounter()
{
	m_CrossCounter = 0;
}

void digger::EnemyComponent::Kill()
{
	if (m_IsDead)
		return;

	m_IsDead = true;

	if (m_Manager)
		m_Manager->KillEnemy(this);
}