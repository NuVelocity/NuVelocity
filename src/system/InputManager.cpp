#include "InputManager.h"

namespace nuvelocity
{
    InputManager::InputManager()
            : mMousePosition({.x = 0.0F, .y = 0.0F})
            , mMouseDelta({.x = 0.0F, .y = 0.0F})
            , mWheelDelta({.x = 0.0F, .y = 0.0F})
            , mQuitRequested(false)
    {
    }

    bool InputManager::Initialize(char** argv)
    {
        (void)argv;
        if (WarnIfAlreadyInitialized("InputManager"))
        {
            return true;
        }

        mInitialized = true;
        return true;
    }

    void InputManager::ProcessEvent(const SDL_Event& event)
    {
        mFrameEvents.push_back(event);

        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            mQuitRequested = true;
            break;

        case SDL_EVENT_KEY_DOWN:
        {
            ButtonState& state = mKeyStates[event.key.scancode];
            if (!state.down)
            {
                state.pressed = true;
            }
            state.down = true;
            state.released = false;
            state.changeTick = event.key.timestamp;
            break;
        }

        case SDL_EVENT_KEY_UP:
        {
            ButtonState& state = mKeyStates[event.key.scancode];
            state.down = false;
            state.pressed = false;
            state.released = true;
            state.changeTick = event.key.timestamp;
            break;
        }

        case SDL_EVENT_MOUSE_MOTION:
            mMousePosition.x = event.motion.x;
            mMousePosition.y = event.motion.y;
            mMouseDelta.x += event.motion.xrel;
            mMouseDelta.y += event.motion.yrel;
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            ButtonState& state = mMouseButtonStates[event.button.button];
            if (!state.down)
            {
                state.pressed = true;
            }
            state.down = true;
            state.released = false;
            state.changeTick = event.button.timestamp;
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            ButtonState& state = mMouseButtonStates[event.button.button];
            state.down = false;
            state.pressed = false;
            state.released = true;
            state.changeTick = event.button.timestamp;
            break;
        }

        case SDL_EVENT_MOUSE_WHEEL:
            mWheelDelta.x += event.wheel.x;
            mWheelDelta.y += event.wheel.y;
            break;

        case SDL_EVENT_TEXT_INPUT:
            if (event.text.text[0] != '\0')
            {
                mTextInput += event.text.text;
            }
            break;

        case SDL_EVENT_GAMEPAD_ADDED:
        {
            GamepadState& gamepad = mGamepads[event.gdevice.which];
            gamepad.connected = true;
            break;
        }

        case SDL_EVENT_GAMEPAD_REMOVED:
        {
            auto gamepadIt = mGamepads.find(event.gdevice.which);
            if (gamepadIt != mGamepads.end())
            {
                gamepadIt->second.connected = false;
                gamepadIt->second.buttons.clear();
                gamepadIt->second.axes.clear();
            }
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        {
            GamepadState& gamepad = mGamepads[event.gbutton.which];
            gamepad.connected = true;
            ButtonState& state = gamepad.buttons[static_cast<int>(event.gbutton.button)];
            if (!state.down)
            {
                state.pressed = true;
            }
            state.down = true;
            state.released = false;
            state.changeTick = event.gbutton.timestamp;
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_UP:
        {
            GamepadState& gamepad = mGamepads[event.gbutton.which];
            gamepad.connected = true;
            ButtonState& state = gamepad.buttons[static_cast<int>(event.gbutton.button)];
            state.down = false;
            state.pressed = false;
            state.released = true;
            state.changeTick = event.gbutton.timestamp;
            break;
        }

        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        {
            GamepadState& gamepad = mGamepads[event.gaxis.which];
            gamepad.connected = true;
            gamepad.axes[static_cast<int>(event.gaxis.axis)] = event.gaxis.value;
            break;
        }

        default:
            break;
        }
    }

    void InputManager::EndFrame()
    {
        ResetButtonTransitions(mKeyStates);
        ResetButtonTransitions(mMouseButtonStates);
        ResetGamepadTransitions(mGamepads);

        mMouseDelta = SDL_FPoint{.x = 0.0F, .y = 0.0F};
        mWheelDelta = SDL_FPoint{.x = 0.0F, .y = 0.0F};
        mTextInput.clear();
        mFrameEvents.clear();
    }

    bool InputManager::IsQuitRequested() const
    {
        return mQuitRequested;
    }

    bool InputManager::IsKeyDown(SDL_Scancode scancode) const
    {
        return GetButtonDown(mKeyStates, scancode, &ButtonState::down);
    }

    bool InputManager::IsKeyPressed(SDL_Scancode scancode) const
    {
        return GetButtonDown(mKeyStates, scancode, &ButtonState::pressed);
    }

