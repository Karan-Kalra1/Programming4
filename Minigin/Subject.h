#pragma once
#include <vector>
#include <algorithm>
#include "Observer.h"

namespace dae
{
	class Subject
	{
	public:
		virtual ~Subject() = default;

		void AddObserver(Observer* observer)
		{
			if (!observer) return;

			auto it = std::find(m_Observers.begin(), m_Observers.end(), observer);
			if (it == m_Observers.end())
				m_Observers.push_back(observer);
		}

		void RemoveObserver(Observer* observer)
		{
			m_Observers.erase(
				std::remove(m_Observers.begin(), m_Observers.end(), observer),
				m_Observers.end());
		}

	
		void Notify(const Event& event)
		{
			for (auto* observer : m_Observers)
			{
				if (observer)
					observer->OnNotify(event);
			}
		}

	private:
		std::vector<Observer*> m_Observers{};
	};
}