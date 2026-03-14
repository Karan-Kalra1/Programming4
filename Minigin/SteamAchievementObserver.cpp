#include "SteamAchievementObserver.h"
#include "GameActorComponent.h"

#if USE_STEAMWORKS
#include <steam_api.h>
#endif

dae::SteamAchievementObserver::SteamAchievementObserver(GameActorComponent* actor)
	: m_pActor(actor)
{
}

void dae::SteamAchievementObserver::StartListening()
{
	if (m_pActor)
		m_pActor->GetSubject().AddObserver(this);
}

void dae::SteamAchievementObserver::StopListening()
{
	if (m_pActor)
		m_pActor->GetSubject().RemoveObserver(this);
}

void dae::SteamAchievementObserver::OnNotify(const Event& event)
{
	if (!m_pActor || event.actor != m_pActor->GetOwner() || m_Unlocked)
		return;

	if (event.type == EventType::ScoreChanged && m_pActor->GetScore() >= 500)
	{
#if USE_STEAMWORKS
		if (SteamUserStats())
		{
			SteamUserStats()->SetAchievement("ACH_WIN_ONE_GAME");
			SteamUserStats()->StoreStats();
			m_Unlocked = true;
		}
#endif

	}
}