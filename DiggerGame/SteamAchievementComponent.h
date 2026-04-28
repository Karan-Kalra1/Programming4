#pragma once
#include "Component.h"
#include "Observer.h"

namespace dae
{
	class GameActorComponent;

	class SteamAchievementComponent final : public Component, public Observer
	{
	public:
		SteamAchievementComponent(GameObject* owner, GameActorComponent* actor);
	

		void OnNotify(const Event& event) override;

	private:
		GameActorComponent* m_pActor{};
		bool m_HasCheckedInitialState{};
		bool m_AlreadyUnlocked{};
	};
}