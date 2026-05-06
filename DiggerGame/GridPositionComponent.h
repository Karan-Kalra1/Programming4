#pragma once
#include "Component.h"
#include <glm/glm.hpp>

namespace digger
{
	class GridPositionComponent final : public dae::Component
	{
	public:
		GridPositionComponent(dae::GameObject* owner, int tileSize)
			: Component(owner)
			, m_TileSize(tileSize)
		{
		}

		void SetGridPosition(const glm::ivec2& pos)
		{
			m_PreviousGridPosition = m_GridPosition;
			m_GridPosition = pos;
			UpdateWorldPosition();
		}

		const glm::ivec2& GetGridPosition() const { return m_GridPosition; }
		const glm::ivec2& GetPreviousGridPosition() const { return m_PreviousGridPosition; }

		void Move(const glm::ivec2& direction)
		{
			SetGridPosition(m_GridPosition + direction);
		}

	private:
		void UpdateWorldPosition();

		int m_TileSize{};
		glm::ivec2 m_GridPosition{};
		glm::ivec2 m_PreviousGridPosition{};
	};
}