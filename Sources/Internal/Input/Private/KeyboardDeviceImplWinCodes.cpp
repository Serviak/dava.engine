#pragma once

#include "Input/Private/KeyboardDeviceImplWinCodes.h"

namespace DAVA
{
namespace Private
{
const eInputElements nativeScancodeToDavaScancode[] =
{
  eInputElements::NONE, // 0x00
  eInputElements::KB_ESCAPE, // 0x01
  eInputElements::KB_1, // 0x02
  eInputElements::KB_2, // 0x03
  eInputElements::KB_3, // 0x04
  eInputElements::KB_4, // 0x05
  eInputElements::KB_5, // 0x06
  eInputElements::KB_6, // 0x07
  eInputElements::KB_7, // 0x08
  eInputElements::KB_8, // 0x09
  eInputElements::KB_9, // 0x0A
  eInputElements::KB_0, // 0x0B
  eInputElements::KB_MINUS, // 0x0C
  eInputElements::KB_EQUALS, // 0x0D
  eInputElements::KB_BACKSPACE, // 0x0E
  eInputElements::KB_TAB, // 0x0F
  eInputElements::KB_Q, // 0x10
  eInputElements::KB_W, // 0x11
  eInputElements::KB_E, // 0x12
  eInputElements::KB_R, // 0x13
  eInputElements::KB_T, // 0x14
  eInputElements::KB_Y, // 0x15
  eInputElements::KB_U, // 0x16
  eInputElements::KB_I, // 0x17
  eInputElements::KB_O, // 0x18
  eInputElements::KB_P, // 0x19
  eInputElements::KB_LBRACKET, // 0x1A
  eInputElements::KB_RBRACKET, // 0x1B
  eInputElements::KB_ENTER, // 0x1C
  eInputElements::KB_LCTRL, // 0x1D
  eInputElements::KB_A, // 0x1E
  eInputElements::KB_S, // 0x1F
  eInputElements::KB_D, // 0x20
  eInputElements::KB_F, // 0x21
  eInputElements::KB_G, // 0x22
  eInputElements::KB_H, // 0x23
  eInputElements::KB_J, // 0x24
  eInputElements::KB_K, // 0x25
  eInputElements::KB_L, // 0x26
  eInputElements::KB_SEMICOLON, // 0x27
  eInputElements::KB_APOSTROPHE, // 0x28
  eInputElements::KB_GRAVE, // 0x29
  eInputElements::KB_LSHIFT, // 0x2A
  eInputElements::KB_BACKSLASH, // 0x2B
  eInputElements::KB_Z, // 0x2C
  eInputElements::KB_X, // 0x2D
  eInputElements::KB_C, // 0x2E
  eInputElements::KB_V, // 0x2F
  eInputElements::KB_B, // 0x30
  eInputElements::KB_N, // 0x31
  eInputElements::KB_M, // 0x32
  eInputElements::KB_COMMA, // 0x33
  eInputElements::KB_PERIOD, // 0x34
  eInputElements::KB_SLASH, // 0x35
  eInputElements::KB_RSHIFT, // 0x36
  eInputElements::KB_MULTIPLY, // 0x37
  eInputElements::KB_LALT, // 0x38
  eInputElements::KB_SPACE, // 0x39
  eInputElements::KB_CAPSLOCK, // 0x3A
  eInputElements::KB_F1, // 0x3B
  eInputElements::KB_F2, // 0x3C
  eInputElements::KB_F3, // 0x3D
  eInputElements::KB_F4, // 0x3E
  eInputElements::KB_F5, // 0x3F
  eInputElements::KB_F6, // 0x40
  eInputElements::KB_F7, // 0x41
  eInputElements::KB_F8, // 0x42
  eInputElements::KB_F9, // 0x43
  eInputElements::KB_F10, // 0x44
  eInputElements::KB_PAUSE, // 0x45
  eInputElements::KB_SCROLLLOCK, // 0x46
  eInputElements::KB_NUMPAD_7, // 0x47
  eInputElements::KB_NUMPAD_8, // 0x48
  eInputElements::KB_NUMPAD_9, // 0x49
  eInputElements::KB_NUMPAD_MINUS, // 0x4A
  eInputElements::KB_NUMPAD_4, // 0x4B
  eInputElements::KB_NUMPAD_5, // 0x4C
  eInputElements::KB_NUMPAD_6, // 0x4D
  eInputElements::KB_NUMPAD_PLUS, // 0x4E
  eInputElements::KB_NUMPAD_1, // 0x4F
  eInputElements::KB_NUMPAD_2, // 0x50
  eInputElements::KB_NUMPAD_3, // 0x51
  eInputElements::KB_NUMPAD_0, // 0x52
  eInputElements::KB_NUMPAD_DELETE, // 0x53
  eInputElements::NONE, // 0x54
  eInputElements::NONE, // 0x55
  eInputElements::KB_NONUSBACKSLASH, // 0x56
  eInputElements::KB_F11, // 0x57
  eInputElements::KB_F12, // 0x58
};

const eInputElements nativeScancodeExtToDavaScancode[] =
{
  eInputElements::NONE, // 0x00
  eInputElements::NONE, // 0x01
  eInputElements::NONE, // 0x02
  eInputElements::NONE, // 0x03
  eInputElements::NONE, // 0x04
  eInputElements::NONE, // 0x05
  eInputElements::NONE, // 0x06
  eInputElements::NONE, // 0x07
  eInputElements::NONE, // 0x08
  eInputElements::NONE, // 0x09
  eInputElements::NONE, // 0x0A
  eInputElements::NONE, // 0x0B
  eInputElements::NONE, // 0x0C
  eInputElements::NONE, // 0x0D
  eInputElements::NONE, // 0x0E
  eInputElements::NONE, // 0x0F
  eInputElements::NONE, // 0x10
  eInputElements::NONE, // 0x11
  eInputElements::NONE, // 0x12
  eInputElements::NONE, // 0x13
  eInputElements::NONE, // 0x14
  eInputElements::NONE, // 0x15
  eInputElements::NONE, // 0x16
  eInputElements::NONE, // 0x17
  eInputElements::NONE, // 0x18
  eInputElements::NONE, // 0x19
  eInputElements::NONE, // 0x1A
  eInputElements::NONE, // 0x1B
  eInputElements::KB_NUMPAD_ENTER, // 0x1C
  eInputElements::KB_RCTRL, // 0x1D
  eInputElements::NONE, // 0x1E
  eInputElements::NONE, // 0x1F
  eInputElements::NONE, // 0x20
  eInputElements::NONE, // 0x21
  eInputElements::NONE, // 0x22
  eInputElements::NONE, // 0x23
  eInputElements::NONE, // 0x24
  eInputElements::NONE, // 0x25
  eInputElements::NONE, // 0x26
  eInputElements::NONE, // 0x27
  eInputElements::NONE, // 0x28
  eInputElements::NONE, // 0x29
  eInputElements::NONE, // 0x2A
  eInputElements::NONE, // 0x2B
  eInputElements::NONE, // 0x2C
  eInputElements::NONE, // 0x2D
  eInputElements::NONE, // 0x2E
  eInputElements::NONE, // 0x2F
  eInputElements::NONE, // 0x30
  eInputElements::NONE, // 0x31
  eInputElements::NONE, // 0x32
  eInputElements::NONE, // 0x33
  eInputElements::NONE, // 0x34
  eInputElements::KB_DIVIDE, // 0x35
  eInputElements::NONE, // 0x36
  eInputElements::KB_PRINTSCREEN, // 0x37
  eInputElements::KB_RALT, // 0x38
  eInputElements::NONE, // 0x39
  eInputElements::NONE, // 0x3A
  eInputElements::NONE, // 0x3B
  eInputElements::NONE, // 0x3C
  eInputElements::NONE, // 0x3D
  eInputElements::NONE, // 0x3E
  eInputElements::NONE, // 0x3F
  eInputElements::NONE, // 0x40
  eInputElements::NONE, // 0x41
  eInputElements::NONE, // 0x42
  eInputElements::NONE, // 0x43
  eInputElements::NONE, // 0x44
  eInputElements::KB_NUMLOCK, // 0x45
  eInputElements::NONE, // 0x46
  eInputElements::KB_HOME, // 0x47
  eInputElements::KB_UP, // 0x48
  eInputElements::KB_PAGEUP, // 0x49
  eInputElements::NONE, // 0x4A
  eInputElements::KB_LEFT, // 0x4B
  eInputElements::NONE, // 0x4C
  eInputElements::KB_RIGHT, // 0x4D
  eInputElements::NONE, // 0x4E
  eInputElements::KB_END, // 0x4F
  eInputElements::KB_DOWN, // 0x50
  eInputElements::KB_PAGEDOWN, // 0x51
  eInputElements::KB_INSERT, // 0x52
  eInputElements::KB_DELETE, // 0x53
  eInputElements::NONE, // 0x54
  eInputElements::NONE, // 0x55
  eInputElements::NONE, // 0x56
  eInputElements::NONE, // 0x57
  eInputElements::NONE, // 0x58
  eInputElements::NONE, // 0x59
  eInputElements::NONE, // 0x5A
  eInputElements::KB_LWIN, // 0x5B
  eInputElements::KB_RWIN, // 0x5C
  eInputElements::KB_MENU, // 0x5D
};
}
}