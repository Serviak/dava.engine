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


#include "Render/2D/Font.h"
#include "Core/Core.h"
#include "FileSystem/YamlParser.h"
#include "FontManager.h"

#include "Utils/StringFormat.h"
#include "Utils/CRC32.h"

namespace DAVA
{
	
	
int32 Font::globalFontDPI = 72;

void Font::SetDPI(int32 dpi)
{
	globalFontDPI = dpi;
}

int32 Font::GetDPI()
{
	return globalFontDPI;
}
	
Font::Font()
:	size(14.0f)
,   renderSize(14.0f)
,	verticalSpacing(0)
{
	FontManager::Instance()->RegisterFont(this);
}

Font::~Font()
{
	FontManager::Instance()->UnregisterFont(this);
}


bool Font::IsEqual(const Font *font) const
{
    if(!font)
    {
        return false;
    }
    
	if (fontType != font->fontType) 
	{
		return false;
	}
	if (size != font->size || verticalSpacing != font->verticalSpacing)
	{
		return false;
	}
	
	return true;
}

uint32 Font::GetHashCode()
{
	String rawHashString = GetRawHashString();
	return CRC32::ForBuffer(rawHashString.c_str(), rawHashString.length());
}

String Font::GetRawHashString()
{
	return Format("%i_%.0f_%i", fontType, size, verticalSpacing);
}

void Font::SetSize(float32 _size)
{
	size = _size;
    renderSize = _size;
}

float32	Font::GetSize() const
{
	return size;
}

void Font::SetRenderSize(float32 _originalSize)
{
    renderSize = _originalSize;
}

float32	Font::GetRenderSize() const
{
    return renderSize;
}

void Font::SetVerticalSpacing(int32 _verticalSpacing)
{
	verticalSpacing = _verticalSpacing;
}

int32 Font::GetVerticalSpacing() const
{
	return verticalSpacing;
}

    
void Font::SplitTextBySymbolsToStrings(const WideString & text, const Vector2 & targetRectSize, Vector<WideString> & resultVector)
{
	int32 targetWidth = (int32)(targetRectSize.dx * Core::GetVirtualToPhysicalFactor());
    int32 totalSize = (int)text.length();
    
    int32 currentLineStart = 0;
	int32 currentLineEnd = 0;
    int32 currentLineDx = 0;
    
	resultVector.clear();
    
    Vector<int32> sizes;
	GetStringSize(text, &sizes);
	if(sizes.size() == 0)
	{
		return;
	}

    for(int pos = 0; pos < totalSize; pos++)
    {
        char16 t = text[pos];
        char16 tNext = 0;
        if(pos+1 < totalSize)
            tNext = text[pos+1];
        
        currentLineEnd = pos;
        
        if(t == '\n')
        {
            WideString currentLine = text.substr(currentLineStart, currentLineEnd - currentLineStart);
            resultVector.push_back(currentLine);
            
            currentLineStart = pos + 1;
            currentLineDx = 0;
        }
        if(t == '\\' && tNext == 'n')
        {
            WideString currentLine = text.substr(currentLineStart, currentLineEnd - currentLineStart);
            resultVector.push_back(currentLine);
            
            currentLineStart = pos + 2;
            currentLineDx = 0;
        }
		
		// Use additional condition to prevent endless loop, when target size is less than
		// size of one symbol (sizes[pos] > targetWidth)
		// To keep initial index logic we should always perform action currentLineDx += sizes[pos]
		// before entering this condition, so currentLineDx > 0.
        if((currentLineDx > 0) && (currentLineDx + sizes[pos] > targetWidth))
        {
            WideString currentLine = text.substr(currentLineStart, currentLineEnd - currentLineStart);
            resultVector.push_back(currentLine);
            
            currentLineStart = pos;
            currentLineDx = 0;
            pos--;
        }
        else
        {
            currentLineDx += sizes[pos];
        }
    }
    
    WideString currentLine = text.substr(currentLineStart, currentLineEnd - currentLineStart + 1);
    resultVector.push_back(currentLine);
}
    
bool Font::IsWordSeparator(char16 t) const
{
    switch(t)
    {
        case 183: // interpunkt
        // japanese characters that cannot start line ヽヾーァィゥェォッャュョヮヵヶぁぃぅぇぉっゃゅょゎゕゖㇰㇱㇲㇳㇴㇵㇶㇷㇸㇹㇺㇻㇼㇽㇾㇿ々〻
        case 12541:
        case 12542:
        case 12540:
        case 12449:
        case 12451:
        case 12453:
        case 12455:
        case 12457:
        case 12483:
        case 12515:
        case 12517:
        case 12519:
        case 12526:
        case 12533:
        case 12534:
        case 12353:
        case 12355:
        case 12357:
        case 12359:
        case 12361:
        case 12387:
        case 12419:
        case 12421:
        case 12423:
        case 12430:
        case 12437:
        case 12438:
        case 12784:
        case 12785:
        case 12786:
        case 12787:
        case 12788:
        case 12789:
        case 12790:
        case 12791:
        case 12792:
        case 12793:
        case 12794:
        case 12795:
        case 12796:
        case 12797:
        case 12798:
        case 12799:
        case 12293:
        case 12347:
        // brackets )]｝〕〉》」』】〙〗〟’”｠»
        case 41:
        case 93:
        case 65373:
        case 12309:
        case 12297:
        case 12299:
        case 12301:
        case 12303:
        case 12305:
        case 12313:
        case 12311:
        case 12319:
        case 8217:
        case 8221:
        case 65376:
        case 187:
        // hyphens ‐゠–〜
        case 8208:
        case 12448:
        case 8211:
        case 12316:
        // delimeters ?!‼⁇⁈⁉
        case 63:
        case 33:
        case 8252:
        case 8263:
        case 8264:
        case 8265:
        // punctuation mid ・、:;,
        case 12539:
        case 12289: // ideographic comma
        case 58:
        case 59:
        case 44:
        // punctuation end 。.
        case 12290:
        case 46:
            return true;
    }
    
    return false;
}
    
void Font::SplitTextToStrings(const WideString & text, const Vector2 & targetRectSize, Vector<WideString> & resultVector)
{
	int32 targetWidth = (int32)(targetRectSize.dx * Core::GetVirtualToPhysicalFactor());

	enum
	{
		SKIP = 0,
		GOODCHAR,	// all characters we like (symbols, special chars, except \n and space
		FINISH,	// process last line
		EXIT,
	};

    // Yuri Coder, 2013/12/10. Replace "\n" occurrences (two chars) to '\n' (one char) is done by Yaml parser,
    // so appropriate code (state NEXTLINE)is removed from here. See please MOBWOT-6499.

	resultVector.clear();
	//int textLength = text.length();
	int state = SKIP;
	int lastWordStart = 0;
	int lastWordEnd = 0;
	int currentLineStart = -1;
	int currentLineEnd = 0;
	int totalSize = (int)text.length();
	
	Vector<int32> sizes;
	GetStringSize(text, &sizes);
    if(sizes.size() == 0)
    {
        return;
    }
	
	for(int pos = 0; state != EXIT; pos++)
	{
		char16 t = 0;
		if(pos < totalSize)
		{
			t = text[pos];
		}
		switch (state) 
		{
			case SKIP:
				if (t == 0){ state = FINISH; break; } // if end of string process FINISH state and exit
				else if (IsSpace(t))break; // if space continue with the same state
				else if (IsWordSeparator(t))break; // if word separator - continue with the same state
                else if(IsLineEnd(t))
				{
					// this block is copied from case NEXTLINE: if(t == 'n')
					// unlike in NEXTLINE where we ignore 2 symbols, here we ignore only one
					// so last position is pos instead of (pos-1)
					if (currentLineStart != -1) // if we already have something in current line we add to result
					{
						//Logger::FrameworkDebug("before=%d %d", currentLineStart, pos - 1);
						WideString currentLineWithoutLastWord = text.substr(currentLineStart, pos - currentLineStart);
						//Logger::FrameworkDebug(L"after=%S", currentLineWithoutLastWord.c_str());
						resultVector.push_back(currentLineWithoutLastWord);
						
						currentLineStart = -1;	// start search of characters for the new line
					}else
					{
						resultVector.push_back(L""); // here we add empty line if there was no characters in current line
					}
					state = SKIP; //always switch to SKIP because we do not know here what will be next
					break;
				}
				else // everything else is good characters
				{
					state = GOODCHAR;
					lastWordStart = pos;
					lastWordEnd = pos;
					if (currentLineStart == -1)currentLineStart = pos;
				}
				break;
			case GOODCHAR:
				if(IsSpace(t) || IsLineEnd(t) || IsWordSeparator(t) || t == 0) // if we've found any possible separator process current line
                {
                    if(IsWordSeparator(t))
                    {
                        lastWordEnd = pos + 1;
                    }
                    else
                    {
                        lastWordEnd = pos;
                    }
					
					//					WideString currentLine = text.substr(currentLineStart, lastWordEnd - currentLineStart);
					//					Size2i currentLineSize = GetStringSize(currentLine);
					int currentLineDx = 0;
					for (int i = currentLineStart; i < lastWordEnd ; i++)
					{
						currentLineDx += sizes[i];
					}
					if ((currentLineDx < targetWidth) || (currentLineEnd <= currentLineStart)) // if current line size < rect size set line end to current word end
					{
						currentLineEnd = lastWordEnd;
					}else // here we add current line to results because current word is too big for current line
					{
						//Logger::FrameworkDebug("before=%d %d", currentLineStart, currentLineEnd);
						WideString currentLineWithoutLastWord = text.substr(currentLineStart, currentLineEnd - currentLineStart);
						//Logger::FrameworkDebug(L"after=%S", currentLineWithoutLastWord.c_str());
						resultVector.push_back(currentLineWithoutLastWord);
						currentLineStart = lastWordStart;
						//fix: 
						// there can be case when last word on current line with one more word (lastWordEnd = pos;) will be wider than targetRect.dx
						// in this case currentLineEnd could have been less than currentLineStart
						currentLineEnd = lastWordEnd;   
					}
				}
				if (IsSpace(t)) state = SKIP; // if cur char is space go to skip
                else if (IsWordSeparator(t)) state = SKIP; // if cur char is word separator go to skip
                else if(IsLineEnd(t))
				{
					// this block is copied from case NEXTLINE: if(t == 'n')
					// unlike in NEXTLINE where we ignore 2 symbols, here we ignore only one
					// so last position is pos instead of (pos-1)
					if (currentLineStart != -1) // if we already have something in current line we add to result
					{
						//Logger::FrameworkDebug("before=%d %d", currentLineStart, pos - 1);
						WideString currentLineWithoutLastWord = text.substr(currentLineStart, pos - currentLineStart);
						//Logger::FrameworkDebug(L"after=%S", currentLineWithoutLastWord.c_str());
						resultVector.push_back(currentLineWithoutLastWord);
						
						currentLineStart = -1;	// start search of characters for the new line
					}else
					{
						resultVector.push_back(L""); // here we add empty line if there was no characters in current line
					}
					state = SKIP; //always switch to SKIP because we do not know here what will be next
					break;
				}

				else if (t == 0) state = FINISH; 
				
				break;
			case FINISH:
				if (currentLineStart != -1) // we check if we have something left in currentline and add this line to results
				{
					//Logger::FrameworkDebug("ending=%d %d", currentLineStart, currentLineEnd);
					WideString currentLine = text.substr(currentLineStart, currentLineEnd - currentLineStart);
					//Logger::FrameworkDebug(L"after=%S", currentLine.c_str());
					resultVector.push_back(currentLine);
				}
				state = EXIT; // always exit from here
				break;
		};
	};
}

Font::eFontType Font::GetFontType() const
{
    return  fontType;
}

YamlNode * Font::SaveToYamlNode() const
{
    YamlNode *node = new YamlNode(YamlNode::TYPE_MAP);
    
    VariantType *nodeValue = new VariantType();
    //Type
    node->Set("type", "Font");
    //Font size
    node->Set("size", this->GetSize());
    //Vertical Spacing
    node->Set("verticalSpacing", this->GetVerticalSpacing());

    SafeDelete(nodeValue);
    
    return node;
}

Size2i Font::DrawString(float32 /*offsetX*/, float32 /*offsetY*/, const WideString & /*str*/, int32 /*justifyWidth*/)
{
	return Size2i(0, 0);
}

Size2i Font::DrawStringToBuffer(void * /*buffer*/, int32 /*bufWidth*/, int32 /*bufHeight*/, int32 /*offsetX*/, int32 /*offsetY*/, int32 /*justifyWidth*/, int32 /*spaceAddon*/, const WideString & /*str*/, bool /*contentScaleIncluded*/)
{
	return  Size2i(0, 0);
}

};