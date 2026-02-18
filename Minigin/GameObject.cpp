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


