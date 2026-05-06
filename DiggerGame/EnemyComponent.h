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

		glm::ivec2 GetGridPosition() const;
		glm::ivec2 GetPlayerGridPosition() const;

		bool CanMoveTo(const glm::ivec2& pos, bool canDig) const;
		void DigTile(const glm::ivec2& pos);
		dae::GameObject* GetGameObject() const;

		bool IsOnPlayer() const;

		void RegisterEnemyCollision();
		void ResetCollisionCounter();

		GameManagerComponent* GetManager() const { return m_Manager; }

	private:
		GameManagerComponent* m_Manager{};
		std::unique_ptr<EnemyState> m_State{};

		int m_CrossCounter{};
	};
}