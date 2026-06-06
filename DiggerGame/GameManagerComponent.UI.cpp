#include "GameManagerComponent.h"
#include "HudController.h"

void digger::GameManagerComponent::CreateHUD()
{
	if (m_HudController)
		m_HudController->Create(*this);
}

void digger::GameManagerComponent::UpdateHUD()
{
	if (m_HudController)
		m_HudController->Update(*this);
}

void digger::GameManagerComponent::ClearHUD()
{
	if (m_HudController)
		m_HudController->Clear();
}