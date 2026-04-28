#pragma once
#include <string>
#include "Component.h"
#include "Observer.h"

namespace dae
{
	class GameActorComponent;

	class LivesDisplayComponent final : public Component, public Observer
	{
	public:
		LivesDisplayComponent(GameObject* owner, GameActorComponent* actor, const std::string& prefix = "Lives: ");
		

		void OnNotify(const Event& event) override;

	private:
		void RefreshText(int lives);

		GameActorComponent* m_pActor{};
		std::string m_Prefix{};
	};
}