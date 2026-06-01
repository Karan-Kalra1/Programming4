#pragma once
#include <vector>
#include <filesystem>
#include "Component.h"
#include "GameMode.h"
#include "LevelData.h"
#include "EnemyComponent.h"
#include <unordered_map>
#include <string>
#include "LevelLoader.h"
#include "GameActorComponent.h"
#include "TextComponent.h"

#include "HighScoreManager.h"
#include <array>

namespace dae
{
	class Scene;
	class GameObject;
}

enum class GameScreenState
{
	StartMenu,
	ModeSelect,
	Playing,
	EnteringHighScore,
	ShowingHighScores
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
};

namespace digger
{
	class MoneyBagComponent;

	class GameManagerComponent final : public dae::Component
	{
	public:
		GameManagerComponent(dae::GameObject* owner, dae::Scene* scene);

		void ChangeHighScoreLetter(int delta);
		void MoveHighScoreCursor(int delta);
		void ConfirmHighScoreLetter();

		void CreateHUD();
		void UpdateHUD();
		void GameOver();
		void ShowHighScoreEntryScreen();
		void RefreshHighScoreEntryText();
		void SubmitHighScore();
		void ShowHighScoreList();
		//void ClearUI();
		void ConfirmHighScoreScreen();

		void StartGame(GameMode mode);
		void LoadLevel(int index);
		void SkipLevel();
		void ToggleMute();

		void CollectDiamond(dae::GameObject* diamond);
			
		bool CanEnemyMoveTo(const glm::ivec2& pos, bool canDig) const;
		void DigTile(const glm::ivec2& pos);
		bool IsDirt(const glm::ivec2& pos) const;
		//void DamagePlayer();
		bool CanPlayerMoveTo(const glm::ivec2& pos) const;
		void DigAtWorldPosition(const glm::vec2& worldPos, const glm::ivec2& direction);
		bool IsPlayerAtGridPosition(const glm::ivec2& pos) const;

		dae::GameObject* GetPlayer(int index = 0) const;
		glm::ivec2 GetPlayerGridPosition(int index = 0) const;
		glm::vec2 GetPlayerWorldPosition(int index = 0) const;


		glm::ivec2 GetClosestAlivePlayerGridPosition(const glm::ivec2& fromGrid) const;
		//int GetClosestAlivePlayerIndexToWorld(const glm::vec2& worldPos) const;
		int GetPlayerIndexAtWorldPosition(const glm::vec2& worldPos, float radius) const;
		int GetPlayerIndexAtGridPosition(const glm::ivec2& pos) const;

		void DamagePlayer(int playerIndex = 0);

		bool AreAllPlayersDead() const;

		void SpawnDiggerPlayer(
			int playerIndex,
			const glm::ivec2& spawn,
			const std::string& textureFile);

		void RemovePlayerObservers();

		int GetPlayerCount() const
		{
			return static_cast<int>(m_Players.size());
		}

		float GetCollisionRadius() const { return m_TileSize * 0.7f; }
		glm::vec2 GetMapOffset(){ return m_MapOffset; }
		int GetTileSize() { return m_TileSize; }
		void KillEnemy(EnemyComponent* enemy);
		float GetFireballCooldown() const;
		void ShootFireball(int playerIndex = 0);
		void CheckFireballHit(dae::GameObject* fireball);
		glm::ivec2 GetPlayerFacingDirection() const;
		void DigAtPoint(const glm::vec2& point);

		bool HasDirtBelow(const glm::ivec2& pos) const;
		bool CanMoneyBagMoveTo(const glm::ivec2& pos) const;
		bool TryPushMoneyBagAt(const glm::ivec2& bagPos, const glm::ivec2& direction);
		bool HasMoneyBagAt(const glm::ivec2& pos) const;

		void RegisterMoneyBag(MoneyBagComponent* bag);
		void UnregisterMoneyBag(MoneyBagComponent* bag);
		void CollectGold(dae::GameObject* gold);

		bool IsGameplayFrozen() const { return m_DeathSequenceActive || m_ScreenState != GameScreenState::Playing; }


		bool HasEnemyAt(const glm::ivec2& pos) const;

		EnemyComponent* GetEnemyAtWorldPosition(const glm::vec2& worldPos, float radius) const;

