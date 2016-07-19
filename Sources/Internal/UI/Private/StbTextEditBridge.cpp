#include "StbTextEditBridge.h"
#include "Debug/DVAssert.h"
#include "Utils/TextBox.h"

#if __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

#define STB_TEXTEDIT_CHARTYPE DAVA::WideString::value_type
#define STB_TEXTEDIT_STRING DAVA::StbTextEditBridge
#define STB_TEXTEDIT_NEWLINE L'\n'

#include <stb/stb_textedit.h>

inline void stb_layoutrow(StbTexteditRow* row, STB_TEXTEDIT_STRING* str, int start_i)
{
    const DAVA::TextBox* tb = str->GetDelegate()->GetTextBox();
    if (DAVA::uint32(start_i) >= tb->GetCharactersCount())
    {
        return;
    }

    const DAVA::TextBox::Character& ch = tb->GetCharacter(start_i);
    const DAVA::TextBox::Line& line = tb->GetLine(ch.lineIndex);

    row->num_chars = line.length;
    row->x0 = line.xoffset;
    row->x1 = line.xoffset + line.xadvance;
    row->baseline_y_delta = 0.f;
    row->ymin = line.yoffset;
    row->ymax = line.yoffset + line.yadvance;

    // If single line mode enabled then extend height for first/last
    // lines to size of text field
    if (str->IsSingleLineMode())
    {
        if (ch.lineIndex == 0)
        {
            row->ymin = std::numeric_limits<float>::lowest();
        }
        if (ch.lineIndex == tb->GetLinesCount() - 1)
        {
            row->ymax = std::numeric_limits<float>::max();
        }
    }
}

inline void stb_layoutchar(STB_TEXTEDIT_STRING* str, int n, int i, float* x0, float* x1)
{
    const DAVA::int32 li = n + i;
    const DAVA::TextBox* tb = str->GetDelegate()->GetTextBox();
    if (DAVA::uint32(li) >= tb->GetCharactersCount())
    {
        return;
    }

    const DAVA::TextBox::Character& ch = tb->GetCharacter(li);
    const DAVA::TextBox::Line& line = tb->GetLine(ch.lineIndex);
    if (x0 != nullptr)
    {
        *x0 = ch.xoffset + line.xoffset;
    }
    if (x1 != nullptr)
    {
        *x1 = ch.xoffset + ch.xadvance + line.xoffset;
    }
}

inline float stb_getwidth(STB_TEXTEDIT_STRING* str, int n, int i)
{
    const DAVA::TextBox* tb = str->GetDelegate()->GetTextBox();
    DAVA::int32 li = n + i;
    if (tb->GetCharactersCount() > DAVA::uint32(li))
    {
        DAVA::int32 vi = tb->GetCharacter(li).visualIndex;
        return tb->GetCharacter(vi).xadvance;
    }
    return 0.f;
}

inline int stb_insertchars(STB_TEXTEDIT_STRING* str, int pos, STB_TEXTEDIT_CHARTYPE* newtext, int num)
{
    return int(str->GetDelegate()->InsertText(DAVA::uint32(pos), newtext, DAVA::uint32(num)));
}

inline int stb_deletechars(STB_TEXTEDIT_STRING* str, int pos, int num)
{
    return int(str->GetDelegate()->DeleteText(DAVA::uint32(pos), DAVA::uint32(num)));
}

inline int stb_stringlen(STB_TEXTEDIT_STRING* str)
{
    return int(str->GetDelegate()->GetTextLength());
}

inline int stb_keytotext(int key)
{
    return key;
}

inline STB_TEXTEDIT_CHARTYPE stb_getchar(STB_TEXTEDIT_STRING* str, int i)
{
    return str->GetDelegate()->GetCharAt(DAVA::uint32(i));
}

inline int stb_isspace(STB_TEXTEDIT_CHARTYPE ch)
{
    return iswspace(ch) || iswpunct(ch);
}

#define STB_TEXTEDIT_LAYOUTROW stb_layoutrow
#define STB_DAVA_TEXTEDIT_LAYOUTCHAR stb_layoutchar
#define STB_TEXTEDIT_INSERTCHARS stb_insertchars
#define STB_TEXTEDIT_DELETECHARS stb_deletechars
#define STB_TEXTEDIT_STRINGLEN stb_stringlen
#define STB_TEXTEDIT_GETWIDTH stb_getwidth
#define STB_TEXTEDIT_KEYTOTEXT stb_keytotext
#define STB_TEXTEDIT_GETCHAR stb_getchar
#define STB_TEXTEDIT_IS_SPACE stb_isspace

