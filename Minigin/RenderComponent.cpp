#include "RenderComponent.h"

#include "Renderer.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "Texture2D.h"

void dae::RenderComponent::Render() const
{
	if (!m_texture)
		return;

	auto* transform =
		GetOwner()->GetComponent<TransformComponent>();

	if (!transform)
		return;

	const auto& pos = transform->GetWorldPosition();
	const auto& scale = transform->GetWorldScale();

	if (m_UseFixedSize)
	{
		Renderer::GetInstance().RenderTextureSized(
			*m_texture,
			pos.x,
			pos.y,
			m_Width,
			m_Height);
	}
	else
	{
		Renderer::GetInstance().RenderTexture(
			*m_texture,
			pos.x,
			pos.y,
			scale.x,
			scale.y);
	}
}