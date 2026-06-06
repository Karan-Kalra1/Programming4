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

	class FireballManager final
	{
	public:
		explicit FireballManager(dae::Scene* scene);

		float GetCooldown(const GameManagerComponent& game) const;

		void Shoot(GameManagerComponent& game, int playerIndex);
		void CheckHit(GameManagerComponent& game, dae::GameObject* fireball);

		void Remove(GameManagerComponent& game, dae::GameObject* fireball);
		void RemoveAll(GameManagerComponent& game);

		bool IsBlockingTileForFireball(
			const GameManagerComponent& game,
			const glm::ivec2& pos) const;

	private:
		glm::ivec2 WorldToGrid(
			const GameManagerComponent& game,
			const glm::vec2& worldPos) const;

		bool HasAnyFireballs() const;

		dae::Scene* m_Scene{};
		std::vector<dae::GameObject*> m_Fireballs{};
	};
}