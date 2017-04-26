#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
    \addtogroup input
    \{
*/

/** Helper enum for describing possible digital element state flags */
enum class eDigitalElementStates : uint32
{
    /** A button is in released state */
    RELEASED = 0,

    /** A button is in pressed state */
    PRESSED = 1 << 0,

    /** A button has just been pressed */
    JUST_PRESSED = 1 << 1,

    /** A button has just been released */
    JUST_RELEASED = 1 << 2
};

DAVA_DEFINE_ENUM_BITWISE_OPERATORS(eDigitalElementStates)

/**
Struct describing digital element state.

|                                                  | IsPressed() | IsJustPressed() | IsReleased() | IsJustReleased() |
|--------------------------------------------------|-------------|-----------------|--------------|------------------|
| Initial element state                            | -           | -               | +            | -                |
| Right after a user pressed a button (same frame) | +           | +               | -            | -                |
| A user keeps the button pressed (next frames)    | +           | -               | -            | -                |
| A user released the button (same frame)          | -           | -               | +            | +                |
| A user released the button (next frames)         | -           | -               | +            | -                |
*/
struct DigitalElementState
{
    DigitalElementState(eDigitalElementStates initialState)
        : state(initialState)
    {
    }
    DigitalElementState()
        : DigitalElementState(eDigitalElementStates::RELEASED)
    {
    }

    void Press()
    {
        if (!IsPressed())
        {
            state = eDigitalElementStates::PRESSED | eDigitalElementStates::JUST_PRESSED;
        }
    }

    void Release()
    {
        if (!IsReleased())
        {
            state = eDigitalElementStates::JUST_RELEASED;
        }
    }

    bool IsPressed() const
    {
        return (state & eDigitalElementStates::PRESSED) == eDigitalElementStates::PRESSED;
    }

    bool IsJustPressed() const
    {
        return (state & eDigitalElementStates::JUST_PRESSED) == eDigitalElementStates::JUST_PRESSED;
    }

    bool IsReleased() const
    {
        return state == eDigitalElementStates::RELEASED;
    }

    bool IsJustReleased() const
    {
        return (state & eDigitalElementStates::JUST_RELEASED) == eDigitalElementStates::JUST_RELEASED;
    }

    void OnEndFrame()
    {
        state &= ~(eDigitalElementStates::JUST_PRESSED | eDigitalElementStates::JUST_RELEASED);
    }

    bool operator==(const DigitalElementState& other) const
    {
        if (other.state == eDigitalElementStates::RELEASED)
        {
            return state == eDigitalElementStates::RELEASED;
        }
        else
        {
            return (state & other.state) == other.state;
        }
    }

    bool operator!=(const DigitalElementState& other) const
    {
        return !(*this == other);
    }

private:
    eDigitalElementStates state;
};

/**
Struct describing analog element state.
Meanings of `x`, `y` and `z` values can be different for different elements.

For example, a gamepad's stick defines x and y values in range of [-1; 1] for according axes.
*/
struct AnalogElementState final
{
    /** Analog X value */
    float32 x;

    /** Analog Y value */
    float32 y;

    /** Analog Z value */
    float32 z;
};

/**
    \}
*/
}