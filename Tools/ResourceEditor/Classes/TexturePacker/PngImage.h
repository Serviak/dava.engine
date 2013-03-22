/*
 *  PngImage.h
 *  texturepack
 *
 *  Created by Vitaliy Borodovsky on 10/28/08.
 *  Copyright 2008 DAVA Consulting, LLC. All rights reserved.
 *
 */
#ifndef __DAVAENGINE_PNGIMAGEEXT_H__
#define __DAVAENGINE_PNGIMAGEEXT_H__

#include "DAVAEngine.h"
using namespace DAVA;

class PngImageExt
{
public:
	PngImageExt();
	~PngImageExt();
	
	bool Create(uint32 width, uint32 height);
	
	bool Read(const FilePath & filename);
	void Write(const FilePath & filename);
	
	void DrawImage(int32 sx, int32 sy, PngImageExt * image);
	void DrawImage(int32 sx, int32 sy, PngImageExt * image, const Rect2i & srcRect);

	void DrawRect(const Rect2i & rect, uint32 color);
	
	void FindNonOpaqueRect(Rect2i & rect);
	
    void DitherAlpha();
    
    inline uint32 GetWidth() const;
	inline uint32 GetHeight() const;
    
private:

    inline uint8 * GetData() const;

    bool IsHorzLineOpaque(int32 y);
	bool IsVertLineOpaque(int32 x);

    Color GetDitheredColorForPoint(int32 x, int32 y);
    
    Image *internalData;
};

inline uint8 * PngImageExt::GetData() const
{
    DVASSERT(internalData);
    return internalData->GetData();
}


inline uint32 PngImageExt::GetWidth() const
{
    DVASSERT(internalData);
	return internalData->GetWidth();
}

inline uint32 PngImageExt::GetHeight() const
{
    DVASSERT(internalData);
	return internalData->GetHeight();
}



#endif // __DAVAENGINE_PNGIMAGEEXT_H__
