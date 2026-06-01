#pragma once
#include "Component.h"
#include <glm/glm.hpp>

namespace digger
{
	class GameManagerComponent;

	class GridMovementComponent final : public dae::Component
	{
	public:
		GridMovementComponent(
			dae::GameObject* owner,
			GameManagerComponent* manager,
			int tileSize,
			float speed,
			glm::vec2 offset);

		void Update() override;

		void SetGridPosition(const glm::ivec2& position);

		void PressDirection(const glm::ivec2& direction);
		void ReleaseDirection(const glm::ivec2& direction);

		glm::ivec2 GetGridPosition() const;
		glm::vec2 GetWorldPosition() const { return m_WorldPosition; }
		void ReleaseAllDirections();
		glm::ivec2 GetFacingDirection() const { return m_FacingDirection; }
		

	private:
		glm::ivec2 WorldToGrid(const glm::vec2& world) const;
		bool CanOccupyPosition(const glm::vec2& worldPos) const;
		bool IsSameAxis(const glm::ivec2& a, const glm::ivec2& b) const;
		bool IsDirectionHeld(const glm::ivec2& direction) const;

		void SetCurrentDirection(const glm::ivec2& direction);
		void ApplyRotation();
		void UpdateTransform();
		void Dig();

		GameManagerComponent* m_Manager{};

		int m_TileSize{};
		float m_Speed{};
		glm::vec2 m_Offset{};

		glm::vec2 m_WorldPosition{};

		glm::ivec2 m_CurrentDirection{};
		glm::ivec2 m_LastDirection{};
		glm::ivec2 m_PendingTurnDirection{};
		glm::ivec2 m_FacingDirection{ 1, 0 };

		bool m_AligningForTurn{};
		bool m_TurnKeyHeld{};

		bool m_HeldUp{};
		bool m_HeldDown{};
		bool m_HeldLeft{};
		bool m_HeldRight{};

		glm::vec2 m_PlayerCenterOffset{ 32.f, 32.f };
	};
}