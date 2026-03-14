#pragma once
#include "Observer.h"

namespace dae
{
	class GameActorComponent;

	class SteamAchievementObserver final : public Observer
	{
	public:
		explicit SteamAchievementObserver(GameActorComponent* actor);

		void StartListening();
		void StopListening();

		void OnNotify(const Event& event) override;

	private:
		GameActorComponent* m_pActor{};
		bool m_Unlocked{};
	};
}