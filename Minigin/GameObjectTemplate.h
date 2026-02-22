#pragma once

namespace dae
{
	template<typename T, typename... Args>
	T* GameObject::AddComponent(Args&&... args)
	{
		auto comp = std::make_unique<T>(std::forward<Args>(args)...);
		T* ptr = comp.get();
		m_components.push_back(std::move(comp));
		return ptr;
	}

	template<typename T>
	T* GameObject::GetComponent()
	{
		for (auto& comp : m_components)
		{
			if (auto casted = dynamic_cast<T*>(comp.get()))
				return casted;
		}
		return nullptr;
	}

	template<typename T>
	bool GameObject::HasComponent() const
	{
		for (const auto& comp : m_components)
		{
			if (dynamic_cast<T*>(comp.get()))
				return true;
		}
		return false;
	}

	template<typename T>
	void GameObject::RemoveComponent()
	{
		m_components.erase(
			std::remove_if(
				m_components.begin(),
				m_components.end(),
				[](auto& c)
				{
					return dynamic_cast<T*>(c.get()) != nullptr;
				}),
			m_components.end());
	}
}
