#pragma once
#include <memory>
#include "Component.h"

namespace dae
{
	class Texture2D;

	class RenderComponent final : public Component
	{
	public:
		RenderComponent(GameObject* owner,std::shared_ptr<Texture2D> texture)
			: Component::Component(owner),m_texture(texture)
		{
			
		}

		void Render() const override;

	private:
		std::shared_ptr<Texture2D> m_texture;
	};
}
