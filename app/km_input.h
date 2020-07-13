#pragma once

#include <km_common/km_math.h>

enum KmKeyCode
{
    KM_KEY_A = 0,
    KM_KEY_B,
    KM_KEY_C,
    KM_KEY_D,
    KM_KEY_E,
    KM_KEY_F,
    KM_KEY_G,
    KM_KEY_H,
    KM_KEY_I,
    KM_KEY_J,
    KM_KEY_K,
    KM_KEY_L,
    KM_KEY_M,
    KM_KEY_N,
    KM_KEY_O,
    KM_KEY_P,
    KM_KEY_Q,
    KM_KEY_R,
    KM_KEY_S,
    KM_KEY_T,
    KM_KEY_U,
    KM_KEY_V,
    KM_KEY_W,
    KM_KEY_X,
    KM_KEY_Y,
    KM_KEY_Z,
    KM_KEY_SPACE,

    KM_KEY_0,
    KM_KEY_1,
    KM_KEY_2,
    KM_KEY_3,
    KM_KEY_4,
    KM_KEY_5,
    KM_KEY_6,
    KM_KEY_7,
    KM_KEY_8,
    KM_KEY_9,
    KM_KEY_NUMPAD_0,
    KM_KEY_NUMPAD_1,
    KM_KEY_NUMPAD_2,
    KM_KEY_NUMPAD_3,
    KM_KEY_NUMPAD_4,
    KM_KEY_NUMPAD_5,
    KM_KEY_NUMPAD_6,
    KM_KEY_NUMPAD_7,
    KM_KEY_NUMPAD_8,
    KM_KEY_NUMPAD_9,

    KM_KEY_ESCAPE,
    KM_KEY_ENTER,
    KM_KEY_BACKSPACE,
    KM_KEY_TAB,
    KM_KEY_SHIFT,
    KM_KEY_CTRL,

    KM_KEY_ARROW_UP,
    KM_KEY_ARROW_DOWN,
    KM_KEY_ARROW_LEFT,
    KM_KEY_ARROW_RIGHT,

    KM_KEY_LAST, // Always keep these two at the end
    KM_KEY_INVALID,
};

enum KmMouseCode
{
    KM_MOUSE_LEFT,
    KM_MOUSE_RIGHT,
    KM_MOUSE_MIDDLE,
    KM_MOUSE_ALT1,
    KM_MOUSE_ALT2,

    KM_MOUSE_LAST, // Always keep these two at the end
    KM_MOUSE_INVALID,
};

struct AppButtonState
{
    int transitions;
    bool isDown;
};

struct AppControllerInput
{
    bool isConnected;

    Vec2 leftStart;
    Vec2 leftEnd;
    Vec2 rightStart;
    Vec2 rightEnd;

    union
    {
        AppButtonState buttons[6];
        struct
        {
            AppButtonState a;
            AppButtonState b;
            AppButtonState x;
            AppButtonState y;
            AppButtonState lShoulder;
            AppButtonState rShoulder;
        };
    };
};

struct AppInput
{
    static const int MAX_KEYS_PER_FRAME = 256;

    AppButtonState mouseButtons[KM_MOUSE_LAST];
    Vec2Int mousePos;
    Vec2Int mouseDelta;
    int mouseWheel;
    int mouseWheelDelta;

    AppButtonState keyboard[KM_KEY_LAST];
    char keyboardString[MAX_KEYS_PER_FRAME];
    uint32 keyboardStringLen;

    AppControllerInput controllers[4];
};

inline bool KeyDown(const AppInput& input, KmKeyCode keyCode);
inline bool KeyPressed(const AppInput& input, KmKeyCode keyCode);
inline bool KeyReleased(const AppInput& input, KmKeyCode keyCode);

inline bool MouseDown(const AppInput& input, KmMouseCode mouseCode);
inline bool MousePressed(const AppInput& input, KmMouseCode mouseCode);
inline bool MouseReleased(const AppInput& input, KmMouseCode mouseCode);

void ClearInput(AppInput* input);
void ClearInput(AppInput* input, const AppInput& inputPrev);
