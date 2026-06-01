#include "GridPositionComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"

void digger::GridPositionComponent::SetGridPosition(const glm::ivec2& pos)
{
	m_GridPosition = pos;
	UpdateWorldPosition();
}

void digger::GridPositionComponent::UpdateWorldPosition()
{
	auto* transform = GetOwner()->GetComponent<dae::TransformComponent>();
	if (!transform)
		return;

	transform->SetLocalPosition(
		m_Offset.x + static_cast<float>(m_GridPosition.x * m_TileSize),
		m_Offset.y + static_cast<float>(m_GridPosition.y * m_TileSize));
}

