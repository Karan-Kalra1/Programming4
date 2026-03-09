#include "InputManager.h"

#include <algorithm>

#include <backends/imgui_impl_sdl3.h>

dae::InputManager::InputManager()
{
	// One controller for now
	m_Controllers.emplace_back(std::make_unique<Controller>(0));

	m_pCurrentKeyboardState = SDL_GetKeyboardState(nullptr);
	m_PreviousKeyboardState.resize(SDL_SCANCODE_COUNT, false);
}

bool dae::InputManager::ProcessInput()
{
	// Store previous keyboard state
	for (int i{}; i < SDL_SCANCODE_COUNT; ++i)
	{
		m_PreviousKeyboardState[i] = m_pCurrentKeyboardState[i];
	}

	SDL_Event e{};
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_EVENT_QUIT)
			return false;

		ImGui_ImplSDL3_ProcessEvent(&e);
	}

	// Update current keyboard state
	m_pCurrentKeyboardState = SDL_GetKeyboardState(nullptr);

	// Update controllers
	for (auto& controller : m_Controllers)
	{
		controller->Update();
	}

	// Execute keyboard commands
	for (auto& binding : m_KeyboardBindings)
	{
		const bool isPressed = m_pCurrentKeyboardState[binding.key];
		const bool wasPressed = m_PreviousKeyboardState[binding.key];

		bool shouldExecute = false;

		switch (binding.state)
		{
		case KeyState::Down:
			shouldExecute = isPressed && !wasPressed;
			break;

		case KeyState::Up:
			shouldExecute = !isPressed && wasPressed;
			break;

		case KeyState::Pressed:
			shouldExecute = isPressed;
			break;
		}

		if (shouldExecute && binding.command)
		{
			binding.command->Execute();
		}
	}

	// Execute controller commands
	for (auto& binding : m_ControllerBindings)
	{
		if (binding.controllerIndex < 0 || binding.controllerIndex >= static_cast<int>(m_Controllers.size()))
			continue;

		const auto& controller = m_Controllers[binding.controllerIndex];
		bool shouldExecute = false;

		switch (binding.state)
		{
		case KeyState::Down:
			shouldExecute = controller->IsDown(binding.button);
			break;

		case KeyState::Up:
			shouldExecute = controller->IsUp(binding.button);
			break;

		case KeyState::Pressed:
			shouldExecute = controller->IsPressed(binding.button);
			break;
		}

		if (shouldExecute && binding.command)
		{
			binding.command->Execute();
		}
	}

	return true;
}

void dae::InputManager::BindKeyboardCommand(SDL_Scancode key, KeyState state, std::unique_ptr<Command> command)
{
	m_KeyboardBindings.push_back(KeyboardBinding{ key, state, std::move(command) });
}

void dae::InputManager::BindControllerCommand(int controllerIndex, ControllerButton button, KeyState state, std::unique_ptr<Command> command)
{
	m_ControllerBindings.push_back(ControllerBinding{ controllerIndex, button, state, std::move(command) });
}

void dae::InputManager::UnbindKeyboardCommand(SDL_Scancode key, KeyState state)
{
	m_KeyboardBindings.erase(
		std::remove_if(
			m_KeyboardBindings.begin(),
			m_KeyboardBindings.end(),
			[key, state](const KeyboardBinding& binding)
			{
				return binding.key == key && binding.state == state;
			}),
		m_KeyboardBindings.end());
}

void dae::InputManager::UnbindControllerCommand(int controllerIndex, ControllerButton button, KeyState state)
{
	m_ControllerBindings.erase(
		std::remove_if(
			m_ControllerBindings.begin(),
			m_ControllerBindings.end(),
			[controllerIndex, button, state](const ControllerBinding& binding)
			{
				return binding.controllerIndex == controllerIndex
					&& binding.button == button
					&& binding.state == state;
			}),
		m_ControllerBindings.end());
}