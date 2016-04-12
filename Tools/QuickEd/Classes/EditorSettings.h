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


#ifndef UIEditor_EditorSettings_h
#define UIEditor_EditorSettings_h

#include "DAVAEngine.h"

enum eBackgroundType : DAVA::int64
{
    BackgroundTexture,
    BackgroundColor
};

class EditorSettings : public DAVA::Singleton<EditorSettings>
{
public:
public:
    EditorSettings();
    virtual ~EditorSettings();

    void Save();


    DAVA::Color GetGrigColor() const;
    void SetGrigColor(const DAVA::Color& color);
    DAVA::Signal<const DAVA::Color&> GridColorChanged;

    eBackgroundType GetGridType() const;
    void SetGridType(eBackgroundType type);
    DAVA::Signal<eBackgroundType> GridTypeChanged;

    bool IsUsingAssetCache() const;
    DAVA::String GetAssetCacheIp() const;
    DAVA::String GetAssetCachePort() const;
    DAVA::String GetAssetCacheTimeoutSec() const;

protected:
    DAVA::Color GetColor(const DAVA::String& colorName, const DAVA::Color& defaultColor) const;
    void SetColor(const DAVA::String& colorName, const DAVA::Color& color);

private:
    using HashType = size_t;

    DAVA::KeyedArchive* settings;
};

#endif //UIEditor_EditorSettings_h
