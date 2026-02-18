#include "RenderComponent.h"
#include "Renderer.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "Texture2D.h"

void dae::RenderComponent::Render() const
{
	auto transform = GetOwner()->GetComponent<TransformComponent>();
	if (!transform || !m_texture)
		return;

	const auto& pos = transform->GetPosition();
	Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
}
