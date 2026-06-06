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
	class EnemyComponent;

	class EnemyManager final
	{
	public:
		explicit EnemyManager(dae::Scene* scene);

		void BeginStage(GameManagerComponent& game);
		bool Update(GameManagerComponent& game, float deltaTime);

		void Clear();

		void SetSpawn(const glm::ivec2& spawn, bool hasSpawn);

		void SpawnEnemy(GameManagerComponent& game);
		void RegisterEnemy(EnemyComponent* enemy);
		void KillEnemy(GameManagerComponent& game, EnemyComponent* enemy);
		void ResetEnemiesAfterPlayerDeath(GameManagerComponent& game);

		void CheckEnemyCrossings();

		bool HasEnemyAt(const glm::ivec2& pos) const;

		EnemyComponent* GetEnemyAtWorldPosition(
			const glm::vec2& worldPos,
			float radius) const;

		std::vector<EnemyComponent*> GetEnemiesAtWorldPosition(
			const glm::vec2& worldPos,
			float radius) const;

		const std::vector<EnemyComponent*>& GetEnemies() const
		{
			return m_Enemies;
		}

		int GetEnemiesAlive() const { return m_EnemiesAlive; }
		int GetEnemiesRemainingToSpawn() const { return m_EnemiesRemainingToSpawn; }

	private:
		float GetSpawnInterval(const GameManagerComponent& game) const;
		int GetEnemyCountForStage(const GameManagerComponent& game) const;

		dae::Scene* m_Scene{};

		std::vector<EnemyComponent*> m_Enemies{};

		glm::ivec2 m_EnemySpawn{};
		bool m_HasEnemySpawn{};

		int m_TotalEnemiesThisStage{};
		int m_EnemiesRemainingToSpawn{};
		int m_EnemiesAlive{};

		float m_EnemySpawnTimer{};
	};
}