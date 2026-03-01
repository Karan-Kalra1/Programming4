#include <string>
#include "GameObject.h"
#include "ResourceManager.h"
#include "Renderer.h"

dae::GameObject::~GameObject() = default;

void dae::GameObject::Update()
{
	for (auto& component : m_components)
		component->Update();

	
}

void dae::GameObject::Render() const
{
	for (const auto& component : m_components)
		component->Render();

}


void dae::GameObject::SetParent(GameObject* newParent, bool keepWorldPosition)
{
    if (newParent == m_parent || newParent == this || IsChild(newParent))
        return;

    auto transform = GetComponent<TransformComponent>();
    glm::vec3 worldPos = transform->GetWorldPosition();

    if (newParent == nullptr)
    {
        
        transform->SetLocalPosition(worldPos);
    }

    else
    {
        
        if (keepWorldPosition)
        {
            glm::vec3 parentWorld =
                newParent->GetComponent<TransformComponent>()->GetWorldPosition();
            transform->SetLocalPosition(worldPos - parentWorld);
        }
        else
        {
            transform->SetDirty();
        }

    }

    if (m_parent)
        m_parent->RemoveChild(this);

    m_parent = newParent;

    if (m_parent)
        m_parent->AddChild(this);
}

void dae::GameObject::AddChild(GameObject* child)
{
    if (!child) return;
    m_children.push_back(child);
}

void dae::GameObject::RemoveChild(GameObject* child)
{
    m_children.erase(
        std::remove(m_children.begin(), m_children.end(), child),
        m_children.end());
}

bool dae::GameObject::IsChild(GameObject* object) const
{
    if (!object) return false;

    for (auto child : m_children)
    {
        if (child == object || child->IsChild(object))
            return true;
    }
    return false;
}

    