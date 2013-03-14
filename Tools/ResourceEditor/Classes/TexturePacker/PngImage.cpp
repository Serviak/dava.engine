/*
 *  PngImage.cpp
 *  texturepack
 *
 *  Created by Vitaliy Borodovsky on 10/28/08.
 *  Copyright 2008 DAVA Consulting, LLC. All rights reserved.
 *
 */

#include "PngImage.h"
#include "CommandLineParser.h"

PngImageExt::PngImageExt()
:	internalData(0)
{
		
}

PngImageExt::~PngImageExt()
{
    SafeRelease(internalData);
}

bool PngImageExt::Read(const String & filename)
{
    SafeRelease(internalData);
    
    Vector<Image *> imageSet = ImageLoader::CreateFromFile(filename);
    if(imageSet.size() == 1)
    {
        internalData = imageSet[0];
        return true;
    }
    
    if (CommandLineParser::Instance()->GetVerbose())
    {
        Logger::Error("[PngImageExt::Read] failed to open png file: %s", filename.c_str());
    }

    for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
	return false;
}

void PngImageExt::Write(const String & filename)
{
    DVASSERT(internalData);
    ImageLoader::Save(internalData, filename);
}

bool PngImageExt::Create(uint32 width, uint32 height)
{
    SafeRelease(internalData);
    
    internalData = Image::Create(width, height, FORMAT_RGBA8888);
	return (internalData != 0);
}

void PngImageExt::DrawImage(int sx, int sy, PngImageExt * image, const Rect2i & srcRect)
{
	uint32 * destData32 = (uint32*)GetData();
	uint32 * srcData32 = (uint32*)image->GetData();
	
	int rx, ry;
	ry = sy;
	for (int y = srcRect.y; y < srcRect.y + srcRect.dy; ++y)
	{
		rx = sx;
		for (int x = srcRect.x; x < srcRect.x + srcRect.dx; ++x)
		{
			if ((rx) < 0)continue;
			if ((rx) >= GetWidth())continue;
			if ((ry) < 0)continue;
			if ((ry) >= GetHeight())continue;
			if (x < 0)continue;
			if (x >= image->GetWidth())continue;
			if (y < 0)continue;
			if (y >= image->GetHeight())continue;
			
			destData32[(rx) + (ry) * GetWidth()] = srcData32[x + y * image->GetWidth()];
			//printf("%04x ", srcData32[x + y * image->width]);
			rx++;
		}
		ry++;
	}
}

void PngImageExt::DrawImage(int sx, int sy, PngImageExt * image)
{
	// printf("0x%08x 0x%08x %d %d\n", data, image->data, sx, sy);
	
	uint32 * destData32 = (uint32*)GetData();
	uint32 * srcData32 = (uint32*)image->GetData();
	
	for (int y = 0; y < image->GetHeight(); ++y)
		for (int x = 0; x < image->GetWidth(); ++x)
		{
			if ((sx + x) < 0)continue;
			if ((sx + x) >= GetWidth())continue;
			if ((sy + y) < 0)continue;
			if ((sy + y) >= GetHeight())continue;
			
			uint32 srcRead  = srcData32[x + y * image->GetWidth()];
			destData32[(sx + x) + (sy + y) * GetWidth()] = srcRead;
			//printf("%04x ", srcData32[x + y * image->width]);
		}
}

bool PngImageExt::IsHorzLineOpaque(int y)
{
	uint8 * line = GetData() + y * GetWidth() * 4;
	for (int x = 0; x < GetWidth(); ++x)
		if (line[x * 4 + 3] != 0)
			return false;
	return true;
}

bool PngImageExt::IsVertLineOpaque(int x)
{
	uint8 * vertLine = GetData() + x * 4;
	for (int x = 0; x < GetHeight(); ++x)
	{
		if (vertLine[3] != 0)
			return false;
		
		vertLine += GetWidth() * 4;
	}
	return true;
}

void PngImageExt::FindNonOpaqueRect(Rect2i & rect)
{
	rect = Rect2i(0, 0, GetWidth(), GetHeight());
	for (int y = 0; y < GetHeight(); ++y)
		if (IsHorzLineOpaque(y))
		{
			rect.y++;
			rect.dy--;
		}else break;
	
	for (int x = 0; x < GetWidth(); ++x)
		if (IsVertLineOpaque(x))
		{
			rect.x++;
			rect.dx--;
		}else break;
		
	if ((rect.dx == 0) && (rect.dy == 0))
	{
		rect.x = rect.y = 0; 
		rect.dx = rect.dy = 1;
		return;
	}
	
	for (int y = GetHeight() - 1; y >= 0; --y)
		if (IsHorzLineOpaque(y))rect.dy--;
		else break;
	
	for (int x = GetWidth() - 1; x >= 0; --x)
		if (IsVertLineOpaque(x))rect.dx--;
		else break;
}

void PngImageExt::DrawRect(const Rect2i & rect, uint32 color)
{
	uint32 * destData32 = (uint32*)GetData();
	
	for (int i = 0; i < rect.dx; ++i)
	{
		destData32[rect.y * GetWidth() + rect.x + i] = color;
		destData32[(rect.y + rect.dy - 1) * GetWidth() + rect.x + i] = color;
	}
	for (int i = 0; i < rect.dy; ++i)
	{
		destData32[(rect.y + i) * GetWidth() + rect.x] = color;
		destData32[(rect.y + i) * GetWidth() + rect.x + rect.dx - 1] = color;
	}
}


void PngImageExt::DitherAlpha()
{
    Image *image = Image::Create(GetWidth(), GetHeight(), FORMAT_RGBA8888);
    
    uint8 *ditheredPtr = image->GetData();
    uint8 *dataPtr = GetData();

    for(int32 y = 0; y < GetHeight(); ++y)
    {
        for(int32 x = 0; x < GetWidth(); ++x)
        {
            if(dataPtr[3])
            {
                Memcpy(ditheredPtr, dataPtr, 4);
            }
            else
            {
                Color color = GetDitheredColorForPoint(x, y);
                
                ditheredPtr[0] = (uint8)color.r;
                ditheredPtr[1] = (uint8)color.g;
                ditheredPtr[2] = (uint8)color.b;
                ditheredPtr[3] = 0;
            }
            
            ditheredPtr += 4;
            dataPtr += 4;
        }
    }
    
    SafeRelease(internalData);
    internalData = image;
}

Color PngImageExt::GetDitheredColorForPoint(int32 x, int32 y)
{
    int32 count = 0;
    Color newColor(0, 0, 0, 0);
    
    int32 startY = Max(y - 1, 0);
    int32 endY = Min(y + 1, (int32)GetHeight());
    int32 startX = Max(x - 1, 0);
    int32 endX = Min(x + 1, (int32)GetWidth());
    
    for (int32 alphaY = startY; alphaY < endY; ++alphaY)
    {
        for (int32 alphaX = startX; alphaX < endX; ++alphaX)
        {
            int32 offset = (y * GetWidth() + x)*4;
            if(GetData()[offset + 3])
            {
                ++count;
                newColor.r += (float32)GetData()[offset];
                newColor.g += (float32)GetData()[offset + 1];
                newColor.b += (float32)GetData()[offset + 2];
            }
        }
    }
    
    if(count)
    {
        newColor /= (float32)count;
    }
    
    return newColor;
}


