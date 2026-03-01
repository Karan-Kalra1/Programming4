#pragma once
#include "Component.h"
#include <memory>

namespace dae
{
    class TransformComponent;

    class CircularMovementComponent final : public Component
    {
    public:
        CircularMovementComponent(GameObject* owner,float radius, float speed);

        void Update() override;

    private:
        float m_radius{};
        float m_speed{};
        float m_angle{};
        TransformComponent* m_pTransform{};
    };
}