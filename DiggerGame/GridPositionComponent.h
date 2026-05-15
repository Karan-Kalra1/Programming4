#pragma once
#include "Component.h"
#include <glm/glm.hpp>

namespace digger
{
	class GridPositionComponent final : public dae::Component
	{
	public:
		GridPositionComponent(dae::GameObject* owner, int tileSize, glm::vec2 offset)
			: Component(owner)
			, m_TileSize(tileSize)
			, m_Offset(offset)
		{
		}

		void SetGridPosition(const glm::ivec2& pos);
		const glm::ivec2& GetGridPosition() const { return m_GridPosition; }

		void Move(const glm::ivec2& direction)
		{
			SetGridPosition(m_GridPosition + direction);
		}

	private:
		void UpdateWorldPosition();

		int m_TileSize{};
		glm::vec2 m_Offset{};
		glm::ivec2 m_GridPosition{};
	};
}