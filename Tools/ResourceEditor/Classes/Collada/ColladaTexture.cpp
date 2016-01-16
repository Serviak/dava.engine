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


#include "stdafx.h"
#include <wchar.h>
#include "ColladaTexture.h"
#include "FileSystem/Logger.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "FileSystem/FileSystem.h"
#include "Utils/Utils.h"
#include "Qt/Main/QtUtils.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

namespace DAVA
{
ColladaTexture::ColladaTexture(FCDImage* _image)
    : texturePathName(_image->GetFilename())
{
    image = _image;

    bool pathApplied = false;

    const FilePath texturePath(texturePathName);
    if (FileSystem::Instance()->Exists(texturePath))
    {
        const String extension = texturePath.GetExtension();
        if (!extension.empty())
        {
            auto imageFormat = ImageSystem::Instance()->GetImageFormatForExtension(texturePath.GetExtension());
            if (imageFormat != IMAGE_FORMAT_UNKNOWN)
            {
                TextureDescriptorUtils::CreateDescriptorIfNeed(texturePath);
                pathApplied = true;
            }
        }
	}

    if (!pathApplied)
    {
        texturePathName.clear();
    }
}

ColladaTexture::~ColladaTexture()
{
}

bool ColladaTexture::PreLoad()
{
    return true;
}
};
