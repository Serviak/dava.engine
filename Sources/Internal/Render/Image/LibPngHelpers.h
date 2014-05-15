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


#ifndef __DAVAENGINE_LIBPNG_HELPERS_H__
#define __DAVAENGINE_LIBPNG_HELPERS_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"

#include "Render/Image/Image.h"
#include "Render/Image/ImageFormatInterface.h"

namespace DAVA 
{

class Texture;
class Sprite;
class Image;

class LibPngWrapper: public ImageFormatInterface
{
public:
    
    LibPngWrapper();
    
    virtual bool IsImage(File *file) const;
    
    virtual eErrorCode ReadFile(File *infile, Vector<Image *> &imageSet, int32 baseMipMap = 0) const;
    
    virtual eErrorCode WriteFile(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat) const;
    
    virtual eErrorCode WriteFileAsCubeMap(const FilePath & fileName, const Vector<Image *> &imageSet, PixelFormat compressionFormat) const;

    virtual uint32 GetDataSize(File *infile) const;

protected:
    
  	static int ReadPngFile(File *infile, Image * image);

};

class PngImage : public BaseObject
{
protected:
	~PngImage();
public:
	PngImage();
	
	bool Create(int32 _width, int32 _height);
	bool CreateFromFBOSprite(Sprite * fboSprite);
		
	void DrawImage(int sx, int sy, PngImage * image);
	void DrawRect(const Rect2i & rect, uint32 color);

	uint8 * GetData() { return data; };
	int32 GetWidth() { return width; };
	int32 GetHeight() { return height; }; 
private:	
	int32		width;
	int32		height;
	uint8  *	data;
    PixelFormat format;
};
};

#endif // __PNG_IMAGE_H__
