#pragma once

#include <glm/glm.hpp>

namespace dae
{
	class Scene;
	class GameObject;
}

namespace digger
{
	class DeathSequenceController final
	{
	public:
		DeathSequenceController(dae::Scene* scene, float duration);

		void Begin(const glm::vec2& tombstonePosition);
		bool Update(float deltaTime);

		void Clear();

		bool IsActive() const { return m_Active; }

	private:
		void SpawnTombstone(const glm::vec2& position);
		void RemoveTombstone();

		dae::Scene* m_Scene{};

		bool m_Active{};
		float m_Timer{};
		float m_Duration{};

		dae::GameObject* m_Tombstone{};
	};
}