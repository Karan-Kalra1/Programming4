#pragma once
#include "Command.h"

namespace digger
{
	class GameManagerComponent;

	class ShootFireballCommand final : public dae::Command
	{
	public:
		explicit ShootFireballCommand(GameManagerComponent* manager)
			: m_Manager(manager)
		{
		}

		void Execute() override;

	private:
		GameManagerComponent* m_Manager{};
	};
}