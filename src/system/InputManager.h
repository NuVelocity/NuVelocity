#ifndef NVE_INPUT_MANAGER_H
#define NVE_INPUT_MANAGER_H

#include "API.h"
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

        NVE_API InputManager();
        NVE_API ~InputManager() override = default;

        NVE_API bool Initialize(char** argv) override;

        NVE_API void ProcessEvent(const SDL_Event& event);
        NVE_API void EndFrame();

        NVE_API bool IsQuitRequested() const;

        NVE_API bool IsKeyDown(SDL_Scancode scancode) const;
        NVE_API bool IsKeyPressed(SDL_Scancode scancode) const;
        NVE_API bool IsKeyReleased(SDL_Scancode scancode) const;

        NVE_API bool IsMouseButtonDown(uint8_t button) const;
        NVE_API bool IsMouseButtonPressed(uint8_t button) const;
        NVE_API bool IsMouseButtonReleased(uint8_t button) const;
        NVE_API void ConsumeKey(SDL_Scancode scancode);
        NVE_API void ConsumeMouseButton(uint8_t button);

        NVE_API SDL_Point GetMousePosition() const;
        NVE_API SDL_Point GetMouseDelta() const;
        NVE_API SDL_Point GetWheelDelta() const;

        NVE_API bool IsGamepadConnected(SDL_JoystickID gamepadId) const;
        NVE_API bool IsGamepadButtonDown(SDL_JoystickID gamepadId, SDL_GamepadButton button) const;
        NVE_API bool IsGamepadButtonPressed(SDL_JoystickID gamepadId,
                                            SDL_GamepadButton button) const;
        NVE_API bool IsGamepadButtonReleased(SDL_JoystickID gamepadId,
                                             SDL_GamepadButton button) const;
        NVE_API int16_t GetGamepadAxis(SDL_JoystickID gamepadId, SDL_GamepadAxis axis) const;

        NVE_API const std::string& GetTextInput() const;
        NVE_API const std::vector<SDL_Event>& GetFrameEvents() const;

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
