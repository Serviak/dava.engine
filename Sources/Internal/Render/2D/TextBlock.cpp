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


#include "Render/2D/Sprite.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Render/RenderManager.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "Render/2D/TextBlock.h"
#include "Core/Core.h"
#include "Job/JobManager.h"
#include "Job/JobWaiter.h"

#include "Render/2D/TextBlockSoftwareRender.h"
#include "Render/2D/TextBlockGraphicsRender.h"
#include "Render/2D/TextBlockDistanceRender.h"

#include "Utils/UTF8Utils.h"
#include "Utils/BiDiUtils.h"

namespace DAVA 
{

struct TextBlockData
{
    TextBlockData(): font(NULL) { };
    ~TextBlockData() { SafeRelease(font); };

    Font *font;
};




//TODO: использовать мапу	
static	Vector<TextBlock *> registredBlocks;

#define NEW_RENDER 1

void RegisterTextBlock(TextBlock *tbl)
{
    registredBlocks.push_back(tbl);
}

void UnregisterTextBlock(TextBlock *tbl)
{
    for(Vector<TextBlock *>::iterator it = registredBlocks.begin(); it != registredBlocks.end(); it++)
    {
        if (tbl == *it) 
        {
            registredBlocks.erase(it);
            return;
        }
    }
}

void TextBlock::ScreenResolutionChanged()
{
    Logger::FrameworkDebug("Regenerate text blocks");
    for(Vector<TextBlock *>::iterator it = registredBlocks.begin(); it != registredBlocks.end(); it++)
    {
        (*it)->Prepare();
    }
}

TextBlock * TextBlock::Create(const Vector2 & size)
{
    TextBlock * textSprite = new TextBlock();
    textSprite->SetRectSize(size);
    return textSprite;
}


TextBlock::TextBlock()
    : cacheFinalSize(0.f, 0.f)
    , cacheW(0)
    , cacheDx(0)
    , cacheDy(0)
    , cacheOx(0)
    , cacheOy(0)
{
    font = NULL;
    isMultilineEnabled = false;
    fittingType = FITTING_DISABLED;

    needRedraw = true;

    originalFontSize = 0.1f;
    align = ALIGN_HCENTER|ALIGN_VCENTER;
    RegisterTextBlock(this);
    isMultilineBySymbolEnabled = false;
    treatMultilineAsSingleLine = false;
    
	textBlockRender = NULL;
    textureInvalidater = NULL;
}

TextBlock::~TextBlock()
{
	SafeRelease(textBlockRender);
    SafeDelete(textureInvalidater);
	SafeRelease(font);
	UnregisterTextBlock(this);
}

// Setters // Getters

void TextBlock::SetFont(Font * _font)
{
    mutex.Lock();

    if (!_font || _font == font)
    {
        mutex.Unlock();
        return;
    }

    SafeRelease(font);
    font = SafeRetain(_font);

	originalFontSize = font->GetSize();
	
	SafeRelease(textBlockRender);
    SafeDelete(textureInvalidater);
	switch (font->GetFontType()) {
		case Font::TYPE_FT:
			textBlockRender = new TextBlockSoftwareRender(this);
            textureInvalidater = new TextBlockSoftwareTexInvalidater(this);
			break;
		case Font::TYPE_GRAPHICAL:
			textBlockRender = new TextBlockGraphicsRender(this);
			break;
		case Font::TYPE_DISTANCE:
			textBlockRender = new TextBlockDistanceRender(this);
			break;
			
		default:
			DVASSERT(!"Unknown font type");
			break;
	}
	
	needRedraw = true;

    mutex.Unlock();
    Prepare();
}

void TextBlock::SetRectSize(const Vector2 & size)
{
    mutex.Lock();
    if (rectSize != size)
    {
        rectSize = size;
        needRedraw = true;

        mutex.Unlock();
        Prepare();
        return;
    }
    mutex.Unlock();
}

void TextBlock::SetPosition(const Vector2& position)
{
    this->position = position;
}

void TextBlock::SetPivotPoint(const Vector2& pivotPoint)
{
    this->pivotPoint = pivotPoint;
}

void TextBlock::SetText(const WideString & _string, const Vector2 &requestedTextRectSize)
{
    mutex.Lock();
	if(originalText == _string && requestedSize == requestedTextRectSize)
	{
        mutex.Unlock();
		return;
	}
	requestedSize = requestedTextRectSize;
	originalText = _string;
    needRedraw = true;

    if(!BiDiUtils::TransformString(originalText, text, isRtl))
    {
        text = originalText;
        isRtl = false;
    }

    mutex.Unlock();
    Prepare();
}

void TextBlock::SetMultiline(bool _isMultilineEnabled, bool bySymbol)
{
    mutex.Lock();
    if (isMultilineEnabled != _isMultilineEnabled || isMultilineBySymbolEnabled != bySymbol)
    {
        isMultilineBySymbolEnabled = bySymbol;
        isMultilineEnabled = _isMultilineEnabled;
        needRedraw = true;

        mutex.Unlock();
        Prepare();
        return;
    }
    mutex.Unlock();
}

void TextBlock::SetFittingOption(int32 _fittingType)
{
    mutex.Lock();
    if (fittingType != _fittingType)
    {
        fittingType = _fittingType;
        needRedraw = true;

        mutex.Unlock();
        Prepare();
        return;
    }
    mutex.Unlock();
}


Font * TextBlock::GetFont()
{
    mutex.Lock();
    mutex.Unlock();

    return font;
}

const Vector<WideString> & TextBlock::GetMultilineStrings()
{
    mutex.Lock();
    mutex.Unlock();

    return multilineStrings;
}

const WideString & TextBlock::GetText()
{
    mutex.Lock();
    mutex.Unlock();

	return originalText;
}

bool TextBlock::GetMultiline()
{
    mutex.Lock();
    mutex.Unlock();

    return isMultilineEnabled;
}

bool TextBlock::GetMultilineBySymbol()
{
    mutex.Lock();
    mutex.Unlock();

    return isMultilineBySymbolEnabled;
}

int32 TextBlock::GetFittingOption()
{
    mutex.Lock();
    mutex.Unlock();

    return fittingType;
}

void TextBlock::SetAlign(int32 _align)
{
    mutex.Lock();
    if (align != _align) 
    {
        align = _align;
        needRedraw = true;

        mutex.Unlock();
        Prepare();
        return;
    }
    mutex.Unlock();
}

void TextBlock::SetUseRtlAlign(bool const& useRtlAlign)
{
    mutex.Lock();
	if(this->useRtlAlign != useRtlAlign)
	{
		this->useRtlAlign = useRtlAlign;
		needRedraw = true;
		mutex.Unlock();
		Prepare();
		return;
	}
    mutex.Unlock();
}

bool TextBlock::GetUseRtlAlign()
{
    mutex.Lock();
    mutex.Unlock();
    return useRtlAlign;
}

bool TextBlock::IsRtl()
{
    mutex.Lock();
    mutex.Unlock();
    return isRtl;
}

int32 TextBlock::GetAlign()
{
    mutex.Lock();
    mutex.Unlock();
	return align;
}
	
int32 TextBlock::GetVisualAlign()
{
	mutex.Lock();
    mutex.Unlock();
	return GetVisualAlignUnsafe();
}
	
int32 TextBlock::GetVisualAlignUnsafe() const
{
	if(useRtlAlign && isRtl && (align & ALIGN_LEFT || align & ALIGN_RIGHT))
    {
        // Mirror left/right align
        return align ^ (ALIGN_LEFT | ALIGN_RIGHT);
    }
	return align;
}

Sprite * TextBlock::GetSprite()
{
    mutex.Lock();

    Sprite* sprite = NULL;
    if (textBlockRender)
        sprite = textBlockRender->GetSprite();

    DVASSERT(sprite);
    if (!sprite) 
    {
        sprite = Sprite::CreateAsRenderTarget(8, 8, FORMAT_RGBA4444);
        Logger::Error("[Textblock] getting NULL sprite");
    }

    mutex.Unlock();

    return sprite;
}

bool TextBlock::IsSpriteReady()
{
    mutex.Lock();
    Sprite* sprite = NULL;
    if (textBlockRender)
    {
        sprite = textBlockRender->GetSprite();
    }

    mutex.Unlock();
    return sprite != NULL;
}

void TextBlock::Prepare(Texture *texture /*=NULL*/)
{
	Retain();
	ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &TextBlock::PrepareInternal,
                                                                                            SafeRetain(texture)));
}

