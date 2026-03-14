#pragma once

namespace dae
{
	class GameObject;

	enum class EventType
	{
		PlayerDied,
		LivesChanged,
		ScoreChanged
	};

	struct Event
	{
		EventType type{};
		GameObject* actor{};
		int value{}; // lives or score depending on event
	};
}