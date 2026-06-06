#pragma once

#include "PlayerTypes.h"
#include "GameMode.h"
#include "LevelData.h"

#include <array>
#include <optional>
#include <string>
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

	class PlayerManager final
	{
	public:
		explicit PlayerManager(dae::Scene* scene);

		void ResetForNewGame(GameMode mode);
		void ClearLevel();

		void UpdateCooldowns(float deltaTime);

		void SpawnPlayersForLevel(
			GameManagerComponent& game,
			const LevelData& data);

		const std::vector<PlayerRuntime>& GetPlayers() const
		{
			return m_Players;
		}

		const PlayerRuntime* GetPlayerRuntime(int playerIndex) const;

		dae::GameObject* GetPlayer(int playerIndex) const;

		glm::ivec2 GetPlayerGridPosition(int playerIndex) const;
		glm::vec2 GetPlayerWorldPosition(int playerIndex) const;

		glm::ivec2 GetClosestAlivePlayerGridPosition(
			const glm::ivec2& fromGrid) const;

		int GetPlayerIndexAtWorldPosition(
			const glm::vec2& worldPos,
			float radius) const;

		int GetDiggerPlayerIndexAtWorldPosition(
			const glm::vec2& worldPos,
			float radius) const;

		int GetPlayerIndexAtGridPosition(
			const glm::ivec2& pos) const;

		bool IsPlayerAlive(int playerIndex) const;
		bool IsPlayerControllable(
			const GameManagerComponent& game,
			int playerIndex) const;

		bool CanPlayerShootFireball(
			const GameManagerComponent& game,
			int playerIndex) const;

		void StartPlayerFireballCooldown(
			int playerIndex,
			float cooldown);

		void SetPlayerFireballActive(
			int playerIndex,
			bool active);

		void ClearAllPlayerFireballStates();

		void DamagePlayer(
			GameManagerComponent& game,
			int playerIndex);

		void FinishDeathSequence(GameManagerComponent& game);

		bool AreAllPlayersDead() const;

		void CheckVersusCollision(GameManagerComponent& game);

	private:
		void SpawnDiggerPlayer(
			GameManagerComponent& game,
			int playerIndex,
			const glm::ivec2& spawn,
			const std::string& textureFile,
			PlayerRole role);

		void RemovePlayerObservers();
		void ReleaseAllDirections();
		void HidePlayer(int playerIndex);

		int GetPlayerIndexAtWorldPosition(
			const glm::vec2& worldPos,
			float radius,
			std::optional<PlayerRole> requiredRole) const;

		PlayerRuntime* GetMutablePlayerRuntime(int playerIndex);

		dae::Scene* m_Scene{};

		std::vector<PlayerRuntime> m_Players{};

		std::array<int, 2> m_PlayerLives{ 4, 4 };
		std::array<bool, 2> m_PlayerAlive{ true, true };

		int m_DeathPlayerIndex{ -1 };
	};
}