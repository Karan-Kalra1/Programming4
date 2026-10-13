
#include "Controller.h"

#if defined(_WIN32) && !defined(__EMSCRIPTEN__)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#else
#include <SDL3/SDL.h>
#endif

namespace dae
{
    class Controller::ControllerImpl
    {
    public:
        explicit ControllerImpl(int index)
            : m_Index(index)
        {
#if defined(_WIN32) && !defined(__EMSCRIPTEN__)
            // XInput uses controller indexes directly.
#else
            OpenGamepad();
#endif
        }

        ~ControllerImpl()
        {
#if !defined(_WIN32) || defined(__EMSCRIPTEN__)
            CloseGamepad();
#endif
        }

        void Update()
        {
            // Preserve the previous frame's state before polling.
            m_PreviousButtons = m_CurrentButtons;
            m_CurrentButtons = 0;

#if defined(_WIN32) && !defined(__EMSCRIPTEN__)

            XINPUT_STATE state{};
            const DWORD result =
                XInputGetState(static_cast<DWORD>(m_Index), &state);

            if (result == ERROR_SUCCESS)
            {
                const WORD buttons = state.Gamepad.wButtons;

                if (buttons & XINPUT_GAMEPAD_DPAD_UP)
                    m_CurrentButtons |= ToBit(ControllerButton::DPadUp);

                if (buttons & XINPUT_GAMEPAD_DPAD_DOWN)
                    m_CurrentButtons |= ToBit(ControllerButton::DPadDown);

                if (buttons & XINPUT_GAMEPAD_DPAD_LEFT)
                    m_CurrentButtons |= ToBit(ControllerButton::DPadLeft);

                if (buttons & XINPUT_GAMEPAD_DPAD_RIGHT)
                    m_CurrentButtons |= ToBit(ControllerButton::DPadRight);

                if (buttons & XINPUT_GAMEPAD_A)
                    m_CurrentButtons |= ToBit(ControllerButton::ButtonA);

                if (buttons & XINPUT_GAMEPAD_B)
                    m_CurrentButtons |= ToBit(ControllerButton::ButtonB);

                if (buttons & XINPUT_GAMEPAD_X)
                    m_CurrentButtons |= ToBit(ControllerButton::ButtonX);

                if (buttons & XINPUT_GAMEPAD_Y)
                    m_CurrentButtons |= ToBit(ControllerButton::ButtonY);
            }

#else

            // Retry opening if no gamepad was available at construction.
            if (m_Gamepad == nullptr)
                OpenGamepad();

            if (m_Gamepad != nullptr && SDL_GamepadConnected(m_Gamepad))
            {
                if (SDL_GetGamepadButton(
                    m_Gamepad, SDL_GAMEPAD_BUTTON_DPAD_UP))
                    m_CurrentButtons |= ToBit(ControllerButton::DPadUp);

                if (SDL_GetGamepadButton(
                    m_Gamepad, SDL_GAMEPAD_BUTTON_DPAD_DOWN))
                    m_CurrentButtons |= ToBit(ControllerButton::DPadDown);

                if (SDL_GetGamepadButton(
                    m_Gamepad, SDL_GAMEPAD_BUTTON_DPAD_LEFT))
                    m_CurrentButtons |= ToBit(ControllerButton::DPadLeft);

                if (SDL_GetGamepadButton(
                    m_Gamepad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT))
                    m_CurrentButtons |= ToBit(ControllerButton::DPadRight);

                if (SDL_GetGamepadButton(
                    m_Gamepad, SDL_GAMEPAD_BUTTON_SOUTH))
                    m_CurrentButtons |= ToBit(ControllerButton::ButtonA);

                if (SDL_GetGamepadButton(
                    m_Gamepad, SDL_GAMEPAD_BUTTON_EAST))
                    m_CurrentButtons |= ToBit(ControllerButton::ButtonB);

                if (SDL_GetGamepadButton(
                    m_Gamepad, SDL_GAMEPAD_BUTTON_WEST))
                    m_CurrentButtons |= ToBit(ControllerButton::ButtonX);

                if (SDL_GetGamepadButton(
                    m_Gamepad, SDL_GAMEPAD_BUTTON_NORTH))
                    m_CurrentButtons |= ToBit(ControllerButton::ButtonY);
            }
            else if (m_Gamepad != nullptr)
            {
                // The device disconnected.
                CloseGamepad();
            }

#endif
        }

        bool IsDown(ControllerButton button) const
        {
            const uint16_t bit = ToBit(button);

            return (m_CurrentButtons & bit) != 0 &&
                (m_PreviousButtons & bit) == 0;
        }

        bool IsUp(ControllerButton button) const
        {
            const uint16_t bit = ToBit(button);

            return (m_CurrentButtons & bit) == 0 &&
                (m_PreviousButtons & bit) != 0;
        }

        bool IsPressed(ControllerButton button) const
        {
            return (m_CurrentButtons & ToBit(button)) != 0;
        }

    private:
        static uint16_t ToBit(ControllerButton button)
        {
            return static_cast<uint16_t>(
                1u << static_cast<uint16_t>(button));
        }

#if !defined(_WIN32) || defined(__EMSCRIPTEN__)

        void OpenGamepad()
        {
            int count = 0;
            SDL_JoystickID* gamepads = SDL_GetGamepads(&count);

            if (gamepads == nullptr)
                return;

            if (m_Index >= 0 && m_Index < count)
                m_Gamepad = SDL_OpenGamepad(gamepads[m_Index]);

            SDL_free(gamepads);
        }

        void CloseGamepad()
        {
            if (m_Gamepad != nullptr)
            {
                SDL_CloseGamepad(m_Gamepad);
                m_Gamepad = nullptr;
            }
        }

        SDL_Gamepad* m_Gamepad{};

#endif

        int m_Index{};
        uint16_t m_CurrentButtons{};
        uint16_t m_PreviousButtons{};
    };

    Controller::Controller(int index)
        : m_pImpl(std::make_unique<ControllerImpl>(index))
    {}

    Controller::~Controller() = default;

    void Controller::Update()
    {
        m_pImpl->Update();
    }

    bool Controller::IsDown(ControllerButton button) const
    {
        return m_pImpl->IsDown(button);
    }

    bool Controller::IsUp(ControllerButton button) const
    {
        return m_pImpl->IsUp(button);
    }

    bool Controller::IsPressed(ControllerButton button) const
    {
        return m_pImpl->IsPressed(button);
    }
}
