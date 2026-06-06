#pragma once

#include "LevelData.h"
#include "LevelLoader.h"

#include <string>
#include <unordered_map>
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

	class LevelSystem final
	{
	public:
		explicit LevelSystem(dae::Scene* scene);

		//Level Setup
		void SetLevelData(const LevelData& data);
		void SpawnStaticObjects(GameManagerComponent& game);
		void Clear();

		//Level Information
		const LevelData& GetLevelData() const { return m_LevelData; }

		int GetWidth() const { return m_LevelData.width; }
		int GetHeight() const { return m_LevelData.height; }

		int GetTileSize() const { return m_TileSize; }
		glm::vec2 GetMapOffset() const { return m_MapOffset; }

		//Tile Queries
		bool CanPlayerMoveTo(const glm::ivec2& pos) const;
		bool CanEnemyMoveTo(const glm::ivec2& pos, bool canDig, bool hasDiamond) const;

		bool IsDirt(const glm::ivec2& pos) const;
		bool IsSolidWall(const glm::ivec2& pos) const;
		bool HasDirtBelow(const glm::ivec2& pos) const;
		bool IsSolidForMoneyBag(const glm::ivec2& pos, bool hasMoneyBag) const;

		//Diamonds
		bool HasDiamondAt(const glm::ivec2& pos) const;
		void CollectDiamond(GameManagerComponent& game, dae::GameObject* diamond);

		//Digging
		void DigTile(const glm::ivec2& pos);
		void DigAtWorldPosition(
			GameManagerComponent& game,
			const glm::vec2& worldPos,
			const glm::ivec2& direction);

		void DigAtPoint(GameManagerComponent& game, const glm::vec2& point);

	private:
		//Level Helpers
		std::string MakeTileKey(const glm::ivec2& pos) const;
		glm::ivec2 WorldToGrid(const glm::vec2& worldPos) const;

		//Static Object Spawning
		void SpawnDirtTiles(GameManagerComponent& game);
		void SpawnDiamonds(GameManagerComponent& game);
		void SpawnMoneyBags(GameManagerComponent& game);

		dae::Scene* m_Scene{};

		//Level Data
		LevelData m_LevelData{};

		std::unordered_map<std::string, DirtTile> m_DirtTiles{};
		std::vector<dae::GameObject*> m_Diamonds{};

		int m_TileSize{ 64 };
		glm::vec2 m_MapOffset{ -32.f, -32.f };

		//Digging Settings
		glm::vec2 m_PlayerCenterOffset{ 32.f, 32.f };
		float m_DigRadius{ 20.f };
	};
}