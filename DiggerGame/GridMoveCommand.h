#pragma once
#include "Command.h"
#include <glm/glm.hpp>

namespace digger
{
	class GameManagerComponent;

	class GridMoveCommand final : public dae::Command
	{
	public:
		GridMoveCommand(GameManagerComponent* manager, const glm::ivec2& direction);

		void Execute() override;

	private:
		GameManagerComponent* m_Manager{};
		glm::ivec2 m_Direction{};
	};
}