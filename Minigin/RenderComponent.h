#pragma once
#include <memory>
#include <SDL3/SDL_rect.h>
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

		void SetSourceRect(float x, float y, float width, float height)
		{
			m_SourceRect = { x, y, width, height };
			m_UseSourceRect = true;
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

		SDL_FRect m_SourceRect{};
		bool m_UseSourceRect = false;

		bool m_UseFixedSize{};
		float m_Width{};
		float m_Height{};
	};
}