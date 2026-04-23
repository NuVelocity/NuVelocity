#ifndef NVE_INPUT_MANAGER_H
#define NVE_INPUT_MANAGER_H

#include "Manager.h"

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace nuvelocity
{
    class InputManager : public Manager
    {
    public:
        struct ButtonState
        {
            bool down = false;
            bool pressed = false;
            bool released = false;
            uint64_t changeTick = 0;
        };

        struct GamepadState
        {
            std::unordered_map<int, ButtonState> buttons;
            std::unordered_map<int, int16_t> axes;
            bool connected = false;
        };

        InputManager();
        ~InputManager() override = default;

        bool Initialize(char** argv) override;

        void ProcessEvent(const SDL_Event& event);
        void EndFrame();

        bool IsQuitRequested() const;

        bool IsKeyDown(SDL_Scancode scancode) const;
        bool IsKeyPressed(SDL_Scancode scancode) const;
        bool IsKeyReleased(SDL_Scancode scancode) const;

        bool IsMouseButtonDown(uint8_t button) const;
        bool IsMouseButtonPressed(uint8_t button) const;
        bool IsMouseButtonReleased(uint8_t button) const;

        SDL_Point GetMousePosition() const;
        SDL_Point GetMouseDelta() const;
        SDL_Point GetWheelDelta() const;

        bool IsGamepadConnected(SDL_JoystickID gamepadId) const;
        bool IsGamepadButtonDown(SDL_JoystickID gamepadId, SDL_GamepadButton button) const;
        bool IsGamepadButtonPressed(SDL_JoystickID gamepadId, SDL_GamepadButton button) const;
        bool IsGamepadButtonReleased(SDL_JoystickID gamepadId, SDL_GamepadButton button) const;
        int16_t GetGamepadAxis(SDL_JoystickID gamepadId, SDL_GamepadAxis axis) const;

        const std::string& GetTextInput() const;
        const std::vector<SDL_Event>& GetFrameEvents() const;

    private:
        std::unordered_map<SDL_Scancode, ButtonState> mKeyStates;
        std::unordered_map<uint8_t, ButtonState> mMouseButtonStates;
        std::unordered_map<SDL_JoystickID, GamepadState> mGamepads;

        SDL_Point mMousePosition;
        SDL_Point mMouseDelta;
        SDL_Point mWheelDelta;

        bool mQuitRequested;
        std::string mTextInput;
        std::vector<SDL_Event> mFrameEvents;

        static bool GetButtonDown(const std::unordered_map<SDL_Scancode, ButtonState>& states,
                                  SDL_Scancode code,
                                  bool ButtonState::* member);

        static bool GetMouseButtonState(const std::unordered_map<uint8_t, ButtonState>& states,
                                        uint8_t button,
                                        bool ButtonState::* member);

        static bool
        GetGamepadButtonState(const std::unordered_map<SDL_JoystickID, GamepadState>& gamepads,
                              SDL_JoystickID gamepadId,
                              SDL_GamepadButton button,
                              bool ButtonState::* member);

        static void ResetButtonTransitions(std::unordered_map<SDL_Scancode, ButtonState>& states);
        static void ResetButtonTransitions(std::unordered_map<uint8_t, ButtonState>& states);
        static void
        ResetGamepadTransitions(std::unordered_map<SDL_JoystickID, GamepadState>& gamepads);
    };
} // namespace nuvelocity

#endif // NVE_INPUT_MANAGER_H
