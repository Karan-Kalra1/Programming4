#define _USE_MATH_DEFINES

#include "CircularMovementComponent.h"
#include "TransformComponent.h"
#include "GameObject.h"
#include "MiniginTime.h"
#include <cmath>
#include <math.h>


dae::CircularMovementComponent::CircularMovementComponent(GameObject* owner,float radius, float speed)
    : Component::Component(owner), m_radius(radius), m_speed(speed)
{
    m_pTransform = GetOwner()->GetComponent<TransformComponent>();
}

void dae::CircularMovementComponent::Update()
{
    if (m_angle >= 2 * M_PI || m_angle<= - 2 * M_PI)
    {
        m_angle = 0;
    }

    m_angle += m_speed * dae::MiniginTime::GetDeltaTime();

    float x = cos(m_angle) * m_radius;
    float y = sin(m_angle) * m_radius;

    if(m_pTransform)
    m_pTransform->SetLocalPosition(x, y);
}