#pragma once
#include "Command.h"

namespace dae
{
	class GameActorComponent;

	class LoseLifeCommand final : public Command
	{
	public:
		explicit LoseLifeCommand(GameActorComponent* actor)
			: m_pActor(actor) {
		}

		void Execute() override;

	private:
		GameActorComponent* m_pActor{};
	};
}