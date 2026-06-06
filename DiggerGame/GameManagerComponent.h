#pragma once

#include "Component.h"
#include "GameMode.h"
#include "LevelData.h"
#include "HighScoreManager.h"
#include "LevelLoader.h"


#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

namespace dae
{
	class Scene;
	class GameObject;
	class GameActorComponent;
	class TextComponent;
}

namespace digger
{

	enum class GameScreenState
	{
		StartMenu,
		ModeSelect,
		Playing,
		EnteringHighScore,
		ShowingHighScores
	};

	enum class PlayerRole
	{
		Digger,
		VersusEnemy
	};

	struct PlayerRuntime
	{
		dae::GameObject* object{};
		dae::GameActorComponent* actor{};

		glm::ivec2 spawn{};

		int lives{ 4 };
		bool alive{ true };
		bool dying{ false };

		float damageCooldown{};
		float fireballCooldown{};
		bool fireballActive{};

		PlayerRole role{ PlayerRole::Digger };
	};


	class MoneyBagComponent;
	class EnemyComponent;
	class HudController;
	class MenuController;
	class HighScoreScreenController;
	class DeathSequenceController;
	class MoneyBagManager;
	class FireballManager;
	class EnemyManager;
	//class PlayerManager;
	//class LevelSystem;
	

	class GameManagerComponent final : public dae::Component
	{
	public:
		GameManagerComponent(dae::GameObject* owner, dae::Scene* scene);
		~GameManagerComponent() override;
		
		//Hud
		void CreateHUD();
		void UpdateHUD();
		void GameOver();

		//HighScore
		void ChangeHighScoreLetter(int delta);
		void MoveHighScoreCursor(int delta);
		void ConfirmHighScoreLetter();
		void ShowHighScoreEntryScreen();
		void SubmitHighScore();
		void ShowHighScoreList();
		void ConfirmHighScoreScreen();


		//Start Screen
		void NavigateMenu(int delta);
		void ConfirmMenuSelection();
		void ShowStartMenu();

		//Main Game Functions
		void StartGame(GameMode mode);
		void LoadLevel(int index);
		void SkipLevel();
		void ToggleMute();
		bool IsGameplayFrozen() const;
		int GetScore() const { return m_Score; }

		//Diamonds
		void CollectDiamond(dae::GameObject* diamond);


		//Level Information
		bool CanEnemyMoveTo(const glm::ivec2& pos, bool canDig) const;	
		bool CanPlayerMoveTo(const glm::ivec2& pos) const;
		glm::vec2 GetMapOffset()const { return m_MapOffset; }
		int GetTileSize()const { return m_TileSize; }
		bool IsSolidWall(const glm::ivec2& pos) const;
		bool HasDirtBelow(const glm::ivec2& pos) const;	
		GameMode GetGameMode() const { return m_Mode; }
		int GetCurrentLevel() const { return m_CurrentLevel; }
		int GetLevelWidth() const { return m_LevelData.width; }
		int GetLevelHeight() const { return m_LevelData.height; }


		//Digging
		void DigTile(const glm::ivec2& pos);
		bool IsDirt(const glm::ivec2& pos) const;
		void DigAtWorldPosition(const glm::vec2& worldPos, const glm::ivec2& direction);
		void DigAtPoint(const glm::vec2& point);

		//Player Data/Location
		bool IsPlayerAtGridPosition(const glm::ivec2& pos) const;
		dae::GameObject* GetPlayer(int index = 0) const;
		const PlayerRuntime* GetPlayerRuntime(int playerIndex) const;
		glm::ivec2 GetPlayerGridPosition(int index = 0) const;
		glm::vec2 GetPlayerWorldPosition(int index = 0) const;
		glm::ivec2 GetClosestAlivePlayerGridPosition(const glm::ivec2& fromGrid) const;	
		int GetPlayerIndexAtWorldPosition(const glm::vec2& worldPos, float radius) const;
		int GetPlayerIndexAtGridPosition(const glm::ivec2& pos) const;
		int GetDiggerPlayerIndexAtWorldPosition(
			const glm::vec2& worldPos,
			float radius) const;
		int GetPlayerIndexAtWorldPosition(
			const glm::vec2& worldPos,
			float radius,
			std::optional<PlayerRole> requiredRole) const;
		const std::vector<PlayerRuntime>& GetPlayers() const
		{
			return m_Players;
		}



		//Player Collision/Life status
		void CheckVersusCollision();
		void DamagePlayer(int playerIndex = 0);
		bool AreAllPlayersDead() const;
		float GetCollisionRadius() const { return m_TileSize * 0.7f; }
		bool IsPlayerAlive(int playerIndex) const;
		bool IsPlayerControllable(int playerIndex) const;

