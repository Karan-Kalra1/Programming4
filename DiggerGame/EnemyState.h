#pragma once

namespace digger
{
	class EnemyComponent;

	class EnemyState
	{
	public:
		virtual ~EnemyState() = default;

		virtual void OnEnter(EnemyComponent&) {}
		virtual void Update(EnemyComponent& enemy) = 0;
		virtual void OnExit(EnemyComponent&) {}
	};
}