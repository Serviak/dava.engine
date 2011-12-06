/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan Petrochenko
=====================================================================================*/
#include "Render/2D/FTFont.h"
#include "Render/2D/FontManager.h"
#include "FileSystem/Logger.h"
#include "Utils/Utils.h"
#include "Debug/DVAssert.h"
#include "FileSystem/File.h"
#include "Core/Core.h"

#include <ft2build.h>
#include <freetype/ftglyph.h>
#include FT_FREETYPE_H


namespace DAVA
{

Map<String,FTInternalFont*> fontMap;

class FTInternalFont : public BaseObject
{
	friend class FTFont;
	String fontPath;
	uint8 * memoryFont;
	uint32 memoryFontSize;
private:
	FTInternalFont(const String& path);
	virtual ~FTInternalFont();

public:
	FT_Face face;
	Size2i DrawString(const WideString& str, void * buffer, int32 bufWidth, int32 bufHeight, 
		uint8 r, uint8 g, uint8 b, uint8 a, 
		float32 size, bool realDraw, 
		int32 offsetX, int32 offsetY,
		int32 justifyWidth, int32 spaceAddon,
		Vector<int32> *charSizes = NULL,
		bool contentScaleIncluded = false);
	uint32 GetFontHeight(float32 size);

	bool IsCharAvaliable(char16 ch);

	void SetFTCharSize(float32 size);

	virtual int32 Release();

private:
	static Mutex drawStringMutex;

	struct Glyph
	{
		FT_UInt		index;
		FT_Glyph	image;    /* the glyph image */

		FT_Pos		delta;    /* delta caused by hinting */
	};
	Vector<Glyph> glyphs;

	void ClearString();
	int32 LoadString(const WideString& str);
	void Prepare(FT_Vector * advances);

