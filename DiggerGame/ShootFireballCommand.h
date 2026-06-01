#pragma once
#include "Command.h"

namespace digger
{
	class GameManagerComponent;

	class ShootFireballCommand final : public dae::Command
	{
	public:
		ShootFireballCommand(GameManagerComponent* manager, int playerIndex)
			: m_Manager(manager)
			, m_PlayerIndex(playerIndex)
		{
		}

		void Execute() override;

	private:
		GameManagerComponent* m_Manager{};
		int m_PlayerIndex{};
	};
}