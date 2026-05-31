#pragma once
#include <vector>
#include <filesystem>
#include "Component.h"
#include "GameMode.h"
#include "LevelData.h"
#include "EnemyComponent.h"
#include "EnemyComponent.h"
#include <unordered_map>
#include <string>
#include "LevelLoader.h"
#include "GameActorComponent.h"

namespace dae
{
	class Scene;
	class GameObject;
}

namespace digger
{
	class GameManagerComponent final : public dae::Component
	{
	public:
		GameManagerComponent(dae::GameObject* owner, dae::Scene* scene);

		void StartGame(GameMode mode);
		void LoadLevel(int index);
		void SkipLevel();
		void ToggleMute();

		void CollectDiamond(dae::GameObject* diamond);
		glm::ivec2 GetPlayerGridPosition() const;	
		bool CanEnemyMoveTo(const glm::ivec2& pos, bool canDig) const;
		void DigTile(const glm::ivec2& pos);
		bool IsDirt(const glm::ivec2& pos) const;
		void DamagePlayer();
		bool CanPlayerMoveTo(const glm::ivec2& pos) const;
		void DigAtWorldPosition(const glm::vec2& worldPos, const glm::ivec2& direction);

		dae::GameObject* GetPlayer() const { return m_Player; }
		glm::vec2 GetPlayerWorldPosition() const;
		float GetCollisionRadius() const { return m_TileSize * 0.7f; }
		glm::vec2 GetMapOffset(){ return m_MapOffset; }
		int GetTileSize() { return m_TileSize; }
		void KillEnemy(EnemyComponent* enemy);
		float GetFireballCooldown() const;
		void ShootFireball();
		void CheckFireballHit(dae::GameObject* fireball);
		glm::ivec2 GetPlayerFacingDirection() const;

		void DigAtPoint(const glm::vec2& point);
	private:
		void ClearLevel();
		void Update() override;
		void SpawnLevel(const LevelData& data);
		std::string MakeTileKey(const glm::ivec2& pos) const;
		

		dae::Scene* m_Scene{};
		GameMode m_Mode{ GameMode::SinglePlayer };
		bool m_ShouldLoadNextLevel{};

		int m_CurrentLevel{};
		int m_TileSize{ 64 };
		int m_Score{};
		int m_Lives{ 4 };

		glm::ivec2 m_PlayerSpawn{};
		
		void RegisterEnemy(EnemyComponent* enemy);
		void CheckEnemyCrossings();

		LevelData m_LevelData{};

		glm::vec2 m_PlayerCenterOffset{ 32.f, 32.f };
		float m_DigRadius{ 20.f };

		std::vector<dae::GameObject*> m_LevelObjects;
		dae::GameObject* m_Player{};
		dae::GameActorComponent* m_PlayerActor{};
		std::vector<dae::GameObject*> m_Diamonds{};
		std::vector<EnemyComponent*> m_Enemies{};
		std::vector<std::unique_ptr<dae::GameObject>> m_DirtVisuals{};
		std::unordered_map<std::string, DirtTile> m_DirtTiles{};
		glm::vec2 m_MapOffset{ -32.f, -32.f };


		void SpawnEnemy();
		
		void ResetEnemiesAfterPlayerDeath();

		glm::ivec2 m_EnemySpawn{};
		int m_EnemiesRemainingToSpawn{};
		int m_EnemiesAlive{};
		int m_TotalEnemiesThisStage{};

		float m_EnemySpawnTimer{};
		float m_EnemySpawnInterval{ 2.0f };

		float m_FireballCooldownTimer{};
		std::vector<dae::GameObject*> m_Fireballs{};
		
	};
}