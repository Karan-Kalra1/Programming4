#include "GridMovementComponent.h"

#include "GameManagerComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "MiniginTime.h"
#include "MoneyBagComponent.h"

#include <cmath>

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

void digger::GridMovementComponent::SetGridPosition(const glm::ivec2& position)
{
	m_WorldPosition = {
		m_Offset.x + static_cast<float>(position.x * m_TileSize),
		m_Offset.y + static_cast<float>(position.y * m_TileSize)
	};

	m_CurrentDirection = {};
	m_LastDirection = {};
	m_PendingTurnDirection = {};
	m_AligningForTurn = false;
	m_TurnKeyHeld = false;

	UpdateTransform();
}

void digger::GridMovementComponent::PressDirection(const glm::ivec2& direction)
{
	if (direction == glm::ivec2{})
		return;

	if (direction.x > 0) m_HeldRight = true;
	if (direction.x < 0) m_HeldLeft = true;
	if (direction.y > 0) m_HeldDown = true;
	if (direction.y < 0) m_HeldUp = true;

	if (m_CurrentDirection == glm::ivec2{})
	{
		if (m_LastDirection == glm::ivec2{})
		{
			SetCurrentDirection(direction);
			return;
		}

		if (IsSameAxis(m_LastDirection, direction))
		{
			SetCurrentDirection(direction);
			return;
		}

		SetCurrentDirection(m_LastDirection);
		m_PendingTurnDirection = direction;
		m_AligningForTurn = true;
		m_TurnKeyHeld = true;
		m_AlignToNearestGrid = true;
		return;
	}

	if (direction == m_CurrentDirection)
		return;

	if (IsSameAxis(m_CurrentDirection, direction))
	{
		SetCurrentDirection(direction);
		m_PendingTurnDirection = {};
		m_AligningForTurn = false;
		m_TurnKeyHeld = false;
		return;
	}

	m_PendingTurnDirection = direction;
	m_AligningForTurn = true;
	m_TurnKeyHeld = true;
	m_AlignToNearestGrid = false;
}

void digger::GridMovementComponent::ReleaseDirection(const glm::ivec2& direction)
{
	if (direction.x > 0) m_HeldRight = false;
	if (direction.x < 0) m_HeldLeft = false;
	if (direction.y > 0) m_HeldDown = false;
	if (direction.y < 0) m_HeldUp = false;

	if (direction == m_PendingTurnDirection)
	{
		m_TurnKeyHeld = false;
	}

	if (direction == m_CurrentDirection && !m_AligningForTurn)
	{
		m_LastDirection = m_CurrentDirection;
		m_CurrentDirection = {};
	}
}

