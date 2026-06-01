#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "Component.h"

namespace digger
{
	class EnemyState;
	class GameManagerComponent;

	class EnemyComponent final : public dae::Component
	{
	public:
		EnemyComponent(dae::GameObject* owner, GameManagerComponent* manager);
		~EnemyComponent() override;

		void Update() override;

		void ChangeState(std::unique_ptr<EnemyState> newState);

		void MoveTowardPlayer(bool canDig);

		int GetTouchingPlayerIndex() const;

		glm::ivec2 GetGridPosition() const;
		glm::ivec2 GetPlayerGridPosition() const;

		bool CanMoveTo(const glm::ivec2& pos, bool canDig) const;
		void DigTile(const glm::ivec2& pos);
		dae::GameObject* GetGameObject() const;
		void SetGridPosition(const glm::ivec2& pos);
		bool IsMoving() const { return m_IsMoving; }

		//void MoveTowardPlayerSmooth(bool canDig);
		void UpdateSmoothMovement();


		bool IsOnPlayer() const;
		glm::ivec2 FindRoamTarget(bool canDig) const;


		bool IsDead() const { return m_IsDead; }
		void Kill();

		void RegisterEnemyCollision();
		void ResetCollisionCounter();

		GameManagerComponent* GetManager() const { return m_Manager; }

	private:
		GameManagerComponent* m_Manager{};
		std::unique_ptr<EnemyState> m_State{};

		bool m_IsMoving{};
		float m_Speed{ 120.f };
		bool m_IsDead{};
		bool m_CanDigCurrentMove{};

		glm::ivec2 m_CurrentGrid{};
		glm::ivec2 m_TargetGrid{};

		glm::vec2 m_WorldPosition{};
		glm::vec2 m_TargetWorldPosition{};

		int m_CrossCounter{};
	};
}