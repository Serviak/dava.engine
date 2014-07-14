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

#include "DFFont.h"

#include "Render/Texture.h"
#include "Render/RenderManager.h"
#include "Render/Shader.h"
#include "FileSystem/YamlParser.h"

#define NOT_DEF_CHAR 0xffff

namespace DAVA {

DFFont::DFFont()
:   fontTexture(NULL)
{
    fontType = TYPE_DISTANCE;
    baseSize = 0;
    paddingLeft = paddingRight = paddingTop = paddingBottom = 0;
    lineHeight = 0;
    spread = 1.f;
}

DFFont::~DFFont()
{
    RenderManager::Instance()->ReleaseTextureState(fontTextureHandler);
    SafeRelease(fontTexture);
}

DFFont* DFFont::Create(const FilePath & path)
{
    DFFont* font = new DFFont();
    
    if (!font->LoadConfig(path) || !font->LoadTexture(font->GetTexturePath()))
    {
        SafeRelease(font);
        return NULL;
    }
    
    return font;
}

Size2i DFFont::GetStringSize(const WideString & str, Vector<float32> *charSizes/* = 0*/) const
{ 
    int32 charDrawed = 0;
    return DrawStringToBuffer(str, 0, 0, NULL, charDrawed, charSizes);
}

bool DFFont::IsCharAvaliable(char16 ch) const
{
    CharsMap::const_iterator iter = chars.find(ch);
    return iter != chars.end();
}

uint32 DFFont::GetFontHeight() const
{
    return (lineHeight) * GetSizeScale();
}

Font * DFFont::Clone() const
{
    DFFont* dfFont = new DFFont();
    dfFont->chars = chars;
    dfFont->baseSize = baseSize;
    dfFont->paddingLeft = paddingLeft;
    dfFont->paddingRight = paddingRight;
    dfFont->paddingTop = paddingTop;
    dfFont->paddingBottom = paddingBottom;
    dfFont->lineHeight = lineHeight;
    dfFont->spread = spread;
    dfFont->configPath = configPath;
    dfFont->fontTexture = SafeRetain(fontTexture);
    dfFont->fontTextureHandler = fontTextureHandler;
    dfFont->size = size;
    dfFont->renderSize = renderSize;
    RenderManager::Instance()->RetainTextureState(fontTextureHandler);

    return dfFont;
}

bool DFFont::IsEqual(const Font *font) const
{
    if (!Font::IsEqual(font))
        return false;
    
    DFFont* dfFont = (DFFont*) font;
    if (dfFont->configPath != configPath)
        return false;
    
    return true;
}

Size2i DFFont::DrawStringToBuffer(const WideString & str,
                                  int32 xOffset,
                                  int32 yOffset,
                                  DFFontVertex* vertexBuffer,
                                  int32& charDrawed,
                                  Vector<float32> *charSizes /*= NULL*/,
                                  int32 justifyWidth,
                                  int32 spaceAddon) const
{
    int32 countSpace = 0;
    uint32 strLength = str.length();
	for(int32 i = 0; i < strLength; ++i)
	{
		if( L' ' == str[i])
		{
			countSpace++;
		}
    }
    int32 justifyOffset = 0;
    int32 fixJustifyOffset = 0;
    if (countSpace > 0 && justifyWidth > 0 && spaceAddon > 0)
    {
        int32 diff= justifyWidth - spaceAddon;
        justifyOffset =  diff / countSpace;
        fixJustifyOffset = diff - justifyOffset*countSpace;
        
    }
    uint32 vertexAdded = 0;
    charDrawed = 0;
    
    float32 lastX = xOffset;
    float32 lastY = 0;
    float32 sizeScale = GetSizeScale();

    CharsMap::const_iterator notDef = chars.find(NOT_DEF_CHAR);
    bool notDefExists = (notDef != chars.end());

    
    for (uint32 charPos = 0; charPos < strLength; ++charPos)
    {
        char16 charId = str.at(charPos);
        CharsMap::const_iterator iter = chars.find(charId);
        if (iter == chars.end())
        {
            if (notDefExists)
            {
                iter = notDef;
            }
            else
            {
                DVASSERT_MSG(false, "Font should contain .notDef character!");
                continue;
            }
        }
        
        if (charPos>0 && justifyOffset > 0 && charId == L' ')
        {
            lastX += justifyOffset;
            if (fixJustifyOffset > 0)
            {
                lastX++;
                fixJustifyOffset--;
            }
        }
        
        const CharDescription& charDescription = iter->second;

        float32 width = charDescription.width * sizeScale;
        float32 startX = lastX + charDescription.xOffset * sizeScale;

        float32 startHeight = charDescription.yOffset * sizeScale;
        float32 fullHeight = (charDescription.height + charDescription.yOffset) * sizeScale;
        
        startHeight += yOffset;
        fullHeight += yOffset;

        if (vertexBuffer)
        {
            vertexBuffer[vertexAdded].position.x = startX;
            vertexBuffer[vertexAdded].position.y = startHeight;
            vertexBuffer[vertexAdded].position.z = 0;
            vertexBuffer[vertexAdded].texCoord.x = charDescription.u;
            vertexBuffer[vertexAdded].texCoord.y = charDescription.v;
            
            vertexBuffer[vertexAdded + 1].position.x = startX + width;
            vertexBuffer[vertexAdded + 1].position.y = startHeight;
            vertexBuffer[vertexAdded + 1].position.z = 0;
            vertexBuffer[vertexAdded + 1].texCoord.x = charDescription.u2;
            vertexBuffer[vertexAdded + 1].texCoord.y = charDescription.v;
            
            vertexBuffer[vertexAdded + 2].position.x = startX + width;
            vertexBuffer[vertexAdded + 2].position.y = fullHeight;
            vertexBuffer[vertexAdded + 2].position.z = 0;
            vertexBuffer[vertexAdded + 2].texCoord.x = charDescription.u2;
            vertexBuffer[vertexAdded + 2].texCoord.y = charDescription.v2;
            
            vertexBuffer[vertexAdded + 3].position.x = startX;
            vertexBuffer[vertexAdded + 3].position.y = fullHeight;
            vertexBuffer[vertexAdded + 3].position.z = 0;
            vertexBuffer[vertexAdded + 3].texCoord.x = charDescription.u;
            vertexBuffer[vertexAdded + 3].texCoord.y = charDescription.v2;
            vertexAdded += 4;
        }

        float32 nextKerning = 0;
        if (charPos + 1 < strLength)
        {
            Map<int32, float32>::const_iterator iter = charDescription.kerning.find(str.at(charPos + 1));
            if (iter != charDescription.kerning.end())
            {
                nextKerning = iter->second;
            }
        }
        float32 charWidth = (charDescription.xAdvance + nextKerning) * sizeScale;
        if (charSizes)
            charSizes->push_back(charWidth * Core::GetVirtualToPhysicalFactor());
        lastX += charWidth;
        
        charDrawed++;
    }
    lastY += yOffset + GetFontHeight();

    return Size2i((int32)ceilf(lastX), (int32)ceilf(lastY));
}

float32 DFFont::GetSpread() const
{
    return 0.25f / (spread * GetSizeScale());
}

float32 DFFont::GetSizeScale() const
{
    return renderSize / baseSize;
}

bool DFFont::LoadTexture(const FilePath& path)
{
    DVASSERT(fontTexture == NULL);

    fontTexture = Texture::CreateFromFile(path);
    TextureStateData textureData;
    textureData.SetTexture(0, fontTexture);
    fontTextureHandler = RenderManager::Instance()->CreateTextureState(textureData);

    return true;
}

bool DFFont::LoadConfig(const DAVA::FilePath &path)
{
    YamlParser* parser = YamlParser::Create(path.GetAbsolutePathname());
    if (!parser)
        return false;
    
    configPath = path;
    
    YamlNode* rootNode = parser->GetRootNode();
    const YamlNode* configNode = rootNode->Get("font");
    if (!configNode)
    {
        SafeRelease(parser);
        return false;
    }
    const YamlNode* charsNode = configNode->Get("chars");
    if (!charsNode)
    {
        SafeRelease(parser);
        return false;
    }
    
    baseSize = configNode->Get("size")->AsFloat();
    const YamlNode* paddingTop = configNode->Get("padding_top");
    if (paddingTop)
        this->paddingTop = paddingTop->AsFloat();
    const YamlNode* paddingLeft = configNode->Get("padding_left");
    if (paddingLeft)
        this->paddingLeft = paddingLeft->AsFloat();
    const YamlNode* paddingBottom = configNode->Get("padding_bottop");
    if (paddingBottom)
        this->paddingBottom = paddingBottom->AsFloat();
    const YamlNode* paddingRight = configNode->Get("padding_right");
    if (paddingRight)
        this->paddingRight = paddingRight->AsFloat();
    const YamlNode* lineHeight = configNode->Get("lineHeight");
    if (lineHeight)
        this->lineHeight = lineHeight->AsFloat();
    const YamlNode* spread = configNode->Get("spread");
    if (spread)
        this->spread = spread->AsFloat();
    
    const MultiMap<String, YamlNode*> charsMap = charsNode->AsMap();
    MultiMap<String, YamlNode*>::const_iterator charsMapEnd = charsMap.end();
    for (MultiMap<String, YamlNode*>::const_iterator iter = charsMap.begin(); iter != charsMapEnd; ++iter)
    {
        char16 charId = atoi(iter->first.c_str());
        CharDescription charDescription;
        charDescription.height = iter->second->Get("height")->AsFloat();
        charDescription.width = iter->second->Get("width")->AsFloat();
        charDescription.xOffset = iter->second->Get("xoffset")->AsFloat();
        charDescription.yOffset = iter->second->Get("yoffset")->AsFloat();
        charDescription.xAdvance = iter->second->Get("xadvance")->AsFloat();
        charDescription.u = iter->second->Get("u")->AsFloat();
        charDescription.u2 = iter->second->Get("u2")->AsFloat();
        charDescription.v = iter->second->Get("v")->AsFloat();
        charDescription.v2 = iter->second->Get("v2")->AsFloat();
        
        chars[charId] = charDescription;
    }
    
    const YamlNode* kerningNode = configNode->Get("kerning");
    if (kerningNode)
    {
        const MultiMap<String, YamlNode*> kerningMap = kerningNode->AsMap();
        MultiMap<String, YamlNode*>::const_iterator kerningMapEnd = kerningMap.end();
        for (MultiMap<String, YamlNode*>::const_iterator iter = kerningMap.begin(); iter != kerningMapEnd; ++iter)
        {
            int32 charId = atoi(iter->first.c_str());
            CharsMap::iterator charIter = chars.find(charId);
            if (charIter == chars.end())
                continue;
            
            const MultiMap<String, YamlNode*> charKerningMap = iter->second->AsMap();
            MultiMap<String, YamlNode*>::const_iterator charKerningMapEnd = charKerningMap.end();
            for (MultiMap<String, YamlNode*>::const_iterator iter = charKerningMap.begin(); iter != charKerningMapEnd; ++iter)
            {
                int32 secondCharId = atoi(iter->first.c_str());
                float32 kerning = iter->second->AsFloat();
                charIter->second.kerning[secondCharId] = kerning;
            }
        }
    }

    SafeRelease(parser);
    return true;
}

FilePath DFFont::GetTexturePath() const
{
    return FilePath::CreateWithNewExtension(configPath, ".tex");
}
    
YamlNode * DFFont::SaveToYamlNode() const
{
    YamlNode *node = Font::SaveToYamlNode();
    //Type
    node->Set("type", "DFFont");
    
    String pathname = configPath.GetFrameworkPath();
    node->Set("name", pathname);
    
    return node;
}

const FilePath & DFFont::GetFontPath() const
{
    return configPath;
}

String DFFont::GetRawHashString()
{
    return configPath.GetFrameworkPath() + "_" + Font::GetRawHashString();
}

}