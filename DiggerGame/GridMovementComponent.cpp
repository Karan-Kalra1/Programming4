#include "GridMovementComponent.h"
#include "GameManagerComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "MiniginTime.h"

#include <cmath>
#include <algorithm>

namespace
{
	constexpr float Epsilon = 1.5f;
}

digger::GridMovementComponent::GridMovementComponent(
	dae::GameObject* owner,
	GameManagerComponent* manager,
	int tileSize,
	float speed,
	glm::vec2 offset)
	: Component(owner)
	, m_Manager(manager)
	, m_TileSize(tileSize)
	, m_Speed(speed)
	, m_Offset(offset)
{
}

bool digger::GridMovementComponent::IsChangingAxis(const glm::ivec2& direction) const
{
	return m_CurrentDirection.x != direction.x &&
		m_CurrentDirection.y != direction.y;
}

float digger::GridMovementComponent::DistanceToGridLine(bool horizontalMovement) const
{
	const float axis = horizontalMovement
		? m_WorldPosition.x - m_Offset.x
		: m_WorldPosition.y - m_Offset.y;

	const float grid = std::round(axis / m_TileSize) * m_TileSize;
	return grid - axis;
}



void digger::GridMovementComponent::SetGridPosition(const glm::ivec2& position)
{
	m_WorldPosition = {
		m_Offset.x + static_cast<float>(position.x * m_TileSize),
		m_Offset.y + static_cast<float>(position.y * m_TileSize)
	};

	if (auto* tr = GetOwner()->GetComponent<dae::TransformComponent>())
		tr->SetLocalPosition(m_WorldPosition.x, m_WorldPosition.y);
}

void digger::GridMovementComponent::PressDirection(const glm::ivec2& direction)
{
	if (direction == glm::ivec2{})
		return;

	// If stopped
	if (m_CurrentDirection == glm::ivec2{})
	{
		// If we have no previous direction, start immediately
		if (m_LastDirection == glm::ivec2{})
		{
			m_CurrentDirection = direction;
			m_LastDirection = direction;
			return;
		}

		const bool lastHorizontal = m_LastDirection.x != 0;
		const bool newHorizontal = direction.x != 0;

		// Same axis from stopped
		if (lastHorizontal == newHorizontal)
		{
			m_CurrentDirection = direction;
			m_LastDirection = direction;
			return;
		}

		// align using last direction, then turn
		m_CurrentDirection = m_LastDirection;
		m_PendingTurnDirection = direction;
		m_AligningForTurn = true;
		return;
	}

	// Same direction
	if (direction == m_CurrentDirection)
		return;

	const bool currentHorizontal = m_CurrentDirection.x != 0;
	const bool newHorizontal = direction.x != 0;

	// reverse, no grid alignment
	if (currentHorizontal == newHorizontal)
	{
		m_CurrentDirection = direction;
		m_LastDirection = direction;
		m_AligningForTurn = false;
		m_PendingTurnDirection = {};
		return;
	}

	// Axis change: align first, then turn
	m_PendingTurnDirection = direction;
	m_AligningForTurn = true;
}

void digger::GridMovementComponent::ReleaseDirection(const glm::ivec2& direction)
{
	if (direction != m_CurrentDirection)
		return;

	m_LastDirection = m_CurrentDirection;
	m_CurrentDirection = {};
	m_AligningForTurn = false;
	m_PendingTurnDirection = {};
}

void digger::GridMovementComponent::Update()
{
	const float dt = dae::MiniginTime::GetDeltaTime();

	if (m_CurrentDirection == glm::ivec2{})
		return;

	const float moveDistance = m_Speed * dt;

	
	// Align before turning

	if (m_AligningForTurn)
	{
		const bool movingHorizontally = m_CurrentDirection.x != 0;

		float& axis = movingHorizontally
			? m_WorldPosition.x
			: m_WorldPosition.y;

		const float offsetAxis = movingHorizontally
			? m_Offset.x
			: m_Offset.y;

		const float localAxis = axis - offsetAxis;

		const float tileSize = static_cast<float>(m_TileSize);
		const float gridValue = localAxis / tileSize;

		float targetGridLocal{};

		if (m_CurrentDirection.x > 0 || m_CurrentDirection.y > 0)
			targetGridLocal = std::ceil(gridValue) * tileSize;
		else
			targetGridLocal = std::floor(gridValue) * tileSize;

		const float targetAxis = offsetAxis + targetGridLocal;

		const float distanceToGrid = targetAxis - axis;

		if (std::abs(distanceToGrid) <= moveDistance)
		{
			axis = targetAxis;

			m_CurrentDirection = m_PendingTurnDirection;
			m_PendingTurnDirection = {};
			m_AligningForTurn = false;
		}
		else
		{
			const float sign = distanceToGrid > 0.f ? 1.f : -1.f;
			axis += sign * moveDistance;
		}

		if (m_Manager)
		{
			m_Manager->DigAtWorldPosition(
				m_WorldPosition + glm::vec2{ 32.f, 32.f },
				m_CurrentDirection);
		}

		if (auto* tr = GetOwner()->GetComponent<dae::TransformComponent>())
			tr->SetLocalPosition(m_WorldPosition.x, m_WorldPosition.y);

		return;
	}

	
	// Normal free movement

	glm::vec2 movement{
		static_cast<float>(m_CurrentDirection.x),
		static_cast<float>(m_CurrentDirection.y)
	};

	const glm::vec2 nextWorldPosition =
		m_WorldPosition + movement * moveDistance;

	if (m_Manager)
	{
		const glm::ivec2 nextGrid = WorldToGrid(nextWorldPosition);

		if (!m_Manager->CanPlayerMoveTo(nextGrid))
			return;
	}

	m_WorldPosition = nextWorldPosition;

	if (m_Manager)
	{
		m_Manager->DigAtWorldPosition(
			m_WorldPosition + glm::vec2{ 32.f, 32.f },
			m_CurrentDirection);
	}

	if (auto* tr = GetOwner()->GetComponent<dae::TransformComponent>())
		tr->SetLocalPosition(m_WorldPosition.x, m_WorldPosition.y);
}




bool digger::GridMovementComponent::IsDirectionChange(const glm::ivec2& newDirection) const
{
	if (m_CurrentDirection == glm::ivec2{})
		return false;

	return newDirection != m_CurrentDirection;
}

glm::ivec2 digger::GridMovementComponent::GetGridPosition() const
{
	return WorldToGrid(m_WorldPosition);
}

glm::ivec2 digger::GridMovementComponent::WorldToGrid(const glm::vec2& world) const
{
	const int x = static_cast<int>(std::round((world.x - m_Offset.x) / static_cast<float>(m_TileSize)));
	const int y = static_cast<int>(std::round((world.y - m_Offset.y) / static_cast<float>(m_TileSize)));

	return { x, y };
}