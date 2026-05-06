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


		dae::GameObject* GetPlayer() const { return m_Player; }

	private:
		void ClearLevel();
		void Update() override;
		void SpawnLevel(const LevelData& data);
		std::string MakeTileKey(const glm::ivec2& pos) const;

		dae::Scene* m_Scene{};
		GameMode m_Mode{ GameMode::SinglePlayer };
		bool m_ShouldLoadNextLevel{};

		int m_CurrentLevel{};
		int m_TileSize{ 32 };
		int m_Score{};
		int m_Lives{ 4 };

		glm::ivec2 m_PlayerSpawn{};
		
		void RegisterEnemy(EnemyComponent* enemy);
		void CheckEnemyCrossings();

		LevelData m_LevelData{};

		std::vector<dae::GameObject*> m_LevelObjects;
		dae::GameObject* m_Player{};
		std::vector<dae::GameObject*> m_Diamonds{};
		std::vector<EnemyComponent*> m_Enemies{};
		std::vector<std::unique_ptr<dae::GameObject>> m_DirtVisuals{};
		std::unordered_map<std::string, dae::GameObject*> m_DirtTiles{};


		
	};
}