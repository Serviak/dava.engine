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


#ifndef __DAVAENGINE_IMAGELOADER_H__
#define __DAVAENGINE_IMAGELOADER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"
#include "Render/ImageFileWrapper.h"

namespace DAVA 
{

class File;
class Image;
class ImageLoader
{
public:

    static bool CreateFromFileByExtension(const FilePath & pathname, Vector<Image *> & imageSet, int32 baseMipmap = 0);
    
	static bool CreateFromFileByContent(const FilePath & pathname, Vector<Image *> & imageSet, int32 baseMipmap = 0);
	static bool CreateFromFileByContent(File *file, Vector<Image *> & imageSet, int32 baseMipmap = 0);

    static bool Save(const Image *image, const FilePath & pathname);
    
protected:

    static bool CreateFromPNGFile(const FilePath & pathname, Vector<Image *> & imageSet);
    static bool CreateFromJPEGFile(const FilePath & pathname, Vector<Image *> & imageSet);
	static bool CreateFromPVRFile(const FilePath & pathname, Vector<Image *> & imageSet, int32 baseMipmap = 0);
	static bool CreateFromDDSFile(const FilePath & pathname, Vector<Image *> & imageSet, int32 baseMipmap = 0);
	static bool CreateFromPNG(File *file, Vector<Image *> & imageSet);
    static bool CreateFromJPEG(File *file, Vector<Image *> & imageSet);
	static bool CreateFromPVR(File *file, Vector<Image *> & imageSet, int32 baseMipmap = 0);
	static bool CreateFromDDS(File *file, Vector<Image *> & imageSet, int32 baseMipmap = 0);
    
    static bool IsPVRFile(File *file);
    static bool IsPNGFile(File *file);
	static bool IsDDSFile(File *file);
    static bool IsJPEGFile(File *file);
};
    
class ImageFileWrapperFactory
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
	

    static ImageFileWapper* GetImageFileWrapper(eSupportedImageFileFormats);
};
	
};

#endif // __DAVAENGINE_IMAGELOADER_H__