	inline FT_Pos Round(FT_Pos val);
};

FTFont::FTFont(FTInternalFont* _internalFont)
{
	internalFont = _internalFont;
	internalFont->Retain();
	fontType = TYPE_FT;
}

FTFont::~FTFont()
{
	SafeRelease(internalFont);
}

FTFont * FTFont::Create(const String& path)
{
	FTInternalFont * iFont = 0;
	Map<String,FTInternalFont*>::iterator it = fontMap.find(path);
	if (it != fontMap.end())
	{
		iFont = it->second;
	}
	
	if(!iFont)
	{//TODO: for now internal fonts is never released, need to be fixed later
		iFont = new FTInternalFont(path);
		fontMap[path] = iFont;
	}
	
	FTFont * font = new FTFont(iFont);
	
	return font;
}
	
FTFont *	FTFont::Clone()
{
	FTFont *retFont = new FTFont(internalFont);
	retFont->size =	size;
	retFont->SetColor(color);

	retFont->verticalSpacing =	verticalSpacing;

	return retFont;
	
}

bool FTFont::IsEqual(Font *font)
{
	if (!Font::IsEqual(font) || internalFont != ((FTFont*)font)->internalFont)
	{
		return false;
	}
	return true;
}
	
Size2i FTFont::DrawStringToBuffer(void * buffer, int32 bufWidth, int32 bufHeight, int32 offsetX, int32 offsetY, int32 justifyWidth, int32 spaceAddon, const WideString& str, bool contentScaleIncluded )
{
	uint8 r = ((uint8)(color.r * 255.0f));
	uint8 g = ((uint8)(color.g * 255.0f)); 
	uint8 b = ((uint8)(color.b * 255.0f)); 
	uint8 a = ((uint8)(color.a * 255.0f));
	return internalFont->DrawString(str, buffer, bufWidth, bufHeight, r, g, b, a, size, true, offsetX, offsetY, justifyWidth, spaceAddon, NULL, contentScaleIncluded );
}

Size2i FTFont::GetStringSize(const WideString& str, Vector<int32> *charSizes)
{
	return internalFont->DrawString(str, 0, 0, 0, 0, 0, 0, 0, size, false, 0, 0, 0, 0, charSizes);
}

uint32 FTFont::GetFontHeight()
{
	return internalFont->GetFontHeight(size);
}


bool FTFont::IsCharAvaliable(char16 ch)
{
	return internalFont->IsCharAvaliable(ch);
}

	
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	
FTInternalFont::FTInternalFont(const String& path)
:	face(0),
	fontPath(path)
{

	File * fp = File::Create(path, File::READ|File::OPEN);
	if (!fp)
	{
		Logger::Error("Failed to open font: %s", path.c_str());
		return;
	}

	memoryFontSize = fp->GetSize();
	memoryFont = new uint8[memoryFontSize];
	fp->Read(memoryFont, memoryFontSize);
	SafeRelease(fp);
	
	FT_Error error = FT_New_Memory_Face(FontManager::Instance()->GetFTLibrary(), memoryFont, memoryFontSize, 0, &face);
	if(error == FT_Err_Unknown_File_Format)
	{
		Logger::Error("FTInternalFont::FTInternalFont FT_Err_Unknown_File_Format");
	}
	else if(error)
	{
		Logger::Error("FTInternalFont::FTInternalFont cannot create font(no file?)");
	}
}
	
FTInternalFont::~FTInternalFont()
{
	FT_Done_Face(face);
	SafeDeleteArray(memoryFont);
}


int32 FTInternalFont::Release()
{
	if(1 == GetRetainCount())
	{
		fontMap.erase(fontPath);
	}
	
	return BaseObject::Release();
}

Mutex FTInternalFont::drawStringMutex;

Size2i FTInternalFont::DrawString(const WideString& str, void * buffer, int32 bufWidth, int32 bufHeight, 
					uint8 r, uint8 g, uint8 b, uint8 a,  
					float32 size, bool realDraw, 
					int32 offsetX, int32 offsetY,
					int32 justifyWidth, int32 spaceAddon,
					Vector<int32> *charSizes,
					bool contentScaleIncluded )
{
	drawStringMutex.Lock();

	FT_Error error;

	float32 virtualToPhysicalFactor = Core::GetVirtualToPhysicalFactor();

	// virtualToPhysicalFactor scaling
	{
		FT_Fixed mul = 1<<16;
		FT_Matrix matrix;
		matrix.xx = (FT_Fixed)(virtualToPhysicalFactor*mul);
		matrix.xy = 0;
		matrix.yx = 0;
		matrix.yy = (FT_Fixed)(virtualToPhysicalFactor*mul);
		FT_Set_Transform(face, &matrix, 0);
	}

	int32 faceBboxYMin = FT_MulFix(face->bbox.yMin, face->size->metrics.y_scale);
	int32 faceBboxYMax = FT_MulFix(face->bbox.yMax, face->size->metrics.y_scale);
	
	if(!contentScaleIncluded) 
	{
		bufWidth = (int32)(virtualToPhysicalFactor * bufWidth);
		bufHeight = (int32)(virtualToPhysicalFactor * bufHeight);
		offsetY = (int32)(virtualToPhysicalFactor * offsetY);
		offsetX = (int32)(virtualToPhysicalFactor * offsetX);
	}

	SetFTCharSize(size);

	FT_Vector pen;
	pen.x = offsetX<<6;
	pen.y = offsetY<<6;
	pen.y -= (FT_Pos)(virtualToPhysicalFactor*faceBboxYMin);//bring baseline up



	int16 * resultBuf = (int16*)buffer;

	LoadString(str);
	int32 strLen = str.length();
	FT_Vector * advances = new FT_Vector[strLen];
	Prepare(advances);

	int32 lastRight = 0; //charSizes helper
	//int32 justifyOffset = 0;
	int32 maxWidth = 0;
	
	for(int32 i = 0; i < strLen; ++i)
	{
		Glyph		& glyph = glyphs[i];
		FT_Glyph	image;
		FT_BBox		bbox;

		if (!glyph.image)
			continue;

		error = FT_Glyph_Copy(glyph.image, &image);
		if(error)
			continue;

		if(!error)
			error = FT_Glyph_Transform(image, 0, &pen);

		if(error)
		{
			FT_Done_Glyph( image );
			continue;
		}

		pen.x += advances[i].x;
		pen.y += advances[i].y;

		FT_Glyph_Get_CBox(image, FT_GLYPH_BBOX_PIXELS, &bbox);

		float32 bboxSize = ceilf(((float32)(faceBboxYMax-faceBboxYMin))/64.f);
		int32 baseSize = (int32)ceilf(bboxSize*virtualToPhysicalFactor); 
		int32 multilineOffsetY = baseSize+offsetY*2;
		if(!realDraw || (bbox.xMax>0 && bbox.yMax>0 && bbox.xMin<bufWidth && bbox.yMin < bufHeight))
		{
 			error = FT_Glyph_To_Bitmap(&image, FT_RENDER_MODE_NORMAL, 0, 1);
			if(!error)
			{
				FT_BitmapGlyph  bit = (FT_BitmapGlyph)image;
				FT_Bitmap * bitmap = &bit->bitmap;

				int32 left = bit->left;
				int32 top = multilineOffsetY-bit->top;
				int32 width = bitmap->width;
				//int32 height = bitmap->rows;

				if(charSizes)
				{
					if(0 == width)
					{
						charSizes->push_back(0);
					}
					else if(charSizes->empty())
					{
						charSizes->push_back(width);
						lastRight = width;
					}
					else
					{
						//int32 sizesSize = charSizes->size();
						int32 value = left+width-lastRight;
						lastRight += value;
						charSizes->push_back(value);
					}
				}

				maxWidth = Max(maxWidth, left+width);

				if(realDraw)
				{
					int32 realH = Min((int32)bitmap->rows, (int32)(bufHeight - top));
					int32 realW = Min((int32)bitmap->width, (int32)(bufWidth - left)); 
					int32 ind = top*bufWidth + left;
					DVASSERT(ind >= 0);
					int16 * writeBuf = resultBuf + ind;
					uint8 * readBuf = bitmap->buffer;
					for(int32 h = 0; h < realH; h++)
					{
						for(int32 w = 0; w < realW; w++)
						{
							int32 oldPix = *readBuf;
							uint8 preAlpha = (oldPix*a)>>8;
							if(preAlpha)
							{
								uint8 tempA = preAlpha>>4;
								uint8 tempR = (preAlpha*r)>>12; 
								uint8 tempG = (preAlpha*g)>>12;
								uint8 tempB = (preAlpha*b)>>12;
								DVASSERT(writeBuf-resultBuf <= bufWidth*bufHeight);
								*writeBuf = ((tempR<<12) | (tempG<<8) | (tempB<<4) | tempA);
							}
							++writeBuf;
							++readBuf;
						}
						writeBuf += bufWidth-realW;
					}
					if(writeBuf > resultBuf + ind)
					{
						DVASSERT((writeBuf-resultBuf-(bufWidth-realW)) <= (bufWidth*bufHeight));
					}
				}
			}
		}

		FT_Done_Glyph(image);
	}

	SafeDeleteArray(advances);
	drawStringMutex.Unlock();
	
	if(contentScaleIncluded) 
	{
		return Size2i(maxWidth, GetFontHeight(size));
	}
	else
	{
		return Size2i((int32)ceilf(Core::GetPhysicalToVirtualFactor()*(maxWidth)), GetFontHeight(size));
	}
}


bool FTInternalFont::IsCharAvaliable(char16 ch)
{
	return FT_Get_Char_Index(face, ch) != 0;
}
	

uint32 FTInternalFont::GetFontHeight(float32 size)
{
	SetFTCharSize(size);
	return (uint32)ceilf((float32)((FT_MulFix(face->bbox.yMax-face->bbox.yMin, face->size->metrics.y_scale)))/64.f);
}
	
void FTInternalFont::SetFTCharSize(float32 size)
{
	FT_Error error = FT_Set_Char_Size(face, 0, (int32)(size * 64), 0, (FT_UInt)Font::GetDPI()); 
	
	if(error) 
	{
		Logger::Error("FTInternalFont::FT_Set_Char_Size");
	}
}

void FTInternalFont::Prepare(FT_Vector * advances)
{
	FT_Vector	* prevAdvance = 0;
	FT_Vector	extent = {0, 0};
	FT_UInt		prevIndex   = 0;
	bool		useKerning = (FT_HAS_KERNING(face) > 0);
	int32		size = glyphs.size();

	for(int32 i = 0; i < size; ++i)
	{
		Glyph & glyph = glyphs[i];

		advances[i] = glyph.image->advance;
		advances[i].x >>= 10;
		advances[i].y >>= 10;

		if(prevAdvance)
		{
			//prevAdvance->x += track_kern;

			if(useKerning)
			{
				FT_Vector  kern;

				FT_Get_Kerning(face, prevIndex, glyph.index, FT_KERNING_UNFITTED, &kern );

				prevAdvance->x += kern.x;
				prevAdvance->y += kern.y;

				//if(sc->kerning_mode > KERNING_MODE_NORMAL)
					prevAdvance->x += glyph.delta;
			}

			//if(handle->hinted)
			//{
			//	prevAdvance->x = Round(prevAdvance->x);
			//	prevAdvance->y = Round(prevAdvance->y);
			//}

			extent.x += prevAdvance->x;
			extent.y += prevAdvance->y;
		}

		prevIndex   = glyph.index;
		prevAdvance = &advances[i];
	}

	if(prevAdvance)
	{
		//if(handle->hinted)
		//{
		//	prevAdvance->x = Round(prevAdvance->x);
		//	prevAdvance->y = Round(prevAdvance->y);
		//}

		extent.x += prevAdvance->x;
		extent.y += prevAdvance->y;
	}

	if(size > 0)
	{
		advances[size-1] = extent;
	}
}

void FTInternalFont::ClearString()
{
	int32 size = glyphs.size();
	for(int32 i = 0; i < size; ++i)
	{
		if(glyphs[i].image)
		{
			FT_Done_Glyph(glyphs[i].image);
		}
	}

	glyphs.clear();
}

int32 FTInternalFont::LoadString(const WideString& str)
{
	ClearString();

	int32 spacesCount = 0;
	FT_Pos prevRsbDelta = 0;
	int32 size = str.size();
	for(int32 i = 0; i < size; ++i)
	{
		if( L' ' == str[i])
		{
			spacesCount++;
		}

		Glyph glyph;
		glyph.index = FT_Get_Char_Index(face, str[i]);
		if (!FT_Load_Glyph( face, glyph.index, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING)  &&
			!FT_Get_Glyph(face->glyph, &glyph.image))
		{
			//FT_Glyph_Metrics*  metrics = &face->glyph->metrics;

			if(prevRsbDelta - face->glyph->lsb_delta >= 32 )
				glyph.delta = -1 << 6;
			else if(prevRsbDelta - face->glyph->lsb_delta < -32)
				glyph.delta = 1 << 6;
			else
				glyph.delta = 0;
		}

		glyphs.push_back(glyph);
	}

	return spacesCount;
}

FT_Pos FTInternalFont::Round(FT_Pos val)
{
	return (((val) + 32) & -64);
}

	
};
