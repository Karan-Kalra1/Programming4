#pragma once
#include "Event.h"

namespace dae
{
	class Observer
	{
	public:
		virtual ~Observer() = default;
		virtual void OnNotify(const Event& event) = 0;
	};
}