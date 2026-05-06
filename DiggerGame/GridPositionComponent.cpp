#include "GridPositionComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"

void digger::GridPositionComponent::UpdateWorldPosition()
{
	auto* transform = GetOwner()->GetComponent<dae::TransformComponent>();
	if (!transform)
		return;

	transform->SetLocalPosition(
		static_cast<float>(m_GridPosition.x * m_TileSize),
		static_cast<float>(m_GridPosition.y * m_TileSize));
}