void digger::GridMovementComponent::Update()
{
	if (m_Manager && m_Manager->IsGameplayFrozen())
		return;

	const float dt = dae::MiniginTime::GetDeltaTime();

	if (m_CurrentDirection == glm::ivec2{})
		return;

	const float moveDistance = m_Speed * dt;

	if (m_AligningForTurn)
	{
		const bool movingHorizontally = m_CurrentDirection.x != 0;

		const float offsetAxis = movingHorizontally
			? m_Offset.x
			: m_Offset.y;

		const float axis = movingHorizontally
			? m_WorldPosition.x
			: m_WorldPosition.y;

		const float localAxis = axis - offsetAxis;
		const float tileSize = static_cast<float>(m_TileSize);
		const float gridValue = localAxis / tileSize;

		float targetGridLocal{};

		if (m_AlignToNearestGrid)
		{
			targetGridLocal = std::round(gridValue) * tileSize;
		}
		else
		{
			if (m_CurrentDirection.x > 0 || m_CurrentDirection.y > 0)
				targetGridLocal = std::ceil(gridValue) * tileSize;
			else
				targetGridLocal = std::floor(gridValue) * tileSize;
		}

		const float targetAxis = offsetAxis + targetGridLocal;
		const float distanceToGrid = targetAxis - axis;

		glm::vec2 nextWorldPosition = m_WorldPosition;

		if (std::abs(distanceToGrid) <= moveDistance)
		{
			if (movingHorizontally)
				nextWorldPosition.x = targetAxis;
			else
				nextWorldPosition.y = targetAxis;
		}
		else
		{
			const float sign = distanceToGrid > 0.f ? 1.f : -1.f;

			if (movingHorizontally)
				nextWorldPosition.x += sign * moveDistance;
			else
				nextWorldPosition.y += sign * moveDistance;
		}

		const glm::ivec2 alignmentDirection =
		{
			movingHorizontally
				? (nextWorldPosition.x > m_WorldPosition.x ? 1 : -1)
				: 0,

			movingHorizontally
				? 0
				: (nextWorldPosition.y > m_WorldPosition.y ? 1 : -1)
		};

		if (!CanMoveToWorldPosition(nextWorldPosition, alignmentDirection, false))
		{
			m_AligningForTurn = false;
			m_PendingTurnDirection = {};
			m_TurnKeyHeld = false;
			m_CurrentDirection = {};
			m_AlignToNearestGrid = false;
			UpdateTransform();
			return;
		}

		m_WorldPosition = nextWorldPosition;

		if (std::abs(distanceToGrid) <= moveDistance)
		{
			if (m_PendingTurnDirection != glm::ivec2{} && m_TurnKeyHeld)
			{
				if (CanMoveToWorldPosition(
					m_WorldPosition,
					m_PendingTurnDirection,
					false))
				{
					SetCurrentDirection(m_PendingTurnDirection);
				}
				else
				{
					m_LastDirection = m_CurrentDirection;
					m_CurrentDirection = {};
				}
			}
			else
			{
				m_LastDirection = m_CurrentDirection;
				m_CurrentDirection = {};
			}

			m_PendingTurnDirection = {};
			m_AligningForTurn = false;
			m_TurnKeyHeld = false;
			m_AlignToNearestGrid = false;
		}

		Dig();
		UpdateTransform();
		return;
	}

	const glm::vec2 movement{
		static_cast<float>(m_CurrentDirection.x),
		static_cast<float>(m_CurrentDirection.y)
	};

	const glm::vec2 nextWorldPosition =
		m_WorldPosition + movement * moveDistance;

	if (!CanMoveToWorldPosition(
		nextWorldPosition,
		m_CurrentDirection,
		true))
	{
		return;
	}

	m_WorldPosition = nextWorldPosition;

	Dig();
	UpdateTransform();
}

glm::ivec2 digger::GridMovementComponent::GetGridPosition() const
{
	return WorldToGrid(m_WorldPosition);
}

glm::ivec2 digger::GridMovementComponent::WorldToGrid(const glm::vec2& world) const
{
	const int x = static_cast<int>(
		std::round((world.x - m_Offset.x) / static_cast<float>(m_TileSize)));

	const int y = static_cast<int>(
		std::round((world.y - m_Offset.y) / static_cast<float>(m_TileSize)));

	return { x, y };
}

bool digger::GridMovementComponent::IsSameAxis(const glm::ivec2& a, const glm::ivec2& b) const
{
	if (a == glm::ivec2{} || b == glm::ivec2{})
		return false;

	const bool aHorizontal = a.x != 0;
	const bool bHorizontal = b.x != 0;

	return aHorizontal == bHorizontal;
}

bool digger::GridMovementComponent::IsDirectionHeld(const glm::ivec2& direction) const
{
	if (direction.x > 0) return m_HeldRight;
	if (direction.x < 0) return m_HeldLeft;
	if (direction.y > 0) return m_HeldDown;
	if (direction.y < 0) return m_HeldUp;

	return false;
}

void digger::GridMovementComponent::SetCurrentDirection(const glm::ivec2& direction)
{
	m_CurrentDirection = direction;

	if (direction != glm::ivec2{})
	{
		m_LastDirection = direction;
		m_FacingDirection = direction;
	}
		

	ApplyRotation();
}