#define STB_TEXTEDIT_K_SHIFT DAVA::StbTextEditBridge::KEY_SHIFT_MASK
#define STB_TEXTEDIT_K_LEFT DAVA::StbTextEditBridge::KEY_LEFT
#define STB_TEXTEDIT_K_RIGHT DAVA::StbTextEditBridge::KEY_RIGHT
#define STB_TEXTEDIT_K_UP DAVA::StbTextEditBridge::KEY_UP
#define STB_TEXTEDIT_K_DOWN DAVA::StbTextEditBridge::KEY_DOWN
#define STB_TEXTEDIT_K_LINESTART DAVA::StbTextEditBridge::KEY_LINESTART
#define STB_TEXTEDIT_K_LINEEND DAVA::StbTextEditBridge::KEY_LINEEND
#define STB_TEXTEDIT_K_TEXTSTART DAVA::StbTextEditBridge::KEY_TEXTSTART
#define STB_TEXTEDIT_K_TEXTEND DAVA::StbTextEditBridge::KEY_TEXTEND
#define STB_TEXTEDIT_K_DELETE DAVA::StbTextEditBridge::KEY_DELETE
#define STB_TEXTEDIT_K_BACKSPACE DAVA::StbTextEditBridge::KEY_BACKSPACE
#define STB_TEXTEDIT_K_UNDO DAVA::StbTextEditBridge::KEY_UNDO
#define STB_TEXTEDIT_K_REDO DAVA::StbTextEditBridge::KEY_REDO
#define STB_TEXTEDIT_K_INSERT DAVA::StbTextEditBridge::KEY_INSERT
#define STB_TEXTEDIT_K_WORDLEFT DAVA::StbTextEditBridge::KEY_WORDLEFT
#define STB_TEXTEDIT_K_WORDRIGHT DAVA::StbTextEditBridge::KEY_WORDRIGHT
//#define STB_TEXTEDIT_K_PGUP
//#define STB_TEXTEDIT_K_PGDOWN

#define STB_TEXTEDIT_IMPLEMENTATION
#include <stb/stb_textedit.h>

#if __clang__
#pragma clang diagnostic pop
#endif

////////////////////////////////////////////////////////////////////////////////

namespace DAVA
{
struct StbState : public STB_TexteditState
{
};

StbTextEditBridge::StbTextEditBridge(StbTextDelegate* delegate)
    : stb_state(new StbState())
    , delegate(delegate)
{
    DVASSERT_MSG(delegate, "StbTextEditBridge must be inited with delegate!");
    stb_textedit_initialize_state(stb_state, 0);
}

StbTextEditBridge::StbTextEditBridge(const StbTextEditBridge& c)
    : stb_state(new StbState(*c.stb_state))
{
}

StbTextEditBridge::~StbTextEditBridge()
{
    SafeDelete(stb_state);
}

void StbTextEditBridge::CopyStbStateFrom(const StbTextEditBridge& c)
{
    DVASSERT(stb_state);
    DVASSERT(c.stb_state);
    Memcpy(stb_state, c.stb_state, sizeof(StbState));
}

bool StbTextEditBridge::SendKey(uint32 codePoint)
{
    return stb_textedit_key(this, stb_state, codePoint) != 0;
}

bool StbTextEditBridge::Cut()
{
    return stb_textedit_cut(this, stb_state) != 0;
}

bool StbTextEditBridge::Paste(const WideString& str)
{
    return stb_textedit_paste(this, stb_state, str.c_str(), int(str.length())) != 0;
}

void StbTextEditBridge::Click(const Vector2& point)
{
    stb_textedit_click(this, stb_state, point.x, point.y);
}

void StbTextEditBridge::Drag(const Vector2& point)
{
    stb_textedit_drag(this, stb_state, point.x, point.y);
}

uint32 StbTextEditBridge::GetSelectionStart() const
{
    return static_cast<uint32>(stb_state->select_start);
}

void StbTextEditBridge::SetSelectionStart(uint32 position) const
{
    stb_state->select_start = static_cast<int>(position);
}

uint32 StbTextEditBridge::GetSelectionEnd() const
{
    return static_cast<uint32>(stb_state->select_end);
}

void StbTextEditBridge::SetSelectionEnd(uint32 position) const
{
    stb_state->select_end = static_cast<int>(position);
}

uint32 StbTextEditBridge::GetCursorPosition() const
{
    return static_cast<uint32>(stb_state->cursor);
}

void StbTextEditBridge::SetCursorPosition(uint32 position) const
{
    stb_state->cursor = static_cast<int>(position);
}

void StbTextEditBridge::SetSingleLineMode(bool signleLine)
{
    stb_state->single_line = static_cast<unsigned char>(signleLine);
}

bool StbTextEditBridge::IsSingleLineMode() const
{
    return stb_state->single_line != 0;
}

bool StbTextEditBridge::IsInsertMode() const
{
    return stb_state->insert_mode != 0;
}

void StbTextEditBridge::ClearUndoStack()
{
    stb_state->undostate.undo_point = 0;
    stb_state->undostate.undo_char_point = 0;
    stb_state->undostate.redo_point = STB_TEXTEDIT_UNDOSTATECOUNT;
    stb_state->undostate.redo_char_point = STB_TEXTEDIT_UNDOCHARCOUNT;
}
}
