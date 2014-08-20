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


#include "Render/2D/FTFont.h"
#include "Render/2D/FontManager.h"
#include "FileSystem/Logger.h"
#include "Utils/Utils.h"
#include "Debug/DVAssert.h"
#include "FileSystem/File.h"
#include "Core/Core.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/FilePath.h"

#include <ft2build.h>
#include <freetype/ftglyph.h>
#include FT_FREETYPE_H


namespace DAVA
{
#ifdef USE_FILEPATH_IN_MAP
	typedef Map<FilePath, FTInternalFont *> FontMap;
#else //#ifdef USE_FILEPATH_IN_MAP
	typedef Map<String, FTInternalFont *> FontMap;
#endif //#ifdef USE_FILEPATH_IN_MAP
	FontMap fontMap;

class FTInternalFont : public BaseObject
{
	friend class FTFont;
	FilePath fontPath;
	FT_StreamRec streamFont;
	File * fontFile;

private:
	FTInternalFont(const FilePath & path);
	virtual ~FTInternalFont();

public:
	FT_Face face;
	Font::StringMetrics DrawString(const WideString& str, void * buffer, int32 bufWidth, int32 bufHeight, 
		uint8 r, uint8 g, uint8 b, uint8 a, 
		float32 size, bool realDraw, 
		int32 offsetX, int32 offsetY,
		int32 justifyWidth, int32 spaceAddon,
		Vector<float32> *charSizes = NULL,
		bool contentScaleIncluded = false);
	uint32 GetFontHeight(float32 size) const;

	bool IsCharAvaliable(char16 ch) const;

	virtual int32 Release();

private:
	void SetFTCharSize(float32 size) const;

	static Mutex drawStringMutex;

	struct Glyph
	{
        Glyph(): index(0), image(0), delta(0) {};
        
        bool operator < (const Glyph& right) const { return image < right.image; };

        
		FT_UInt		index;
		FT_Glyph	image;    /* the glyph image */

		FT_Pos		delta;    /* delta caused by hinting */
	};
	Vector<Glyph> glyphs;

	void ClearString();
	int32 LoadString(const WideString& str);
	void Prepare(FT_Vector * advances);

	inline FT_Pos Round(FT_Pos val);
	
