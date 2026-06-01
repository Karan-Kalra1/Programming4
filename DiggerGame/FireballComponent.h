#pragma once
#include "Component.h"
#include <glm/glm.hpp>

namespace digger
{
	class GameManagerComponent;

	class FireballComponent final : public dae::Component
	{
	public:
		FireballComponent(
			dae::GameObject* owner,
			GameManagerComponent* manager,
			glm::vec2 direction,
			float speed,
			int ownerPlayerIndex);

		int GetOwnerPlayerIndex() const { return m_OwnerPlayerIndex; }

		void Update() override;

	private:
		GameManagerComponent* m_Manager{};
		glm::vec2 m_Direction{};
		float m_Speed{};
		int m_OwnerPlayerIndex{};
	};
}