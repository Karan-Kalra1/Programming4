#pragma once
#include "Command.h"

namespace dae
{
	class GameObject;

	class LoseLifeCommand final : public Command
	{
	public:
		LoseLifeCommand(GameObject* actor, int amount = 1)
			: m_pActor(actor)
			, m_Amount(amount) {
		}

		void Execute() override;

	private:
		GameObject* m_pActor{};
		int m_Amount{};
	};
}