	static unsigned long StreamLoad(FT_Stream stream, unsigned long offset, uint8* buffer, unsigned long count);
	static void StreamClose(FT_Stream stream);
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

FTFont * FTFont::Create(const FilePath& path)
{
	FTInternalFont * iFont = 0;

	FontMap::iterator it = fontMap.find(path);
	if (it != fontMap.end())
	{
		iFont = it->second;
	}
	
	if(!iFont)
	{	
		iFont = new FTInternalFont(path);
        if( !iFont->face )
        {
            SafeRelease(iFont);
            return NULL;
        }

		fontMap[FILEPATH_MAP_KEY(path)] = iFont;
	}
	
	FTFont * font = new FTFont(iFont);
	font->fontPath = path;
	
	return font;
}

void FTFont::ClearCache()
{
	while (fontMap.size())
	{
		SafeRelease(fontMap.begin()->second);
		fontMap.erase(fontMap.begin());
	}
}

	
FTFont *	FTFont::Clone() const
{
	FTFont *retFont = new FTFont(internalFont);
	retFont->size =	size;
    retFont->renderSize = renderSize;

	retFont->verticalSpacing =	verticalSpacing;

	retFont->fontPath = fontPath;
	
	return retFont;
}

bool FTFont::IsEqual(const Font *font) const
{
	if (font->GetFontType() != this->GetFontType())
	{
		return false;
	}

    const FTFont *ftfont = DynamicTypeCheck<const FTFont *>(font);
	if (!Font::IsEqual(font) || internalFont != ftfont->internalFont)
	{
		return false;
	}

	return true;
}

String FTFont::GetRawHashString()
{
	return fontPath.GetFrameworkPath() + "_" + Font::GetRawHashString();
}

Font::StringMetrics FTFont::DrawStringToBuffer(void * buffer, int32 bufWidth, int32 bufHeight, int32 offsetX, int32 offsetY, int32 justifyWidth, int32 spaceAddon, const WideString& str, bool contentScaleIncluded )
{
	return internalFont->DrawString(str, buffer, bufWidth, bufHeight, 255, 255, 255, 255, renderSize, true, offsetX, offsetY, justifyWidth, spaceAddon, NULL, contentScaleIncluded );
}

Font::StringMetrics FTFont::GetStringMetrics(const WideString& str, Vector<float32> *charSizes) const
{
	return internalFont->DrawString(str, 0, 0, 0, 0, 0, 0, 0, renderSize, false, 0, 0, 0, 0, charSizes);
}

uint32 FTFont::GetFontHeight() const
{
	return internalFont->GetFontHeight(renderSize);
}


bool FTFont::IsCharAvaliable(char16 ch) const
{
	return internalFont->IsCharAvaliable(ch);
}

const FilePath & FTFont::GetFontPath() const
{
	return internalFont->fontPath;
}

YamlNode * FTFont::SaveToYamlNode() const
{
	YamlNode *node = Font::SaveToYamlNode();
	//Type
	node->Set("type", "FTFont");

    String pathname = internalFont->fontPath.GetFrameworkPath();
	node->Set("name", pathname);

	return node;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
FTInternalFont::FTInternalFont(const FilePath & path)
:	face(NULL),
	fontPath(path),
    streamFont(),
	fontFile(NULL)
{
    FilePath pathName(path);
    pathName.ReplaceDirectory(path.GetDirectory() + (LocalizationSystem::Instance()->GetCurrentLocale() + "/"));
    
    fontFile = File::Create(pathName, File::READ|File::OPEN);
    if (!fontFile)
    {    
        fontFile = File::Create(path, File::READ|File::OPEN);
        if (!fontFile)
        {
            Logger::Error("Failed to open font: %s", path.GetStringValue().c_str());
            return;
        }
    }

	Memset(&streamFont, 0, sizeof(FT_StreamRec));
	streamFont.descriptor.pointer = (void*)fontFile;
	streamFont.size = fontFile->GetSize();
	streamFont.read = &StreamLoad;
	streamFont.close = &StreamClose;
	
	FT_Open_Args args = {0};
	args.flags = FT_OPEN_STREAM;
	args.stream = &streamFont;
	
	FT_Error error = FT_Open_Face(FontManager::Instance()->GetFTLibrary(), &args, 0, &face);
	if(error == FT_Err_Unknown_File_Format)
	{
		Logger::Error("FTInternalFont::FTInternalFont FT_Err_Unknown_File_Format: %s", fontFile->GetFilename().GetStringValue().c_str());
	}
	else if(error)
	{
		Logger::Error("FTInternalFont::FTInternalFont cannot create font(no file?): %s", fontFile->GetFilename().GetStringValue().c_str());
	}
}
	
FTInternalFont::~FTInternalFont()
{
	ClearString();

	FT_Done_Face(face);
	SafeRelease(fontFile);
}


int32 FTInternalFont::Release()
{
// 	if(1 == GetRetainCount())
// 	{
// 		fontMap.erase(fontPath.GetAbsolutePathname());
// 	}
	
	return BaseObject::Release();
}

Mutex FTInternalFont::drawStringMutex;

Font::StringMetrics FTInternalFont::DrawString(const WideString& str, void * buffer, int32 bufWidth, int32 bufHeight, 
					uint8 r, uint8 g, uint8 b, uint8 a,  
					float32 size, bool realDraw, 
					int32 offsetX, int32 offsetY,
					int32 justifyWidth, int32 spaceAddon,
					Vector<float32> *charSizes,
					bool contentScaleIncluded )
{
	drawStringMutex.Lock();

    SetFTCharSize(size);
    
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
	int32 faceBboxXMin = FT_MulFix(face->bbox.xMin, face->size->metrics.y_scale);
	
	if(!contentScaleIncluded) 
	{
		bufWidth = (int32)(virtualToPhysicalFactor * bufWidth);
		bufHeight = (int32)(virtualToPhysicalFactor * bufHeight);
		offsetY = (int32)(virtualToPhysicalFactor * offsetY);
		offsetX = (int32)(virtualToPhysicalFactor * offsetX);
	}

	FT_Vector pen;
	pen.x = offsetX<<6;
	//pen.x -= (FT_Pos)(virtualToPhysicalFactor*faceBboxXMin);
	pen.y = offsetY<<6;
	pen.y -= (FT_Pos)(virtualToPhysicalFactor*faceBboxYMin);//bring baseline up
	

	uint8 * resultBuf = (uint8*)buffer;

	int32 countSpace = LoadString(str);
	int32 strLen = str.length();
	FT_Vector * advances = new FT_Vector[strLen];
	Prepare(advances);

    float32 bboxSize = ceilf(((float32)(faceBboxYMax-faceBboxYMin))/64.f);
	int32 baseSize = (int32)ceilf(bboxSize * virtualToPhysicalFactor); 
	int32 multilineOffsetY = baseSize + offsetY*2;

	int32 lastRight = 0; //charSizes helper
    int32 justifyOffset = 0;
    int32 fixJustifyOffset = 0;
    if (countSpace > 0 && justifyWidth > 0 && spaceAddon > 0)
    {
        int32 diff= justifyWidth - spaceAddon;
        justifyOffset = diff / countSpace;
        fixJustifyOffset = diff - justifyOffset*countSpace;
    }

	Font::StringMetrics metrics;

	metrics.baseline =  (int32)ceilf((float32)faceBboxYMax / 64.f * virtualToPhysicalFactor);
	metrics.height = baseSize;
	metrics.drawRect = Rect2i(0x7fffffff, 0x7fffffff, 0, baseSize);
	int32 layoutWidth = 0; // width in FT points
		
	for(int32 i = 0; i < strLen; ++i)
	{
		if ( i > 0 && (justifyOffset > 0 || fixJustifyOffset > 0))
		{
			if(str[i-1] == L' ')
			{
				pen.x += justifyOffset << 6;
				layoutWidth += justifyOffset << 6;
			}
			if (fixJustifyOffset > 0)
			{
				fixJustifyOffset--;
				pen.x += 1 << 6;
				layoutWidth += 1 << 6;
			}
		}
		
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

		FT_Glyph_Get_CBox(image, FT_GLYPH_BBOX_PIXELS, &bbox);

		pen.x += advances[i].x;
		pen.y += advances[i].y;

 		error = FT_Glyph_To_Bitmap(&image, FT_RENDER_MODE_NORMAL, 0, 1);
		if(!error)
		{
			FT_BitmapGlyph  bit = (FT_BitmapGlyph)image;
			FT_Bitmap * bitmap = &bit->bitmap;

			int32 left = bit->left;
			int32 top = multilineOffsetY - bit->top;
			int32 width = bitmap->width;
			int32 height = bitmap->rows;

			if(charSizes)
			{
				if(0 == width)
				{
                    if(str[i] == ' ')
                    {
						int32 spaceWidth = (int32)advances[i].x >> 6;
                        charSizes->push_back((float32)spaceWidth);
                        lastRight += spaceWidth;
                    }
                    else
                    {
                        charSizes->push_back(0);
                    }
				}
				else if(charSizes->empty())
				{
					charSizes->push_back((float32)width);
					lastRight = width;
				}
				else
				{
					int32 value = left + width - lastRight;
					lastRight += value;
					charSizes->push_back((float32)value);
				}
			}

			layoutWidth += advances[i].x;

			metrics.drawRect.x = Min(metrics.drawRect.x, left);
			metrics.drawRect.y = Min(metrics.drawRect.y, top);
			metrics.drawRect.dx = Max(metrics.drawRect.dx, left + width);
			metrics.drawRect.dy = Max(metrics.drawRect.dy, top + height);

			if(realDraw && bbox.xMin < bufWidth && bbox.yMin < bufHeight && top >= 0)
			{
				int32 realH = Min((int32)bitmap->rows, (int32)(bufHeight - top));
				int32 realW = Min((int32)bitmap->width, (int32)(bufWidth - left));
				int32 ind = top*bufWidth + left;
				DVASSERT(ind >= 0);
				uint8 * writeBuf = resultBuf + ind;
				uint8 * readBuf = bitmap->buffer;
                    
				for(int32 h = 0; h < realH; h++)
				{
					for(int32 w = 0; w < realW; w++)
					{
						*writeBuf++ += *readBuf++;
					}
					writeBuf += bufWidth-realW;
					// DF-1827 - Increment read buffer with proper value
					readBuf += (int32)bitmap->width-realW;
				}
				if(writeBuf > resultBuf + ind)
				{
					DVASSERT((writeBuf-resultBuf-(bufWidth-realW)) <= (bufWidth*bufHeight));
				}
			}
		}
		
		FT_Done_Glyph(image);
	}

	SafeDeleteArray(advances);
	drawStringMutex.Unlock();

	// Transform right/bottom edges into width/height
	metrics.drawRect.dx -= metrics.drawRect.x;
	metrics.drawRect.dy -= metrics.drawRect.y;

	// Transform width from FT points to pixels
	metrics.width = layoutWidth >> 6;

	if(!contentScaleIncluded) 
	{
		float32 physicalToVirtualFactor = Core::GetPhysicalToVirtualFactor();
		metrics.drawRect.x = (int32)floorf(metrics.drawRect.x * physicalToVirtualFactor);
		metrics.drawRect.y = (int32)floorf(metrics.drawRect.y * physicalToVirtualFactor);
		metrics.drawRect.dx = (int32)ceilf(metrics.drawRect.dx * physicalToVirtualFactor);
		metrics.drawRect.dy = (int32)ceilf(metrics.drawRect.dy * physicalToVirtualFactor);
		metrics.baseline = (int32)ceilf(metrics.baseline * physicalToVirtualFactor);
		metrics.width = (int32)ceilf(metrics.width * physicalToVirtualFactor);
		metrics.height = (int32)ceilf(metrics.height * physicalToVirtualFactor);
	}
	return metrics;
}


bool FTInternalFont::IsCharAvaliable(char16 ch) const
{
	return FT_Get_Char_Index(face, ch) != 0;
}
	

uint32 FTInternalFont::GetFontHeight(float32 size) const
{
    drawStringMutex.Lock();

	SetFTCharSize(size);
	uint32 height = (uint32)ceilf((float32)((FT_MulFix(face->bbox.yMax-face->bbox.yMin, face->size->metrics.y_scale)))/64.f);
	
    drawStringMutex.Unlock();

    return height;
}
	
void FTInternalFont::SetFTCharSize(float32 size) const
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
	FT_UInt		prevIndex   = 0;
	const bool		useKerning = (FT_HAS_KERNING(face) > 0);
	const int32		size = glyphs.size();

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
		}

