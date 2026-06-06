#pragma once

#include "Component.h"
#include "GameMode.h"
#include "LevelData.h"
#include "PlayerTypes.h"
#include "LevelLoader.h"

#include <memory>
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
	enum class GameScreenState
	{
		StartMenu,
		ModeSelect,
		Playing,
		EnteringHighScore,
		ShowingHighScores
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
	class PlayerManager;
	class LevelSystem;

	class GameManagerComponent final : public dae::Component
	{
	public:
		GameManagerComponent(dae::GameObject* owner, dae::Scene* scene);
		~GameManagerComponent() override;

		//Hud
		void CreateHUD();
		void UpdateHUD();
		void ClearHUD();
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
		

		GameMode GetGameMode() const { return m_Mode; }
		int GetCurrentLevel() const { return m_CurrentLevel; }

		int GetScore() const { return m_Score; }
		void AddScore(int amount);

		//Diamonds
		void CollectDiamond(dae::GameObject* diamond);

		//Level Information
		bool CanEnemyMoveTo(const glm::ivec2& pos, bool canDig) const;
		bool CanPlayerMoveTo(const glm::ivec2& pos) const;

		bool IsSolidWall(const glm::ivec2& pos) const;
		bool HasDirtBelow(const glm::ivec2& pos) const;
		bool HasDiamondAt(const glm::ivec2& pos) const;

		glm::vec2 GetMapOffset() const;
		int GetTileSize() const;

		int GetLevelWidth() const;
		int GetLevelHeight() const;

		//Digging
		void DigTile(const glm::ivec2& pos);
		bool IsDirt(const glm::ivec2& pos) const;
		void DigAtWorldPosition(const glm::vec2& worldPos, const glm::ivec2& direction);
		void DigAtPoint(const glm::vec2& point);

		//Player Data/Location
		const std::vector<PlayerRuntime>& GetPlayers() const;
		const PlayerRuntime* GetPlayerRuntime(int playerIndex) const;

		dae::GameObject* GetPlayer(int index = 0) const;

		glm::ivec2 GetPlayerGridPosition(int index = 0) const;
		glm::vec2 GetPlayerWorldPosition(int index = 0) const;
		glm::ivec2 GetClosestAlivePlayerGridPosition(const glm::ivec2& fromGrid) const;

		int GetPlayerIndexAtWorldPosition(const glm::vec2& worldPos, float radius) const;
		int GetDiggerPlayerIndexAtWorldPosition(const glm::vec2& worldPos, float radius) const;
		int GetPlayerIndexAtGridPosition(const glm::ivec2& pos) const;

		bool IsPlayerAtGridPosition(const glm::ivec2& pos) const;
		int GetPlayerCount() const;

		//Player Collision/Life status
		void CheckVersusCollision();
		void DamagePlayer(int playerIndex = 0);

		bool AreAllPlayersDead() const;
		bool IsPlayerAlive(int playerIndex) const;
		bool IsPlayerControllable(int playerIndex) const;

		float GetCollisionRadius() const;
		float GetDamageCooldownDuration() const { return m_DamageCooldownDuration; }

		//Enemy
		void KillEnemy(EnemyComponent* enemy);

		bool HasEnemyAt(const glm::ivec2& pos) const;

		void ResetEnemiesAfterPlayerDeath();

		const std::vector<EnemyComponent*>& GetEnemies() const;

		EnemyComponent* GetEnemyAtWorldPosition(
			const glm::vec2& worldPos,
			float radius) const;

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
		void RemoveAllFireballs();

		//Moneybag
		bool CanMoneyBagMoveTo(const glm::ivec2& pos) const;
		bool TryPushMoneyBagAt(const glm::ivec2& bagPos, const glm::ivec2& direction);
		bool HasMoneyBagAt(const glm::ivec2& pos) const;

		void RegisterMoneyBag(MoneyBagComponent* bag);
		void UnregisterMoneyBag(MoneyBagComponent* bag);
		void CollectGold(dae::GameObject* gold);

		bool IsSolidForMoneyBag(const glm::ivec2& pos) const;

		//Level Object Helpers
		void AddLevelObject(dae::GameObject* object);
		void RemoveLevelObject(dae::GameObject* object);

		//Death Logic
		void StartDeathSequenceAt(const glm::vec2& position);

	private:
		//Controllers
		std::unique_ptr<HudController> m_HudController{};
		std::unique_ptr<MenuController> m_MenuController{};
		std::unique_ptr<HighScoreScreenController> m_HighScoreScreenController{};
		std::unique_ptr<DeathSequenceController> m_DeathSequenceController{};

		//Managers
		std::unique_ptr<MoneyBagManager> m_MoneyBagManager{};
		std::unique_ptr<FireballManager> m_FireballManager{};
		std::unique_ptr<EnemyManager> m_EnemyManager{};
		std::unique_ptr<PlayerManager> m_PlayerManager{};
		std::unique_ptr<LevelSystem> m_LevelSystem{};

		//Main Game
		void Update() override;
		void ClearLevel();
		

		//Level Info
		bool m_IsLoadingLevel{};
		float m_LevelLoadFreezeTimer{};
		float m_LevelLoadFreezeDuration{ 0.05f };
		std::vector<dae::GameObject*> m_LevelObjects{};

		//Enemy
		void SpawnEnemy();
		void RegisterEnemy(EnemyComponent* enemy);
		void CheckEnemyCrossings();

		//Fireball
		bool IsBlockingTileForFireball(const glm::ivec2& pos) const;
		void RemoveFireball(dae::GameObject* fireball);
		

		//Death Logic
		void FinishPlayerDeathSequence();

		//Start Screen
		void ShowModeSelectMenu();
		void ClearScreenUI();
		void QuitGame();

		//Main Game Data
		dae::Scene* m_Scene{};
		GameMode m_Mode{ GameMode::SinglePlayer };
		GameScreenState m_ScreenState{ GameScreenState::Playing };


		bool m_ShouldLoadNextLevel{};

		//Level Data
		int m_CurrentLevel{};
	

		//Score
		int m_Score{};

		//Player/Digging Settings
		float m_DamageCooldownDuration{ 1.0f };
	};
}