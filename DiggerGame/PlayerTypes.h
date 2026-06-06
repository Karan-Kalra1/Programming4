#pragma once

#include <glm/glm.hpp>

namespace dae
{
	class GameObject;
	class GameActorComponent;
}

namespace digger
{
	enum class PlayerRole
	{
		Digger,
		VersusEnemy
	};

	struct PlayerRuntime
	{
		dae::GameObject* object{};
		dae::GameActorComponent* actor{};

		glm::ivec2 spawn{};

		int lives{ 4 };
		bool alive{ false };
		bool dying{ false };

		float damageCooldown{};
		float fireballCooldown{};
		bool fireballActive{};

		PlayerRole role{ PlayerRole::Digger };
	};
}