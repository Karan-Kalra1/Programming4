#include <algorithm>
#include "Scene.h"

using namespace dae;

void Scene::Add(std::unique_ptr<GameObject> object)
{
	assert(object != nullptr && "Cannot add a null GameObject to the scene.");

	
		if (m_IsUpdating)
		{
			m_PendingObjects.emplace_back(std::move(object));
		}
		else
		{
			m_objects.emplace_back(std::move(object));
		}
	
	
}

void Scene::Remove(GameObject& object)
{
	object.MarkForDelete();

	/*m_objects.erase(
		std::remove_if(
			m_objects.begin(),
			m_objects.end(),
			[&object](const auto& ptr) { return ptr.get() == &object && &object.MarkforDelete(); }
		),
		m_objects.end()
	);
	*/
}

void Scene::RemoveAll()
{
	m_objects.clear();
}

void dae::Scene::DeleteMarked()
{
	/*for (auto it = m_objects.begin(); it != m_objects.end(); )
	{
		if ((*it)->GetMarkForDelete())
		{
			it = m_objects.erase(it); 
		}
		else
		{
			++it;
		}
	}*/

	m_objects.erase(
		std::remove_if(
			m_objects.begin(),
			m_objects.end(),
			[](const std::unique_ptr<GameObject>& c)
			{
				return c->GetMarkForDelete();
			}),
		m_objects.end());
		

}

void Scene::Update()
{
	m_IsUpdating = true;

	for (auto& object : m_objects)
	{
		if (object)
			object->Update();
	}

	m_IsUpdating = false;

	for (auto& object : m_PendingObjects)
	{
		m_objects.emplace_back(std::move(object));
	}

	m_PendingObjects.clear();

	DeleteMarked();
}

void Scene::Render() const
{
	for (const auto& object : m_objects)
	{
		object->Render();
	}
}

