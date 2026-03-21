#pragma once
#include "Singleton.h"
#include "Subject.h"

namespace dae
{
	class EventBus final : public Singleton<EventBus>
	{
	public:
		Subject& GetSubject() { return m_Subject; }

	private:
		friend class Singleton<EventBus>;
		EventBus() = default;

		Subject m_Subject{};
	};
}