#pragma once
#include "Command.h"

namespace digger
{
	class GameManagerComponent;

	class MenuNavigateCommand final : public dae::Command
	{
	public:
		MenuNavigateCommand(GameManagerComponent* manager, int delta)
			: m_Manager(manager)
			, m_Delta(delta)
		{
		}

		void Execute() override;

	private:
		GameManagerComponent* m_Manager{};
		int m_Delta{};
	};
}