#pragma once
#include <glm/glm.hpp>
#include "Component.h"

namespace dae
{
	class TransformComponent final : public Component
	{
	public:
		void SetPosition(float x, float y, float z = 0.f)
		{
			m_position = { x,y,z };
		}

		const glm::vec3& GetPosition() const
		{
			return m_position;
		}

	private:
		glm::vec3 m_position{};
	};
}
