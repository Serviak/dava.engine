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


#ifndef __QUICKED_INTERFACES_H__
#define __QUICKED_INTERFACES_H__

#include "Base/BaseTypes.h"

namespace DAVA{
    class UIEvent;
}

class ControlNode;

class InputInterface
{
public:
    virtual bool OnInput(DAVA::UIEvent *currentInput) = 0;
};

class SelectionInterface
{
public:
    using SelectedControls = DAVA::Set < ControlNode* >;
    virtual void SelectionWasChanged(const SelectedControls &selected, const SelectedControls &deselected) = 0;
};

class ControlAreaInterface
{
public:
    enum eArea
    {
        TOP_LEFT,
        TOP_CENTER,
        TOP_RIGHT,
        CENTER_LEFT,
        CENTER_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT,
        FRAME,
        PIVOT_POINT,
        ROTATE,
        NO_AREA
    };
    static const int CORNER_COUNT = FRAME - TOP_LEFT;
    static const int AREAS_COuNT = NO_AREA - TOP_LEFT;
    virtual void MouseEnterArea(ControlNode *targetNode, const eArea area) = 0;
    virtual void MouseLeaveArea() = 0;
};

#endif // __QUICKED_INTERFACES_H__
