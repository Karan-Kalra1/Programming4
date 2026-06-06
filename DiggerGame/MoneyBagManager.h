#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace dae
{
	class Scene;
	class GameObject;
}

namespace digger
{
	class GameManagerComponent;
	class MoneyBagComponent;

	class MoneyBagManager final
	{
	public:
		explicit MoneyBagManager(dae::Scene* scene);

		void Register(MoneyBagComponent* bag);
		void Unregister(MoneyBagComponent* bag);
		void Clear();

		bool HasMoneyBagAt(const glm::ivec2& pos) const;

		bool CanMoveTo(
			const GameManagerComponent& game,
			const glm::ivec2& pos) const;

		bool TryPushAt(
			GameManagerComponent& game,
			const glm::ivec2& bagPos,
			const glm::ivec2& direction);

		void CollectGold(
			GameManagerComponent& game,
			dae::GameObject* goldObject);

	private:
		dae::Scene* m_Scene{};
		std::vector<MoneyBagComponent*> m_MoneyBags{};
	};
}