		prevIndex   = glyph.index;
		prevAdvance = &advances[i];
	}
}

void FTInternalFont::ClearString()
{
    //TODO: temporary fix for
    Set<Glyph> clearedGlyphs;
    clearedGlyphs.insert(glyphs.begin(), glyphs.end());
	for(Set<Glyph>::iterator it = clearedGlyphs.begin(), endIt = clearedGlyphs.end(); it != endIt; ++it)
	{
		if(it->image)
		{
			FT_Done_Glyph(it->image);
		}
	}
    clearedGlyphs.clear();
    
//	int32 size = glyphs.size();
//	for(int32 i = 0; i < size; ++i)
//	{
//		if(glyphs[i].image)
//		{
//			FT_Done_Glyph(glyphs[i].image);
//		}
//	}

	glyphs.clear();
}

int32 FTInternalFont::LoadString(const WideString& str)
{
	ClearString();

	int32 spacesCount = 0;
	const FT_Pos prevRsbDelta = 0;
	int32 size = str.size();
	for(int32 i = 0; i < size; ++i)
	{
		if( L' ' == str[i])
		{
			spacesCount++;
		}

		Glyph glyph;
		glyph.index = FT_Get_Char_Index(face, str[i]);
        
        FT_Error loadGlyphError = 0;
        FT_Error getGlyphError = 0;
		if (!(loadGlyphError = FT_Load_Glyph( face, glyph.index, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING))  &&
			!(getGlyphError = FT_Get_Glyph(face->glyph, &glyph.image)))
		{
			//FT_Glyph_Metrics*  metrics = &face->glyph->metrics;
			if(prevRsbDelta - face->glyph->lsb_delta >= 32 )
				glyph.delta = -1 << 6;
			else if(prevRsbDelta - face->glyph->lsb_delta < -32)
				glyph.delta = 1 << 6;
			else
				glyph.delta = 0;
		}
        else
        {
#if defined(__DAVAENGINE_DEBUG__)
//            DVASSERT(false); //This situation can be unnormal. Check it
            Logger::Warning("[FTInternalFont::LoadString] loadError = %d, getGlyphError = %d, str = %s", loadGlyphError, getGlyphError, WStringToString(str).c_str());
#endif //__DAVAENGINE_DEBUG__
        }

		glyphs.push_back(glyph);
	}

#if defined(__DAVAENGINE_DEBUG__)
//    Set<Glyph> tmp;
//    tmp.insert(glyphs.begin(), glyphs.end());
//    DVASSERT(tmp.size() == glyphs.size()); //This situation can be unnormal. Check it
#endif //__DAVAENGINE_DEBUG__
    
    
	return spacesCount;
}

FT_Pos FTInternalFont::Round(FT_Pos val)
{
	return (((val) + 32) & -64);
}

unsigned long FTInternalFont::StreamLoad(FT_Stream stream, unsigned long offset, uint8* buffer, unsigned long count)
{
	File* is = reinterpret_cast<File*>(stream->descriptor.pointer);
	if (count == 0) return 0;
	is->Seek((int32)offset, File::SEEK_FROM_START);
	return is->Read(buffer, (uint32)count);
}

void FTInternalFont::StreamClose(FT_Stream stream)
{
	// Close file stream in destructor
}
	
};
