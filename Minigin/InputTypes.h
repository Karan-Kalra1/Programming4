#pragma once
#include <cstdint>

namespace dae
{
	enum class KeyState
	{
		Down,
		Up,
		Pressed
	};

	enum class ControllerButton : uint16_t
	{
		DPadUp,
		DPadDown,
		DPadLeft,
		DPadRight,
		ButtonA,
		ButtonB,
		ButtonX,
		ButtonY
	};
}