		//Player Spawn/Count
		std::vector<PlayerRuntime> m_Players{};
		void SpawnDiggerPlayer(
			int playerIndex,
			const glm::ivec2& spawn,
			const std::string& textureFile,
			PlayerRole role = PlayerRole::Digger);
		void RemovePlayerObservers();
		int GetPlayerCount() const
		{
			return static_cast<int>(m_Players.size());
		}

		
		//Enemy
		void KillEnemy(EnemyComponent* enemy);
		bool HasEnemyAt(const glm::ivec2& pos) const;
		const std::vector<EnemyComponent*>& GetEnemies() const;
		EnemyComponent* GetEnemyAtWorldPosition(const glm::vec2& worldPos, float radius) const;
		std::vector<EnemyComponent*> GetEnemiesAtWorldPosition(
			const glm::vec2& worldPos,
			float radius) const;

		//Fireball
		float GetFireballCooldown() const;
		void ShootFireball(int playerIndex = 0);
		void CheckFireballHit(dae::GameObject* fireball);
		

		bool CanPlayerShootFireball(int playerIndex) const;

		void StartPlayerFireballCooldown(int playerIndex, float cooldown);
		void SetPlayerFireballActive(int playerIndex, bool active);
		void ClearAllPlayerFireballStates();
		

		//Moneybag
		bool CanMoneyBagMoveTo(const glm::ivec2& pos) const;
		bool TryPushMoneyBagAt(const glm::ivec2& bagPos, const glm::ivec2& direction);
		bool HasMoneyBagAt(const glm::ivec2& pos) const;
		void RegisterMoneyBag(MoneyBagComponent* bag);
		void UnregisterMoneyBag(MoneyBagComponent* bag);
		void CollectGold(dae::GameObject* gold);
		bool IsSolidForMoneyBag(const glm::ivec2& pos) const;


		
		void AddLevelObject(dae::GameObject* object);
		void RemoveLevelObject(dae::GameObject* object);
		
		void AddScore(int amount);
		

	private:


		std::unique_ptr<HudController> m_HudController{};
		std::unique_ptr<MenuController> m_MenuController{};
		std::unique_ptr<HighScoreScreenController> m_HighScoreScreenController{};
		std::unique_ptr<DeathSequenceController> m_DeathSequenceController{};
		std::unique_ptr<MoneyBagManager> m_MoneyBagManager{};
		std::unique_ptr<FireballManager> m_FireballManager{};
		std::unique_ptr<EnemyManager> m_EnemyManager{};
		//std::unique_ptr<PlayerManager> m_PlayerManager{};
		//std::unique_ptr<LevelSystem> m_LevelSystem{};

		//Main Game
		void ClearLevel();
		void Update() override;
		

		//Level Info
		void SpawnLevel(const LevelData& data);
		std::string MakeTileKey(const glm::ivec2& pos) const;
		
		//Enemy
		void SpawnEnemy();
		void ResetEnemiesAfterPlayerDeath();
		void RegisterEnemy(EnemyComponent* enemy);
		void CheckEnemyCrossings();

		//Fireball
		bool IsBlockingTileForFireball(const glm::ivec2& pos) const;
		void RemoveFireball(dae::GameObject* fireball);
		void RemoveAllFireballs();

		//Death Logic
		void BeginPlayerDeathSequence();
		void FinishPlayerDeathSequence();

		
		//Start Screen
		void ShowModeSelectMenu();
		void ClearScreenUI();
		void QuitGame();

		//HUD
		void ClearHUD();
		

		//Main Game
		dae::Scene* m_Scene{};
		GameMode m_Mode{ GameMode::SinglePlayer };
		bool m_ShouldLoadNextLevel{};


		//Level Info
		std::vector<dae::GameObject*> m_LevelObjects;
		//std::vector<std::unique_ptr<dae::GameObject>> m_DirtVisuals{};
		std::unordered_map<std::string, DirtTile> m_DirtTiles{};
		std::vector<dae::GameObject*> m_Diamonds{};
		int m_CurrentLevel{};
		int m_TileSize{ 64 };
		LevelData m_LevelData{};
		glm::vec2 m_MapOffset{ -32.f, -32.f };


		


		//HUD	
		GameScreenState m_ScreenState{ GameScreenState::Playing };
		int m_Score{};
		
		//Death Position
		glm::vec2 m_LastDeathWorldPosition{};


		//Player
		
		int m_DeathPlayerIndex{ -1 };
		std::array<int, 2> m_PlayerLives{ 4, 4 };
		std::array<bool, 2> m_PlayerAlive{ true, true };
		float m_DamageCooldownDuration{ 1.0f };
		glm::vec2 m_PlayerCenterOffset{ 32.f, 32.f };
		float m_DigRadius{ 20.f };


		
				
	};
}