		void NavigateMenu(int delta);
		void ConfirmMenuSelection();
		void ShowStartMenu();
		bool IsPlayerAlive(int playerIndex) const;
		bool IsPlayerControllable(int playerIndex) const;

	private:


		void ClearLevel();
		void Update() override;
		void SpawnLevel(const LevelData& data);
		std::string MakeTileKey(const glm::ivec2& pos) const;

		void SpawnEnemy();

		void ResetEnemiesAfterPlayerDeath();
		bool IsBlockingTileForFireball(const glm::ivec2& pos) const;
		void RemoveFireball(dae::GameObject* fireball);
		void RemoveAllFireballs();

		void BeginPlayerDeathSequence();
		void FinishPlayerDeathSequence();

		void SpawnTombstone(const glm::vec2& position);
		void RemoveTombstone();

		void RegisterEnemy(EnemyComponent* enemy);
		void CheckEnemyCrossings();

		
		void ShowModeSelectMenu();

		void RefreshStartMenuText();
		void RefreshModeSelectText();
		void ClearScreenUI();
		void ClearHUD();

		void QuitGame();
		

		dae::TextComponent* m_StartMenuTitleText{};
		dae::TextComponent* m_StartMenuPlayText{};
		dae::TextComponent* m_StartMenuQuitText{};

		dae::TextComponent* m_ModeMenuTitleText{};
		dae::TextComponent* m_ModeSingleText{};
		dae::TextComponent* m_ModeCoopText{};
		dae::TextComponent* m_ModeVersusText{};

		int m_StartMenuIndex{};
		int m_ModeMenuIndex{};

		std::vector<dae::GameObject*> m_Fireballs{};
		std::vector<dae::GameObject*> m_LevelObjects;

		
		std::vector<PlayerRuntime> m_Players{};
		std::vector<std::array<dae::GameObject*, 4>> m_PlayerLifeIcons{};
		std::vector<dae::TextComponent*> m_PlayerLifeLabels{};
		int m_DeathPlayerIndex{ -1 };

		
		std::vector<dae::GameObject*> m_Diamonds{};
		std::vector<EnemyComponent*> m_Enemies{};
		std::vector<MoneyBagComponent*> m_MoneyBags{};
		std::vector<std::unique_ptr<dae::GameObject>> m_DirtVisuals{};
		std::unordered_map<std::string, DirtTile> m_DirtTiles{};

		GameScreenState m_ScreenState{ GameScreenState::Playing };

		HighScoreManager m_HighScores{ "Data/Highscores.txt" };

		std::vector<dae::GameObject*> m_HUDObjects{};
		std::vector<dae::GameObject*> m_ScreenUIObjects{};
		std::array<dae::GameObject*, 4> m_LifeIcons{};

		dae::TextComponent* m_ScoreText{};
		dae::TextComponent* m_HighScoreEntryText{};
		dae::TextComponent* m_HighScoreListText{};
		dae::TextComponent* m_HighScoreCursorText{};

		std::string m_CurrentInitials{ "AAA" };
		int m_InitialIndex{};
		
		dae::Scene* m_Scene{};
		GameMode m_Mode{ GameMode::SinglePlayer };
		bool m_ShouldLoadNextLevel{};

		
		float m_DamageCooldownDuration{ 1.0f };
		int m_CurrentLevel{};
		int m_TileSize{ 64 };
		int m_Score{};
		
			
		LevelData m_LevelData{};

		glm::vec2 m_PlayerCenterOffset{ 32.f, 32.f };
		float m_DigRadius{ 16.f };

		
		glm::vec2 m_MapOffset{ -32.f, -32.f };


		glm::ivec2 m_EnemySpawn{};
		int m_EnemiesRemainingToSpawn{};
		int m_EnemiesAlive{};
		int m_TotalEnemiesThisStage{};

		bool m_DeathSequenceActive{};
		float m_DeathSequenceTimer{};
		float m_DeathSequenceDuration{ 6.9f }; // Same length as Death music

		dae::GameObject* m_Tombstone{};
		glm::vec2 m_PlayerDeathWorldPosition{};

		float m_EnemySpawnTimer{};
		float m_EnemySpawnInterval{ 2.0f };


		
		
		
	};
}