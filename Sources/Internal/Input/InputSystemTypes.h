#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
    \addtogroup input
    \{
*/

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
struct DigitalElementState final
{
    static DigitalElementState Pressed()
    {
        DigitalElementState result;
        result.pressed = true;
        result.justChanged = false;

        return result;
    }

    static DigitalElementState JustPressed()
    {
        DigitalElementState result;
        result.pressed = true;
        result.justChanged = true;

        return result;
    }

    static DigitalElementState Released()
    {
        DigitalElementState result;

        return result;
    }

    static DigitalElementState JustReleased()
    {
        DigitalElementState result;
        result.justChanged = true;

        return result;
    }

    void Press()
    {
        if (!pressed)
        {
            pressed = true;
            justChanged = true;
        }
    }

    void Release()
    {
        if (pressed)
        {
            pressed = false;
            justChanged = true;
        }
    }

    bool IsPressed() const
    {
        return pressed;
    }

    bool IsJustPressed() const
    {
        return (pressed && justChanged);
    }

    bool IsReleased() const
    {
        return !pressed;
    }

    bool IsJustReleased() const
    {
        return (!pressed && justChanged);
    }

    void OnEndFrame()
    {
        justChanged = false;
    }

    bool operator==(const DigitalElementState& other) const
    {
        return (pressed == other.pressed) && (justChanged == other.justChanged);
    }

    bool operator!=(const DigitalElementState& other) const
    {
        return !(*this == other);
    }

private:
    bool pressed = false;
    bool justChanged = false;
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