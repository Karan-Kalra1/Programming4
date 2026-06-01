#pragma once
#include "Command.h"

namespace digger
{
	class GameManagerComponent;

	class HighScoreConfirmCommand final : public dae::Command
	{
	public:
		explicit HighScoreConfirmCommand(GameManagerComponent* manager)
			: m_Manager(manager)
		{
		}

		void Execute() override;

	private:
		GameManagerComponent* m_Manager{};
	};
}