void digger::GridMovementComponent::ApplyRotation()
{
	auto* transform = GetOwner()->GetComponent<dae::TransformComponent>();
	if (!transform)
		return;

	transform->SetRotation(0.0);
	transform->SetFlipX(false);
	transform->SetFlipY(false);

	if (m_CurrentDirection.x > 0)
	{
		// right
		transform->SetRotation(0.0);
		transform->SetFlipX(false);
	}
	else if (m_CurrentDirection.x < 0)
	{
		// left
		transform->SetRotation(0.0);
		transform->SetFlipX(true);
	}
	else if (m_CurrentDirection.y < 0)
	{
		// up
		transform->SetRotation(-90.0);
		transform->SetFlipX(false);
		transform->SetFlipY(false);
	}
	else if (m_CurrentDirection.y > 0)
	{
		// down
		transform->SetRotation(-90.0);
		transform->SetFlipX(true);
		//transform->SetFlipY(true);
	}
}

void digger::GridMovementComponent::ReleaseAllDirections()
{
	m_CurrentDirection = {};
	m_LastDirection = {};
	m_PendingTurnDirection = {};

	m_AligningForTurn = false;
	m_TurnKeyHeld = false;

	m_HeldUp = false;
	m_HeldDown = false;
	m_HeldLeft = false;
	m_HeldRight = false;
}

void digger::GridMovementComponent::UpdateTransform()
{
	if (auto* transform = GetOwner()->GetComponent<dae::TransformComponent>())
		transform->SetLocalPosition(m_WorldPosition.x, m_WorldPosition.y);
}

void digger::GridMovementComponent::Dig()
{
	if (!m_Manager)
		return;

	if (!m_CanDigDirt)
		return;

	m_Manager->DigAtWorldPosition(
		m_WorldPosition + m_PlayerCenterOffset,
		m_CurrentDirection);
}

bool digger::GridMovementComponent::CanOccupyPosition(
	const glm::vec2& worldPos) const
{
	if (!m_Manager)
		return true;

	const glm::ivec2 grid = WorldToGrid(worldPos);

	if (m_Manager->HasMoneyBagAt(grid))
		return false;

	if (!m_CanEnterDirt && m_Manager->IsDirt(grid))
		return false;

	return m_Manager->CanPlayerMoveTo(grid);
}

glm::ivec2 digger::GridMovementComponent::WorldPointToGrid(
	const glm::vec2& worldPoint) const
{
	const int x = static_cast<int>(
		std::floor((worldPoint.x - m_Offset.x) / static_cast<float>(m_TileSize)));

	const int y = static_cast<int>(
		std::floor((worldPoint.y - m_Offset.y) / static_cast<float>(m_TileSize)));

	return { x, y };
}

glm::ivec2 digger::GridMovementComponent::GetFrontGridPosition(
	const glm::vec2& worldPosition,
	const glm::ivec2& direction) const
{
	const glm::vec2 center =
		worldPosition + m_PlayerCenterOffset;

	glm::vec2 frontPoint = center;

	if (direction.x > 0)
		frontPoint.x += m_CollisionRadius;
	else if (direction.x < 0)
		frontPoint.x -= m_CollisionRadius;
	else if (direction.y > 0)
		frontPoint.y += m_CollisionRadius;
	else if (direction.y < 0)
		frontPoint.y -= m_CollisionRadius;

	return WorldPointToGrid(frontPoint);
}

bool digger::GridMovementComponent::CanMoveToWorldPosition(
	const glm::vec2& worldPosition,
	const glm::ivec2& direction,
	bool allowPush)
{
	if (!m_Manager)
		return true;

	const glm::ivec2 frontGrid =
		GetFrontGridPosition(worldPosition, direction);

	// Money bag blocks movement unless pushed.
	if (m_Manager->HasMoneyBagAt(frontGrid))
	{
		if (!allowPush)
			return false;

		return m_Manager->TryPushMoneyBagAt(frontGrid, direction);
	}

	// Versus enemy / non-digger cannot enter dirt.
	if (!m_CanEnterDirt && m_Manager->IsDirt(frontGrid))
		return false;

	return m_Manager->CanPlayerMoveTo(frontGrid);
}

