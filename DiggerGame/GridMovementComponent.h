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

	private:
		bool IsDirectionChange(const glm::ivec2& newDirection) const;	
		glm::ivec2 WorldToGrid(const glm::vec2& world) const;
		bool IsChangingAxis(const glm::ivec2& direction) const;
		float DistanceToGridLine(bool horizontalMovement) const;


		GameManagerComponent* m_Manager{};

		int m_TileSize{};
		float m_Speed{};
		glm::vec2 m_Offset{};

		glm::vec2 m_WorldPosition{};

		glm::ivec2 m_CurrentDirection{};
		glm::ivec2 m_HeldDirection{};
		glm::ivec2 m_PendingTurnDirection{};
		glm::ivec2 m_LastDirection{};

		bool m_AligningForTurn{};
	};
}