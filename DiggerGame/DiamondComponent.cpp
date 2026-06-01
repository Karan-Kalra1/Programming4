#include "DiamondComponent.h"
#include "GameManagerComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"


void digger::DiamondComponent::Update()
{
	if (m_Collected || !m_Manager)
		return;

	auto* tr = GetOwner()->GetComponent<dae::TransformComponent>();
	if (!tr)
		return;

	const auto& pos = tr->GetWorldPosition();

	glm::vec2 diamondCenter{
		pos.x + 32.f,
		pos.y + 32.f
	};

	const int playerIndex =
		m_Manager->GetDiggerPlayerIndexAtWorldPosition(
			diamondCenter,
			m_Manager->GetCollisionRadius());

	if (playerIndex != -1)
	{
		m_Collected = true;
		m_Manager->CollectDiamond(GetOwner());
	}
}