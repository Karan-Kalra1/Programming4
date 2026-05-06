#pragma once
#include "Component.h"

namespace dae
{
	class GameObject;
}

namespace digger
{
	class GameManagerComponent;

	class DiamondComponent final : public dae::Component
	{
	public:
		DiamondComponent(dae::GameObject* owner, GameManagerComponent* manager)
			: Component(owner)
			, m_Manager(manager)
		{
		}

		void Update() override;

	private:
		GameManagerComponent* m_Manager{};
		bool m_Collected{};
	};
}