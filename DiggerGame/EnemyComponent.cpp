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


digger::EnemyComponent::EnemyComponent(dae::GameObject* owner, GameManagerComponent* manager)
	: Component(owner)
	, m_Manager(manager)
{
	ChangeState(std::make_unique<NobbinState>());
}

digger::EnemyComponent::~EnemyComponent() = default;

void digger::EnemyComponent::Update()
{
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
	auto* grid = GetOwner()->GetComponent<GridPositionComponent>();
	return grid ? grid->GetGridPosition() : glm::ivec2{};
}


glm::ivec2 digger::EnemyComponent::GetPlayerGridPosition() const
{
	return m_Manager ? m_Manager->GetPlayerGridPosition() : glm::ivec2{};
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
	return GetGridPosition() == GetPlayerGridPosition();
}

void digger::EnemyComponent::MoveTowardPlayer(bool canDig)
{
	auto* grid = GetOwner()->GetComponent<GridPositionComponent>();
	if (!grid)
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

	glm::ivec2 direction = current - start;

	if (canDig)
		DigTile(current);

	grid->Move(direction);
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