#pragma once
#include "EnemyState.h"

namespace digger
{
	class EnemyDeadState final : public EnemyState
	{
	public:
		void OnEnter(EnemyComponent& enemy) override;
		void Update(EnemyComponent&) override {}
	};
}