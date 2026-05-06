#pragma once
#include "Command.h"

namespace digger
{
	class GameManagerComponent;

	class ToggleMuteCommand final : public dae::Command
	{
	public:
		explicit ToggleMuteCommand(GameManagerComponent* manager)
			: m_Manager(manager)
		{
		}

		void Execute() override;

	private:
		GameManagerComponent* m_Manager{};
	};
}