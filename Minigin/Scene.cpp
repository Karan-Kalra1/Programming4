#include <algorithm>
#include "Scene.h"

using namespace dae;

void Scene::Add(std::unique_ptr<GameObject> object)
{
	assert(object != nullptr && "Cannot add a null GameObject to the scene.");
	m_objects.emplace_back(std::move(object));
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
	for(auto& object : m_objects)
	{
		object->Update();
	}

	DeleteMarked();
}

void Scene::Render() const
{
	for (const auto& object : m_objects)
	{
		object->Render();
	}
}

