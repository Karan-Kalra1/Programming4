#pragma once
#include "Command.h"
#include <glm/glm.hpp>

namespace dae
{
	class GameObject;
	class TransformComponent;

	class MoveCommand final : public Command
	{
	public:
		MoveCommand(GameObject* gameObject, const glm::vec2& direction, float speed);

		void Execute() override;

	private:
		GameObject* m_pGameObject{};
		glm::vec2 m_Direction{};
		float m_Speed{};
	};
}