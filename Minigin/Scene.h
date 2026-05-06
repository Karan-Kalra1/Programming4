#pragma once
#include <memory>
#include <string>
#include <vector>
#include "GameObject.h"

namespace dae
{
	class Scene final
	{
	public:
		void Add(std::unique_ptr<GameObject> object);
		void Remove(GameObject& object);
		void RemoveAll();
		void DeleteMarked();

		void Update();
		void Render() const;

		~Scene() = default;
		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = delete;
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) = delete;

	private:
		friend class SceneManager;
		explicit Scene() = default;

		bool m_IsUpdating{ false };
		std::vector<std::unique_ptr<GameObject>> m_PendingObjects;
		std::vector < std::unique_ptr<GameObject>> m_objects{};
	};

}
