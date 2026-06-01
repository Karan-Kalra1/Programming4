#include "DiamondComponent.h"
#include "GameManagerComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"

void digger::DiamondComponent::Update()
{
	if (m_Manager && m_Manager->IsGameplayFrozen())
		return;

	if (m_Collected || !m_Manager)
		return;

	auto* myTransform = GetOwner()->GetComponent<dae::TransformComponent>();
	if (!myTransform)
		return;

	const auto& myPos3 = myTransform->GetWorldPosition();
	glm::vec2 diamondPos{ myPos3.x + 32.f, myPos3.y + 32.f };

	glm::vec2 playerPos = m_Manager->GetPlayerWorldPosition();

	const float dx = diamondPos.x - playerPos.x;
	const float dy = diamondPos.y - playerPos.y;

	const float radius = m_Manager->GetCollisionRadius();

	if ((dx * dx + dy * dy) <= radius * radius)
	{
		m_Collected = true;
		m_Manager->CollectDiamond(GetOwner());
	}
}