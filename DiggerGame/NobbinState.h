#pragma once
#include "EnemyState.h"

namespace digger
{
	class NobbinState final : public EnemyState
	{
	public:
		void OnEnter(EnemyComponent& enemy) override;
		void Update(EnemyComponent& enemy) override;

	private:
		float m_MoveTimer{};
		float m_MoveDelay{ 0.35f };
	};
}