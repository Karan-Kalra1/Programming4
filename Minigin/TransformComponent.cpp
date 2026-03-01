#include "TransformComponent.h"
#include "GameObject.h"

void dae::TransformComponent::SetLocalPosition(glm::vec3 localPosition)
{
    m_localPosition = localPosition;
    SetDirty();
}

void dae::TransformComponent::SetLocalPosition(float x, float y, float z)
{
    m_localPosition = { x,y,z };
    SetDirty();
}




void dae::TransformComponent::SetLocalScale(float x, float y, float z)
{
    m_localScale = { x,y,z };
    SetDirty();
}

void dae::TransformComponent::SetDirty()
{
    
        m_isDirty = true;

        for (size_t i = 0; i < GetOwner()->GetChildCount(); ++i)
        {
            auto child = GetOwner()->GetChildAt(i);
            auto childTransform = child->GetComponent<TransformComponent>();
            if (childTransform)
                childTransform->SetDirty();
        }
    
}

const glm::vec3& dae::TransformComponent::GetWorldPosition()
{
    if (m_isDirty)
        UpdateWorldTransform();

    return m_worldPosition;
}

const glm::vec3& dae::TransformComponent::GetWorldScale()
{
    if (m_isDirty)
        UpdateWorldTransform();

    return m_worldScale;
}

void dae::TransformComponent::UpdateWorldTransform()
{
    auto parent = GetOwner()->GetParent();

    if (parent)
    {
        auto parentTransform = parent->GetComponent<TransformComponent>();

        // Position
        m_worldPosition =
            parentTransform->GetWorldPosition() +
            (m_localPosition );

        // Scale
        m_worldScale =
            parentTransform->GetWorldScale() * m_localScale;
    }
    else
    {
        m_worldPosition = m_localPosition;
        m_worldScale = m_localScale;
    }

    m_isDirty = false;
}