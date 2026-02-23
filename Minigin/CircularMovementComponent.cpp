#include "CircularMovementComponent.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "MiniginTime.h"
#include <cmath>

dae::CircularMovementComponent::CircularMovementComponent(GameObject* owner,float radius, float speed)
    : Component::Component(owner), m_radius(radius), m_speed(speed)
{
}

void dae::CircularMovementComponent::Update()
{
    m_angle += m_speed * dae::MiniginTime::GetDeltaTime();

    float x = cos(m_angle) * m_radius;
    float y = sin(m_angle) * m_radius;

    auto transform = GetOwner()->GetComponent<TransformComponent>();
    transform->SetLocalPosition(x, y);
}