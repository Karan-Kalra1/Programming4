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
	return m_Manager ? m_Manager->GetPlayerGridPosition() : glm::ivec2{};
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

	if (auto* tr = GetGameObject()->GetComponent<dae::TransformComponent>())
		tr->SetLocalPosition(m_WorldPosition.x, m_WorldPosition.y);
}

bool digger::EnemyComponent::CanMoveTo(const glm::ivec2& pos, bool canDig) const
{
	
	return m_Manager && m_Manager->CanEnemyMoveTo(pos, canDig);
	
}

void digger::EnemyComponent::DigTile(const glm::ivec2& pos)
{
	if (m_Manager)
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

			if (!CanMoveTo(next, canDig))
				continue;

			if (cameFrom.contains(next))
				continue;

			frontier.push(next);
			cameFrom[next] = current;
		}
	}

	if (!found)
		return;

	glm::ivec2 current = goal;

	while (cameFrom[current] != start)
	{
		current = cameFrom[current];
	}

	m_TargetGrid = current;

	m_TargetWorldPosition = GridToWorld(
		m_TargetGrid,
		m_Manager->GetTileSize(),
		m_Manager->GetMapOffset());

	if (canDig)
		DigTile(m_TargetGrid);

	m_IsMoving = true;
}

dae::GameObject* digger::EnemyComponent::GetGameObject() const
{
	return GetOwner();
}

void digger::EnemyComponent::RegisterEnemyCollision()
{
	++m_CrossCounter;

	if (m_CrossCounter >= 3)
	{
		m_CrossCounter = 0;
		ChangeState(std::make_unique<HobbinState>());
	}
}

void digger::EnemyComponent::ResetCollisionCounter()
{
	m_CrossCounter = 0;
}