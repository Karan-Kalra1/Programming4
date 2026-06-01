#include "FireballComponent.h"
#include "GameManagerComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "MiniginTime.h"

digger::FireballComponent::FireballComponent(
	dae::GameObject* owner,
	GameManagerComponent* manager,
	glm::vec2 direction,
	float speed,
	int ownerPlayerIndex)
	: Component(owner)
	, m_Manager(manager)
	, m_Direction(direction)
	, m_Speed(speed)
	, m_OwnerPlayerIndex(ownerPlayerIndex)
{
}
void digger::FireballComponent::Update()
{
	if (m_Manager && m_Manager->IsGameplayFrozen())
		return;

	auto* tr = GetOwner()->GetComponent<dae::TransformComponent>();
	if (!tr || !m_Manager)
		return;

	const auto& pos3 = tr->GetWorldPosition();

	glm::vec2 pos{ pos3.x, pos3.y };
	pos += m_Direction * m_Speed * dae::MiniginTime::GetDeltaTime();

	tr->SetLocalPosition(pos.x, pos.y);

	m_Manager->CheckFireballHit(GetOwner());
}