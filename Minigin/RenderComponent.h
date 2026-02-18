#pragma once
#include <memory>
#include "Component.h"

namespace dae
{
	class Texture2D;

	class RenderComponent final : public Component
	{
	public:
		RenderComponent(std::shared_ptr<Texture2D> texture)
			: m_texture(texture)
		{
		}

		void Render() const override;

	private:
		std::shared_ptr<Texture2D> m_texture;
	};
}
