#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
/**
\ingroup input
Enum describing possible states for a digital element.

|                                                |  JUST_PRESSED | PRESSED | JUST_RELEASED | RELEASED |
|------------------------------------------------|---------------|---------|---------------|----------|
| initial element state                          | -             | -       | -             | +        |
| right after user pressed a button (same frame) | +             | +       | -             | -        |
| user keeps the button pressed (next frames)    | -             | +       | -             | -        |
| user released the button (same frame)          | -             | -       | +             | +        |
| user released the button (next frames)         | -             | -       | -             | +        |
*/
enum class eDigitalElementStates : uint32
{
    /** Helper value to use in bitwise operations */
    NONE = 0,

    /** A button is in released state */
    RELEASED = 0,

    /** A button is in pressed state */
    PRESSED = 1 << 0,

    /** A button has just been pressed */
    JUST_PRESSED = 1 << 1,

    /** A button has just been released */
    JUST_RELEASED = 1 << 1
};

DAVA_DEFINE_ENUM_BITWISE_OPERATORS(eDigitalElementStates)

/**
\ingroup input
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
}