    bool InputManager::IsKeyReleased(SDL_Scancode scancode) const
    {
        return GetButtonDown(mKeyStates, scancode, &ButtonState::released);
    }

    bool InputManager::IsMouseButtonDown(uint8_t button) const
    {
        return GetMouseButtonState(mMouseButtonStates, button, &ButtonState::down);
    }

    bool InputManager::IsMouseButtonPressed(uint8_t button) const
    {
        return GetMouseButtonState(mMouseButtonStates, button, &ButtonState::pressed);
    }

    bool InputManager::IsMouseButtonReleased(uint8_t button) const
    {
        return GetMouseButtonState(mMouseButtonStates, button, &ButtonState::released);
    }

    SDL_FPoint InputManager::GetMousePosition() const
    {
        return mMousePosition;
    }

    SDL_FPoint InputManager::GetMouseDelta() const
    {
        return mMouseDelta;
    }

    SDL_FPoint InputManager::GetWheelDelta() const
    {
        return mWheelDelta;
    }

    bool InputManager::IsGamepadConnected(SDL_JoystickID gamepadId) const
    {
        auto gamepadIt = mGamepads.find(gamepadId);
        if (gamepadIt == mGamepads.end())
        {
            return false;
        }

        return gamepadIt->second.connected;
    }

    bool InputManager::IsGamepadButtonDown(SDL_JoystickID gamepadId, SDL_GamepadButton button) const
    {
        return GetGamepadButtonState(mGamepads, gamepadId, button, &ButtonState::down);
    }

    bool InputManager::IsGamepadButtonPressed(SDL_JoystickID gamepadId,
                                              SDL_GamepadButton button) const
    {
        return GetGamepadButtonState(mGamepads, gamepadId, button, &ButtonState::pressed);
    }

    bool InputManager::IsGamepadButtonReleased(SDL_JoystickID gamepadId,
                                               SDL_GamepadButton button) const
    {
        return GetGamepadButtonState(mGamepads, gamepadId, button, &ButtonState::released);
    }

    int16_t InputManager::GetGamepadAxis(SDL_JoystickID gamepadId, SDL_GamepadAxis axis) const
    {
        auto gamepadIt = mGamepads.find(gamepadId);
        if (gamepadIt == mGamepads.end())
        {
            return 0;
        }

        auto axisIt = gamepadIt->second.axes.find(axis);
        if (axisIt == gamepadIt->second.axes.end())
        {
            return 0;
        }

        return axisIt->second;
    }

    const std::string& InputManager::GetTextInput() const
    {
        return mTextInput;
    }

    const std::vector<SDL_Event>& InputManager::GetFrameEvents() const
    {
        return mFrameEvents;
    }

    bool InputManager::GetButtonDown(const std::unordered_map<SDL_Scancode, ButtonState>& states,
                                     SDL_Scancode code,
                                     bool ButtonState::* member)
    {
        auto stateIt = states.find(code);
        if (stateIt == states.end())
        {
            return false;
        }

        return stateIt->second.*member;
    }

    bool InputManager::GetMouseButtonState(const std::unordered_map<uint8_t, ButtonState>& states,
                                           uint8_t button,
                                           bool ButtonState::* member)
    {
        auto stateIt = states.find(button);
        if (stateIt == states.end())
        {
            return false;
        }

        return stateIt->second.*member;
    }

    bool InputManager::GetGamepadButtonState(
        const std::unordered_map<SDL_JoystickID, GamepadState>& gamepads,
        SDL_JoystickID gamepadId,
        SDL_GamepadButton button,
        bool ButtonState::* member)
    {
        auto gamepadIt = gamepads.find(gamepadId);
        if (gamepadIt == gamepads.end())
        {
            return false;
        }

        auto buttonIt = gamepadIt->second.buttons.find(static_cast<int>(button));
        if (buttonIt == gamepadIt->second.buttons.end())
        {
            return false;
        }

        return buttonIt->second.*member;
    }

    void InputManager::ResetButtonTransitions(std::unordered_map<SDL_Scancode, ButtonState>& states)
    {
        for (auto& [code, state] : states)
        {
            (void)code;
            state.pressed = false;
            state.released = false;
        }
    }

    void InputManager::ResetButtonTransitions(std::unordered_map<uint8_t, ButtonState>& states)
    {
        for (auto& [button, state] : states)
        {
            (void)button;
            state.pressed = false;
            state.released = false;
        }
    }

    void InputManager::ResetGamepadTransitions(
        std::unordered_map<SDL_JoystickID, GamepadState>& gamepads)
    {
        for (auto& [gamepadId, gamepad] : gamepads)
        {
            (void)gamepadId;
            for (auto& [button, state] : gamepad.buttons)
            {
                (void)button;
                state.pressed = false;
                state.released = false;
            }
        }
    }
} // namespace nuvelocity
