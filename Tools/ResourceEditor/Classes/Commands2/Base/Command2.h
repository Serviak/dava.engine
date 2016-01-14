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


#ifndef __COMMAND2_H__
#define __COMMAND2_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Entity.h"

#include "Commands2/CommandID.h"
#include "Commands2/Base/CommandNotify.h"

class Command2 : public CommandNotifyProvider
{
public:
    Command2(DAVA::int32 id, const DAVA::String& text = "");
    ~Command2() override;

    DAVA::int32 GetId() const;
    const DAVA::String & GetText() const;

    virtual void Undo() = 0;
    virtual void Redo() = 0;

    DAVA_DEPRECATED(virtual DAVA::Entity* GetEntity() const = 0);

    virtual bool MatchCommandID(DAVA::int32 commandID) const;
    virtual bool MatchCommandIDs(const DAVA::Vector<DAVA::int32>& commandIDVector) const;

protected:
    void UndoInternalCommand(Command2* command);
    void RedoInternalCommand(Command2* command);

    DAVA::String text;
    DAVA::int32 id;
};


inline DAVA::int32 Command2::GetId() const
{
    return id;
}

inline const DAVA::String & Command2::GetText() const
{
    return text;
}


#endif // __COMMAND2_H__
