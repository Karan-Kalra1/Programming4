#pragma once
#include <memory>
#include "Component.h"

namespace dae
{
	class Texture2D;

	class RenderComponent final : public Component
	{
	public:

		// Normal render using transform scale
		RenderComponent(
			GameObject* owner,
			std::shared_ptr<Texture2D> texture)
			: Component(owner)
			, m_texture(std::move(texture))
		{
		}

		// Fixed-size render (used for dirt tiles)
		RenderComponent(
			GameObject* owner,
			std::shared_ptr<Texture2D> texture,
			float width,
			float height)
			: Component(owner)
			, m_texture(std::move(texture))
			, m_UseFixedSize(true)
			, m_Width(width)
			, m_Height(height)
		{
		}

		void Render() const override;

		void SetTexture(std::shared_ptr<Texture2D> texture)
		{
			m_texture = std::move(texture);
		}

	private:
		std::shared_ptr<Texture2D> m_texture{};

		bool m_UseFixedSize{};
		float m_Width{};
		float m_Height{};
	};
}