void TextBlock::PrepareInternal(BaseObject * caller, void * param, void *callerData)
{
    Texture * texture = (Texture *)param;
	if(!font)
	{
        Release();
        return;
    }

    mutex.Lock();
    if(needRedraw)
    {
        CalculateCacheParams();

        if(textBlockRender)
        {
			textBlockRender->Prepare(texture);
        }

        needRedraw = false;
    }

    mutex.Unlock();
    SafeRelease(texture);
	Release();
}

void TextBlock::CalculateCacheParams()
{
    if(text.empty())
    {
        cacheFinalSize = Vector2(0.f,0.f);
        cacheW = 0;
        cacheDx = 0;
        cacheDy = 0;
        cacheOx = 0;
        cacheOy = 0;
        cacheSpriteOffset = Vector2(0.f,0.f);

        return;
    }

    bool useJustify = ((align & ALIGN_HJUSTIFY) != 0);
    font->SetSize(originalFontSize);
    Vector2 drawSize = rectSize;

    if(requestedSize.dx > 0)
    {
        drawSize.x = requestedSize.dx;
    }
    if(requestedSize.dy > 0)
    {
        drawSize.y = requestedSize.dy;
    }

    Font::StringMetrics textSize;
    stringSizes.clear();

    // This is a temporary fix to correctly handle long multiline texts
    // which can't be broken to the separate lines.
    if (isMultilineEnabled)
    {
        if(isMultilineBySymbolEnabled)
        {
            font->SplitTextBySymbolsToStrings(text, drawSize, multilineStrings);
        }
        else
        {
            font->SplitTextToStrings(text, drawSize, multilineStrings);
        }

        treatMultilineAsSingleLine = multilineStrings.size() == 1;
    }

    if(!isMultilineEnabled || treatMultilineAsSingleLine)
    {
        textSize = font->GetStringMetrics(text);
        pointsStr.clear();
        if(fittingType & FITTING_POINTS)
        {
            if(drawSize.x < textSize.width)
            {
                Size2i textSizePoints;

                int32 length = (int32)text.length();
                for(int32 i = length - 1; i > 0; --i)
                {
                    pointsStr.clear();
                    pointsStr.append(text, 0, i);
                    pointsStr += L"...";

                    textSize = font->GetStringMetrics(pointsStr);
                    if(textSize.width <= drawSize.x)
                    {
                        break;
                    }
                }
            }
        }
        else if(!((fittingType & FITTING_REDUCE) || (fittingType & FITTING_ENLARGE)) && (drawSize.x < textSize.width) && (requestedSize.x >= 0))
        {
            Size2i textSizePoints;
            int32 length = (int32)text.length();
            if(ALIGN_RIGHT & align)
            {
                for(int32 i = 1; i < length - 1; ++i)
                {
                    pointsStr.clear();
                    pointsStr.append(text, i, length - i);

                    textSize = font->GetStringMetrics(pointsStr);
                    if(textSize.width <= drawSize.x)
                    {
                        break;
                    }
                }
            }
            else if(ALIGN_HCENTER & align)
            {
                int32 endPos = length / 2;
                int32 startPos = endPos - 1;

                int32 count = endPos;
                WideString savedStr = L"";

                for(int32 i = 1; i < count; ++i)
                {
                    pointsStr.clear();
                    pointsStr.append(text, startPos, endPos - startPos);

                    textSize = font->GetStringMetrics(pointsStr);
                    if(drawSize.x <= textSize.width)
                    {
                        break;
                    }

                    --startPos;
                    ++endPos;
                }
            }
        }
        else if(((fittingType & FITTING_REDUCE) || (fittingType & FITTING_ENLARGE)) && (requestedSize.dy >= 0 || requestedSize.dx >= 0))
        {
            bool isChanged = false;
            float prevFontSize = font->GetRenderSize();
            while (true)
            {
                float yMul = 1.0f;
                float xMul = 1.0f;

                bool xBigger = false;
                bool xLower = false;
                bool yBigger = false;
                bool yLower = false;
                if(requestedSize.dy >= 0)
                {
                    if((isChanged || fittingType & FITTING_REDUCE) && textSize.height > drawSize.y)
                    {
                        if (prevFontSize < font->GetRenderSize())
                        {
                            font->SetRenderSize(prevFontSize);
                            textSize = font->GetStringMetrics(text);
                            break;
                        }
                        yBigger = true;
                        yMul = drawSize.y / textSize.height;
                    }
                    else if((isChanged || fittingType & FITTING_ENLARGE) && textSize.height < drawSize.y * 0.9)
                    {
                        yLower = true;
                        yMul = (drawSize.y * 0.9f) / textSize.height;
                        if(yMul < 1.01f)
                        {
                            yLower = false;
                        }
                    }
                }

                if(requestedSize.dx >= 0)
                {
                    if((isChanged || fittingType & FITTING_REDUCE) && textSize.width > drawSize.x)
                    {
                        if (prevFontSize < font->GetRenderSize())
                        {
                            font->SetRenderSize(prevFontSize);
                            textSize = font->GetStringMetrics(text);
                            break;
                        }
                        xBigger = true;
                        xMul = drawSize.x / textSize.width;
                    }
                    else if((isChanged || fittingType & FITTING_ENLARGE) && textSize.width < drawSize.x * 0.95)
                    {
                        xLower = true;
                        xMul = (drawSize.x * 0.95f) / textSize.width;
                        if(xMul < 1.01f)
                        {
                            xLower = false;
                        }
                    }
                }


                if((!xBigger && !yBigger) && (!xLower || !yLower))
                {
                    break;
                }

                float finalSize = font->GetRenderSize();
                prevFontSize = finalSize;
                isChanged = true;
                if(xMul < yMul)
                {
                    finalSize *= xMul;
                }
                else
                {
                    finalSize *= yMul;
                }
                font->SetRenderSize(finalSize);
                textSize = font->GetStringMetrics(text);
            };
        }

        if(!pointsStr.empty())
        {
            textSize = font->GetStringMetrics(pointsStr);
        }

        if (treatMultilineAsSingleLine)
        {
            // Another temporary solution to return correct multiline strings/
            // string sizes.
            multilineStrings.clear();
            stringSizes.clear();
            multilineStrings.push_back(text);
            stringSizes.push_back(textSize.width);
        }
    }
    else //if(!isMultilineEnabled)
    {
        if(fittingType && (requestedSize.dy >= 0/* || requestedSize.dx >= 0*/) && text.size() > 3)
        {
            if(isMultilineBySymbolEnabled)
                font->SplitTextBySymbolsToStrings(text, drawSize, multilineStrings);
            else
                font->SplitTextToStrings(text, drawSize, multilineStrings);

            int32 yOffset = font->GetVerticalSpacing();
            int32 fontHeight = font->GetFontHeight() + yOffset;
            float lastSize = font->GetRenderSize();

            textSize.width = 0;
            textSize.height = fontHeight * (int32)multilineStrings.size() - yOffset;

            bool isChanged = false;
            while (true)
            {
                float yMul = 1.0f;

                bool yBigger = false;
                bool yLower = false;
                if(requestedSize.dy >= 0)
                {
                    if((isChanged || fittingType & FITTING_REDUCE) && textSize.height > drawSize.y)
                    {
                        yBigger = true;
                        yMul = drawSize.y / textSize.height;
                        if(lastSize < font->GetRenderSize())
                        {
                            font->SetRenderSize(lastSize);
                            break;
                        }
                    }
                    else if((isChanged || fittingType & FITTING_ENLARGE) && textSize.height < drawSize.y * 0.95)
                    {
                        yLower = true;
                        if(textSize.height < drawSize.y * 0.75f)
                        {
                            yMul = (drawSize.y * 0.75f) / textSize.height;
                        }
                        else if(textSize.height < drawSize.y * 0.8f)
                        {
                            yMul = (drawSize.y * 0.8f) / textSize.height;
                        }
                        else if(textSize.height < drawSize.y * 0.85f)
                        {
                            yMul = (drawSize.y * 0.85f) / textSize.height;
                        }
                        else if(textSize.height < drawSize.y * 0.9f)
                        {
                            yMul = (drawSize.y * 0.9f) / textSize.height;
                        }
                        else
                        {
                            yMul = (drawSize.y * 0.95f) / textSize.height;
                        }
                        if (yMul == 1.0f)
                        {
                            yMul = 1.05f;
                        }
                    }
                }

                if(!yBigger && !yLower)
                {
                    break;
                }

                float finalSize = lastSize = font->GetRenderSize();
                isChanged = true;
                finalSize *= yMul;

                font->SetRenderSize(finalSize);

                if(isMultilineBySymbolEnabled)
                    font->SplitTextBySymbolsToStrings(text, drawSize, multilineStrings);
                else
                    font->SplitTextToStrings(text, drawSize, multilineStrings);

                yOffset = font->GetVerticalSpacing();
                fontHeight = font->GetFontHeight() + yOffset;
                textSize.height = fontHeight * (int32)multilineStrings.size() - yOffset;

            };

        }

        if(isMultilineBySymbolEnabled)
            font->SplitTextBySymbolsToStrings(text, drawSize, multilineStrings);
        else
            font->SplitTextToStrings(text, drawSize, multilineStrings);

        int32 yOffset = font->GetVerticalSpacing();
        int32 fontHeight = font->GetFontHeight() + yOffset;

        textSize.width = textSize.drawRect.dx = 0;
        textSize.height = textSize.drawRect.dy = fontHeight * (int32)multilineStrings.size() - yOffset;	

        if(textSize.height > drawSize.y)
        {
            int32 needLines = Min((int32)multilineStrings.size(), (int32)ceilf(drawSize.y / fontHeight) + 1);
            Vector<WideString> oldLines;
            multilineStrings.swap(oldLines);
            if(align & ALIGN_TOP)
            {
                multilineStrings.assign(oldLines.begin(), oldLines.begin() + needLines);
            }
            else if(align & ALIGN_VCENTER)
            {
                int32 startIndex = ((int32)oldLines.size() - needLines + 1) / 2;
                multilineStrings.assign(oldLines.begin() + startIndex, oldLines.begin() + startIndex + needLines);
            }
            else //if(ALIGN_BOTTOM)
            {
                int32 startIndex = (int32)oldLines.size() - needLines;
                multilineStrings.assign(oldLines.begin() + startIndex, oldLines.end());
            }
            textSize.height = textSize.drawRect.dy = fontHeight * (int32)multilineStrings.size() - yOffset;	
        }

        stringSizes.reserve(multilineStrings.size());
        for (int32 line = 0; line < (int32)multilineStrings.size(); ++line)
        {
            Font::StringMetrics stringSize = font->GetStringMetrics(multilineStrings[line]);
            stringSizes.push_back(stringSize.width);
            if(requestedSize.dx >= 0)
            {
                textSize.width = Max(textSize.width, Min(stringSize.width, (int)drawSize.x));
                textSize.drawRect.dx = Max(textSize.drawRect.dx, Min(stringSize.drawRect.dx, (int)drawSize.x));
            }
            else
            {
                textSize.width = Max(textSize.width, stringSize.width);
                textSize.drawRect.dx = Max(textSize.drawRect.dx, stringSize.drawRect.dx);
            }
            textSize.drawRect.x = Min(textSize.drawRect.x, stringSize.drawRect.x);
        }
    }

    if (requestedSize.dx >= 0 && useJustify)
    {
        textSize.drawRect.dx = Max(textSize.drawRect.dx, (int)drawSize.dx);
    }

    //calc texture size
    float32 virt2phys = Core::GetVirtualToPhysicalFactor();
    int32 dx = (int32)ceilf(virt2phys * textSize.drawRect.dx);
    int32 dy = (int32)ceilf(virt2phys * textSize.drawRect.dy);
    int32 ox = (int32)floorf(virt2phys * textSize.drawRect.x);
    int32 oy = (int32)floorf(virt2phys * textSize.drawRect.y);

    cacheUseJustify = useJustify;
    cacheDx = dx;
    EnsurePowerOf2(cacheDx);

    cacheDy = dy;
    EnsurePowerOf2(cacheDy);

    cacheOx = ox;
    cacheOy = oy;

    cacheW = textSize.drawRect.dx;
    cacheFinalSize.x = (float32)textSize.drawRect.dx;
    cacheFinalSize.y = (float32)textSize.drawRect.dy;

    // Align sprite offset
    if(align & ALIGN_RIGHT)
    {
        cacheSpriteOffset.x = (float32)(textSize.drawRect.dx - textSize.width + textSize.drawRect.x);
    }
    else if(align & ALIGN_HCENTER)
    {
        cacheSpriteOffset.x = ((float32)(textSize.drawRect.dx - textSize.width) * 0.5f + textSize.drawRect.x);
    }
    else
    {
        cacheSpriteOffset.x = (float32)textSize.drawRect.x;
    }
    if(align & ALIGN_BOTTOM)
    {
        cacheSpriteOffset.y = (float32)(textSize.drawRect.dy - textSize.height + textSize.drawRect.y);
    }
    else if(align & ALIGN_VCENTER)
    {
        cacheSpriteOffset.y = ((float32)(textSize.drawRect.dy - textSize.height) * 0.5f + textSize.drawRect.y);
    }
    else
    {
        cacheSpriteOffset.y = (float32)textSize.drawRect.y;
    }

}

void TextBlock::PreDraw()
{
    if (textBlockRender)
    {
        textBlockRender->PreDraw();
    }
}

void TextBlock::Draw(const Color& textColor, const Vector2* offset/* = NULL*/)
{
    if (textBlockRender)
    {
        textBlockRender->Draw(textColor, offset);
    }
}

TextBlock * TextBlock::Clone()
{
    TextBlock *block = new TextBlock();

    block->SetRectSize(rectSize);
    block->SetMultiline(GetMultiline(), GetMultilineBySymbol());
    block->SetAlign(align);
    block->SetFittingOption(fittingType);

    if (GetFont())
    {
        block->SetFont(GetFont());
    }
    block->SetText(GetText(), requestedSize);

    return block;
}

void TextBlock::ForcePrepare(Texture *texture)
{
    needRedraw = true;
    Prepare(texture);
}

const Vector2 & TextBlock::GetTextSize()
{
    mutex.Lock();
    mutex.Unlock();

    return cacheFinalSize;
}

const Vector<int32> & TextBlock::GetStringSizes() const
{
    return stringSizes;
}

const Vector2& TextBlock::GetSpriteOffset()
{
    mutex.Lock();
    mutex.Unlock();

    return cacheSpriteOffset;
}

};