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


#ifndef __DAVAENGINE_IMAGE_SYSTEM_H__
#define __DAVAENGINE_IMAGE_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"
#include "Render/Image/ImageFormatInterface.h"
#include "Render/Image/LibJpegHelper.h"
#include "Render/Image/LibDdsHelper.h"
#include "Render/Image/LibPngHelpers.h"
#include "Render/Image/LibPVRHelper.h"
#include "FileSystem/File.h"


namespace DAVA 
{
class Image;

    
class ImageSystem: public Singleton<ImageSystem>
{
public:
    enum eSupportedImageFileFormats
    {
        FILE_FORMAT_PNG = 0,
        FILE_FORMAT_DDS,
        FILE_FORMAT_PVR,
        FILE_FORMAT_JPEG,
        FILE_FORMAT_COUNT
    };

    ImageSystem();
    
    virtual ~ImageSystem();
    
    eErrorCode Load(const FilePath & pathname, Vector<Image *> & imageSet, int32 baseMipmap = 0) const;
    
    eErrorCode Load(File *file, Vector<Image *> & imageSet, int32 baseMipmap = 0) const;
    
    eErrorCode Save(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat = FORMAT_RGBA8888) const;
    
    eErrorCode SaveAsCubeMap(const FilePath & fileName, const Vector<Vector<Image *> > &imageSet, PixelFormat compressionFormat = FORMAT_RGBA8888) const;
    
    eErrorCode Save(const FilePath & fileName, Image *image, PixelFormat compressionFormat = FORMAT_RGBA8888) const;
    
    inline ImageFormatInterface* GetImageFormatInterface(eSupportedImageFileFormats fileFormat) const;
	ImageFormatInterface* GetImageFormatInterface(const FilePath & pathname) const;
	ImageFormatInterface* GetImageFormatInterface(File *file) const;
    
protected:
    
       
    ImageFormatInterface* wrappers[FILE_FORMAT_COUNT];
};
   
};

inline DAVA::ImageFormatInterface* DAVA::ImageSystem::GetImageFormatInterface(ImageSystem::eSupportedImageFileFormats fileFormat) const
{
    return wrappers[fileFormat];
}

#endif // __DAVAENGINE_IMAGE_SYSTEM_H__
