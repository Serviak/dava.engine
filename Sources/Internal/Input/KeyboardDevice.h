/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_KEYBOARD_DEVICE_H__
#define __DAVAENGINE_KEYBOARD_DEVICE_H__

#include "Base/BaseObject.h"

#ifdef DELETE
#undef DELETE
#endif

/**
	\defgroup inputsystem	Input System
*/
namespace DAVA
{
enum class Key : uint32
{
    UNKNOWN = 0,
    ESCAPE,
    BACKSPACE,
    TAB,
    ENTER,
    SPACE,
    LSHIFT,
    LCTRL,
    LALT,
    RSHIFT,
    RCTRL,
    RALT,

    LWIN,
    RWIN,
    APPS, // https://en.wikipedia.org/wiki/Menu_key

    PAUSE,
    CAPSLOCK,
    NUMLOCK,
    SCROLLLOCK,

    PGUP,
    PGDN,
    HOME,
    END,
    INSERT,
    DELETE,

    LEFT,
    UP,
    RIGHT,
    DOWN,

    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    GRAVE,
    MINUS,
    EQUALS,
    BACKSLASH,
    LBRACKET,
    RBRACKET,
    SEMICOLON,
    APOSTROPHE,
    COMMA,
    PERIOD,
    SLASH,

    NUMPAD0,
    NUMPAD1,
    NUMPAD2,
    NUMPAD3,
    NUMPAD4,
    NUMPAD5,
    NUMPAD6,
    NUMPAD7,
    NUMPAD8,
    NUMPAD9,

    MULTIPLY,
    DIVIDE,
    ADD,
    SUBTRACT,
    DECIMAL,
    NUMPADENTER,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,

    //Android keys
    BACK,
    MENU,

    // exist on some keyboards
    NON_US_BACKSLASH,
    PRINTSCREEN,

    TOTAL_KEYS_COUNT
};

class KeyboardDevice : public BaseObject
{
public:
    bool IsKeyPressed(Key key) const; // during frame
    static const String& GetKeyName(Key key);

private:
    friend class InputSystem;
    friend class CoreWin32Platform;
    friend class CorePlatformAndroid;
#ifdef __DAVAENGINE_WIN_UAP__
    friend ref class WinUAPXamlApp;
#endif
    ~KeyboardDevice();
    KeyboardDevice();

#ifdef __DAVAENGINE_MACOS__
public:
#endif
    Key GetDavaKeyForSystemKey(uint32 systemKeyCode) const;
    void OnKeyPressed(Key keyCode);
    void OnKeyUnpressed(Key keyCode);

#ifdef __DAVAENGINE_MACOS__
private:
#endif
    void OnFinishFrame();
    void ClearAllKeys();

    void PrepareKeyTranslator();

    Bitset<static_cast<size_t>(Key::TOTAL_KEYS_COUNT)> currentFrameKeyStatus;
    Bitset<static_cast<size_t>(Key::TOTAL_KEYS_COUNT)> realKeyStatus;
    static const int MAX_KEYS = 512;
    Array<Key, MAX_KEYS> keyTranslator;
};

}; // end DAVA namespace

#endif
