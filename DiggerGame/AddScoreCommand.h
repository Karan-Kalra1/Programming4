#pragma once
#include "Command.h"

namespace dae
{
	class GameObject;

	class AddScoreCommand final : public Command
	{
	public:
		AddScoreCommand(GameObject* actor, int amount)
			: m_pActor(actor)
			, m_Amount(amount) {
		}

		void Execute() override;

	private:
		GameObject* m_pActor{};
		int m_Amount{};
	};
}