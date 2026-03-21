#pragma once
#include <string>
#include "Component.h"
#include "Observer.h"

namespace dae
{
	class GameActorComponent;

	class ScoreDisplayComponent final : public Component, public Observer
	{
	public:
		ScoreDisplayComponent(GameObject* owner, GameActorComponent* actor, const std::string& prefix = "Score: ");
		
		void OnNotify(const Event& event) override;

	private:
		void RefreshText(int score);

		GameActorComponent* m_pActor{};
		std::string m_Prefix{};
	};
}