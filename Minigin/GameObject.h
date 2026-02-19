#pragma once
#include <string>
#include <memory>
#include "Transform.h"
#include "Component.h"

namespace dae
{
	class Texture2D;
	class GameObject final
	{
		
	public:
		 void Update();
		 void Render() const;


		GameObject() = default;
		~GameObject();
		GameObject(const GameObject& other) = delete;
		GameObject(GameObject&& other) = delete;
		GameObject& operator=(const GameObject& other) = delete;
		GameObject& operator=(GameObject&& other) = delete;

		template<typename T, typename... Args>
		T* AddComponent(Args&&... args);

		template<typename T>
		T* GetComponent();

		template<typename T>
		bool HasComponent() const;

		template<typename T>
		void RemoveComponent();

	private:
		std::vector<std::unique_ptr<Component>> m_components;


	};
}

#include "GameObjectTemplate.h"

