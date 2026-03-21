#pragma once

namespace dae
{
	class GameObject;

	enum class EventType
	{
		// intent/request events
		DamageRequested,
		ScoreRequested,

		// result/state events
		PlayerDied,
		LivesChanged,
		ScoreChanged
	};

	struct Event
	{
		EventType type{};
		GameObject* actor{};
		int value{};
	};
}