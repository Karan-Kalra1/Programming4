#pragma once
#include "EnemyState.h"

namespace digger
{
	class HobbinState final : public EnemyState
	{
	public:
		void OnEnter(EnemyComponent& enemy) override;
		void Update(EnemyComponent& enemy) override;
		void OnExit(EnemyComponent& enemy) override;

	private:
		float m_MoveTimer{};
		float m_StateTimer{};
		float m_MoveDelay{ 0.28f };
		float m_HobbinDuration{ 6.0f };
	};
}