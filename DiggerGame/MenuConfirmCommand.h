#pragma once
#include "Command.h"

namespace digger
{
	class GameManagerComponent;

	class MenuConfirmCommand final : public dae::Command
	{
	public:
		explicit MenuConfirmCommand(GameManagerComponent* manager)
			: m_Manager(manager)
		{
		}

		void Execute() override;

	private:
		GameManagerComponent* m_Manager{};
	};
}