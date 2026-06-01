#pragma once
#include "Component.h"
#include <glm/glm.hpp>



namespace dae { class GameObject; }
namespace digger { class EnemyComponent; }

namespace digger
{
	class GameManagerComponent;

	enum class MoneyBagState
	{
		Stable,
		WaitingToFall,
		Falling,
		BrokenGold
	};

	class MoneyBagComponent final : public dae::Component
	{
	public:
		MoneyBagComponent(
			dae::GameObject* owner,
			GameManagerComponent* manager,
			int tileSize,
			glm::vec2 offset);

		void Update() override;

		glm::ivec2 GetGridPosition() const { return m_GridPosition; }

		bool TryPush(const glm::ivec2& direction);
		bool IsBrokenGold() const { return m_State == MoneyBagState::BrokenGold; }
		void SetGridPosition(const glm::ivec2& pos);

	private:
		
		void SetState(MoneyBagState state);

		void UpdateStable();
		void UpdateWaitingToFall();
		void UpdateFalling();

		void BreakIntoGold();
		void UpdateVisual();
		void CheckCrushTargets();
		void DragTargetsDown();
		void KillDraggedTargets();
		void ReleaseDraggedTargets();

		glm::vec2 GridToWorld(const glm::ivec2& grid) const;

		GameManagerComponent* m_Manager{};

		int m_TileSize{};
		glm::vec2 m_Offset{};

		MoneyBagState m_State{ MoneyBagState::Stable };

		glm::ivec2 m_GridPosition{};
		glm::vec2 m_WorldPosition{};
		glm::vec2 m_TargetWorldPosition{};
		int m_DraggedPlayerIndex{ -1 };
		EnemyComponent* m_DraggedEnemy{};

		bool m_HasFallen{};

		float m_FallDelayTimer{};
		float m_FallDelay{ 0.35f };

		float m_FallSpeed{ 220.f };
		int m_FallDistance{};
	};
}