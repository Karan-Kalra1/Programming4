#pragma once
#include "Command.h"

namespace dae
{
	class GameActorComponent;

	class AddScoreCommand final : public Command
	{
	public:
		AddScoreCommand(GameActorComponent* actor, int amount)
			: m_pActor(actor)
			, m_Amount(amount) {
		}

		void Execute() override;

	private:
		GameActorComponent* m_pActor{};
		int m_Amount{};
	};
}