#pragma once
#include <glm/glm.hpp>
#include "Component.h"

namespace dae
{
	class TransformComponent final : public Component
	{
	public:

		TransformComponent(GameObject* owner) :Component::Component(owner) {}

        void SetLocalPosition(glm::vec3 localPosition);

        void SetLocalPosition(float x, float y, float z = 0.f);

        const glm::vec3& GetLocalPosition() const { return m_localPosition; }

        const glm::vec3& GetWorldPosition();

        void SetDirty();

        void SetLocalScale(float x, float y, float z = 1.f);
        const glm::vec3& GetLocalScale() const { return m_localScale; }
        const glm::vec3& GetWorldScale();
        void SetFlipX(bool flip) { m_FlipX = flip; }
        bool IsFlippedX() const { return m_FlipX; }
        void SetFlipY(bool flip) { m_FlipY = flip; }
        bool GetFlipY() const { return m_FlipY; }
        void SetRotation(double angle) { m_Rotation = angle; }
        double GetRotation() const { return m_Rotation; }

    private:
        void UpdateWorldTransform();
       
        double m_Rotation{};
        bool m_FlipX{};
        bool m_FlipY{};
        glm::vec3 m_localPosition{};
        glm::vec3 m_worldPosition{};

        glm::vec3 m_localScale{ 1.f,1.f,1.f };
        glm::vec3 m_worldScale{ 1.f,1.f,1.f };


        bool m_isDirty{ true };
	};
}
