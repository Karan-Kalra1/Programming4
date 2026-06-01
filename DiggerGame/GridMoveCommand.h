#pragma once
#include "Command.h"
#include <glm/glm.hpp>

namespace digger
{
	class GameManagerComponent;

	class GridMoveCommand final : public dae::Command
	{
	public:
		GridMoveCommand(GameManagerComponent* manager, int playerIndex, const glm::ivec2& direction, bool pressed)
			: m_Manager(manager)
			, m_PlayerIndex(playerIndex)
			, m_Direction(direction)
			, m_Pressed(pressed)
		{
		}

		void Execute() override;

	private:
		GameManagerComponent* m_Manager{};
		glm::ivec2 m_Direction{};
		bool m_Pressed{};
		int m_PlayerIndex{};
	};
}