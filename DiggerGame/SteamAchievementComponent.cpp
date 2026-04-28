#include "SteamAchievementComponent.h"
#include "GameActorComponent.h"
#include "GameObject.h"
#if USE_STEAMWORKS
#include <steam_api.h>
#endif


dae::SteamAchievementComponent::SteamAchievementComponent(GameObject* owner, GameActorComponent* actor)
	: Component(owner)
	, m_pActor(actor)
{
	if (m_pActor)
	{
		m_pActor->GetSubject().AddObserver(this);
	}
}


void dae::SteamAchievementComponent::OnNotify(const Event& event)
{

	if (!m_pActor || event.actor != m_pActor->GetOwner())
		return;


	if (!m_HasCheckedInitialState)
	{
		m_HasCheckedInitialState = true;
#if USE_STEAMWORKS
		if (SteamUserStats())
		{
			bool achieved = false;
			if (SteamUserStats()->GetAchievement("ACH_WIN_ONE_GAME", &achieved))
			{
				m_AlreadyUnlocked = achieved;
			}
		}
#endif

	}

	if (m_AlreadyUnlocked)
		return;

	if (event.type == EventType::ScoreChanged && m_pActor->GetScore() >= 500)
	{
#if USE_STEAMWORKS

		if (SteamUserStats())
		{
			bool achieved = false;
			if (SteamUserStats()->GetAchievement("ACH_WIN_ONE_GAME", &achieved) && !achieved)
			{
				SteamUserStats()->SetAchievement("ACH_WIN_ONE_GAME");
				SteamUserStats()->StoreStats();
				m_AlreadyUnlocked = true;
			}
			else if(achieved)
			{
				m_AlreadyUnlocked = true;
			}
		}
#endif
	}

}