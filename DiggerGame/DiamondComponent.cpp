#include "DiamondComponent.h"
#include "GameManagerComponent.h"
#include "GameObject.h"
#include "GridPositionComponent.h"

void digger::DiamondComponent::Update()
{
	if (m_Collected || !m_Manager)
		return;

	auto* myGrid = GetOwner()->GetComponent<GridPositionComponent>();
	auto* playerGrid = m_Manager->GetPlayer()->GetComponent<GridPositionComponent>();

	if (!myGrid || !playerGrid)
		return;

	if (myGrid->GetGridPosition() == playerGrid->GetGridPosition())
	{
		m_Collected = true;
		m_Manager->CollectDiamond(GetOwner());
	}
}