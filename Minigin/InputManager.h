#pragma once
#include <memory>
#include <vector>
#include <SDL3/SDL.h>
#include "Singleton.h"
#include "InputTypes.h"
#include "Controller.h"
#include "Command.h"

namespace dae
{
	

	class InputManager final : public Singleton<InputManager>
	{
	public:
		bool ProcessInput();

		void BindKeyboardCommand(SDL_Scancode key, KeyState state, std::unique_ptr<Command> command);
		void BindControllerCommand(int controllerIndex, ControllerButton button, KeyState state, std::unique_ptr<Command> command);

		void UnbindKeyboardCommand(SDL_Scancode key, KeyState state);
		void UnbindControllerCommand(int controllerIndex, ControllerButton button, KeyState state);

	private:
		friend class Singleton<InputManager>;
		InputManager();

		struct KeyboardBinding
		{
			SDL_Scancode key{};
			KeyState state{};
			std::unique_ptr<Command> command{};
		};

		struct ControllerBinding
		{
			int controllerIndex{};
			ControllerButton button{};
			KeyState state{};
			std::unique_ptr<Command> command{};
		};

		std::vector<std::unique_ptr<Controller>> m_Controllers{};
		std::vector<KeyboardBinding> m_KeyboardBindings{};
		std::vector<ControllerBinding> m_ControllerBindings{};

		const bool* m_pCurrentKeyboardState{};
		std::vector<bool> m_PreviousKeyboardState{};
	};
}