#pragma once
#include "Command.h"

namespace digger
{
	class GameManagerComponent;

	class SkipLevelCommand final : public dae::Command
	{
	public:
		explicit SkipLevelCommand(GameManagerComponent* manager)
			: m_Manager(manager)
		{
		}

		void Execute() override;

	private:
		GameManagerComponent* m_Manager{};
	};
}