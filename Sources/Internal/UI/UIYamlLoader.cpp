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


#include "UI/UIYamlLoader.h"
#include "Base/ObjectFactory.h"
#include "Platform/SystemTimer.h"
#include "UI/UIPackage.h"
#include "UI/UIControl.h"
#include "UI/UIScrollBar.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/FileSystem.h"
#include "Render/2D/GraphicsFont.h"
#include "Render/2D/DFFont.h"
#include "Render/2D/FontManager.h"
#include "Render/2D/TextBlock.h"
#include "Utils/Utils.h"
#include "Render/2D/FTFont.h"
#include "UIControlHelpers.h"
#include "UIPackageLoader.h"
#include "DefaultUIPackageBuilder.h"

namespace DAVA
{
UIYamlLoader::UIYamlLoader()
    : mainRootControl(NULL)
{
    // Default mode is to ASSERT if custom control isn't found.
    assertIfCustomControlNotFound = true;

    currentPath = FilePath();
}

int32 UIYamlLoader::GetDrawTypeFromNode(const YamlNode * drawTypeNode) const
{
    int32 ret = UIControlBackground::DRAW_ALIGNED;
    if(!drawTypeNode)
        return ret;

    const String & type = drawTypeNode->AsString();

    if("DRAW_ALIGNED" == type) ret = UIControlBackground::DRAW_ALIGNED;
    if("DRAW_SCALE_TO_RECT" == type) ret = UIControlBackground::DRAW_SCALE_TO_RECT;
    if("DRAW_SCALE_PROPORTIONAL" == type) ret = UIControlBackground::DRAW_SCALE_PROPORTIONAL;
    if("DRAW_SCALE_PROPORTIONAL_ONE" == type) ret = UIControlBackground::DRAW_SCALE_PROPORTIONAL_ONE;
    if("DRAW_FILL" == type) ret = UIControlBackground::DRAW_FILL;
    if("DRAW_STRETCH_HORIZONTAL" == type) ret = UIControlBackground::DRAW_STRETCH_HORIZONTAL;
    if("DRAW_STRETCH_VERTICAL" == type) ret = UIControlBackground::DRAW_STRETCH_VERTICAL;
    if("DRAW_STRETCH_BOTH" == type) ret = UIControlBackground::DRAW_STRETCH_BOTH;
    if("DRAW_TILED" == type) ret = UIControlBackground::DRAW_TILED;

    return ret;
}

String UIYamlLoader::GetDrawTypeNodeValue(int32 drawType) const
{
    String ret;
    switch (drawType) {
        case UIControlBackground::DRAW_ALIGNED:
            ret = "DRAW_ALIGNED";
            break;
        case UIControlBackground::DRAW_SCALE_TO_RECT:
            ret = "DRAW_SCALE_TO_RECT";
            break;
        case UIControlBackground::DRAW_SCALE_PROPORTIONAL:
            ret = "DRAW_SCALE_PROPORTIONAL";
            break;
        case UIControlBackground::DRAW_SCALE_PROPORTIONAL_ONE:
            ret = "DRAW_SCALE_PROPORTIONAL_ONE";
            break;
        case UIControlBackground::DRAW_FILL:
            ret = "DRAW_FILL";
            break;
        case UIControlBackground::DRAW_STRETCH_HORIZONTAL:
            ret = "DRAW_STRETCH_HORIZONTAL";
            break;
        case UIControlBackground::DRAW_STRETCH_VERTICAL:
            ret = "DRAW_STRETCH_VERTICAL";
            break;
        case UIControlBackground::DRAW_STRETCH_BOTH:
            ret = "DRAW_STRETCH_BOTH";
            break;
        case UIControlBackground::DRAW_TILED:
            ret = "DRAW_TILED";
            break;
        default:
            ret = "DRAW_ALIGNED";
            break;
    }
    return ret;
}

int32 UIYamlLoader::GetColorInheritTypeFromNode(const YamlNode * colorInheritNode) const
{
    int32 ret = UIControlBackground::COLOR_IGNORE_PARENT;
    if(!colorInheritNode)
        return ret;

    const String & type = colorInheritNode->AsString();

    if("COLOR_MULTIPLY_ON_PARENT" == type) ret = UIControlBackground::COLOR_MULTIPLY_ON_PARENT;
    if("COLOR_ADD_TO_PARENT" == type) ret = UIControlBackground::COLOR_ADD_TO_PARENT;
    if("COLOR_REPLACE_TO_PARENT" == type) ret = UIControlBackground::COLOR_REPLACE_TO_PARENT;
    if("COLOR_IGNORE_PARENT" == type) ret = UIControlBackground::COLOR_IGNORE_PARENT;
    if("COLOR_MULTIPLY_ALPHA_ONLY" == type) ret = UIControlBackground::COLOR_MULTIPLY_ALPHA_ONLY;
    if("COLOR_REPLACE_ALPHA_ONLY" == type) ret = UIControlBackground::COLOR_REPLACE_ALPHA_ONLY;

    return ret;
}

String UIYamlLoader::GetColorInheritTypeNodeValue(int32 colorInheritType) const
{
    String ret;
    switch (colorInheritType) {
        case UIControlBackground::COLOR_MULTIPLY_ON_PARENT:
            ret = "COLOR_MULTIPLY_ON_PARENT";
            break;
        case UIControlBackground::COLOR_ADD_TO_PARENT:
            ret = "COLOR_ADD_TO_PARENT";
            break;
        case UIControlBackground::COLOR_REPLACE_TO_PARENT:
            ret = "COLOR_REPLACE_TO_PARENT";
            break;
        case UIControlBackground::COLOR_IGNORE_PARENT:
            ret = "COLOR_IGNORE_PARENT";
            break;
        case UIControlBackground::COLOR_MULTIPLY_ALPHA_ONLY:
            ret = "COLOR_MULTIPLY_ALPHA_ONLY";
            break;
        case UIControlBackground::COLOR_REPLACE_ALPHA_ONLY:
            ret = "COLOR_REPLACE_ALPHA_ONLY";
            break;
        default:
            ret = "COLOR_IGNORE_PARENT";
            break;
    }
    return ret;
}

int32 UIYamlLoader::GetPerPixelAccuracyTypeFromNode(const YamlNode *perPixelAccuracyNode) const
{
	int32 ret = UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
	if(!perPixelAccuracyNode)
		return ret;

	const String & type = perPixelAccuracyNode->AsString();

	if("PER_PIXEL_ACCURACY_DISABLED" == type) ret = UIControlBackground::PER_PIXEL_ACCURACY_DISABLED;
	if("PER_PIXEL_ACCURACY_ENABLED" == type) ret = UIControlBackground::PER_PIXEL_ACCURACY_ENABLED;
	if("PER_PIXEL_ACCURACY_FORCED" == type) ret = UIControlBackground::PER_PIXEL_ACCURACY_FORCED;

	return ret;
}

String UIYamlLoader::GetPerPixelAccuracyTypeNodeValue(int32 perPixelAccuracyType) const
{
	String ret;
    switch (perPixelAccuracyType) {
        case UIControlBackground::PER_PIXEL_ACCURACY_DISABLED:
            ret = "PER_PIXEL_ACCURACY_DISABLED";
            break;
        case UIControlBackground::PER_PIXEL_ACCURACY_ENABLED:
            ret = "PER_PIXEL_ACCURACY_ENABLED";
            break;
        case UIControlBackground::PER_PIXEL_ACCURACY_FORCED:
            ret = "PER_PIXEL_ACCURACY_FORCED";
            break;
    }
    return ret;
}
	
int32 UIYamlLoader::GetAlignFromYamlNode(const YamlNode * alignNode) const
{
    if (!alignNode)return ALIGN_HCENTER | ALIGN_VCENTER;

    const Vector<YamlNode*> & vec = alignNode->AsVector();

    if (vec.size() == 1 && vec[0]->AsString() == "HJUSTIFY") return ALIGN_HJUSTIFY;
    if (vec.size() != 2)return ALIGN_HCENTER | ALIGN_VCENTER;

    const String & horzAlign = vec[0]->AsString();
    const String & vertAlign = vec[1]->AsString();

    int32 align = 0;
    if (horzAlign == "LEFT")align |= ALIGN_LEFT;
    else if (horzAlign == "HCENTER")align |= ALIGN_HCENTER;
    else if (horzAlign == "RIGHT")align |= ALIGN_RIGHT;

    if (vertAlign == "TOP")align |= ALIGN_TOP;
    else if (vertAlign == "VCENTER")align |= ALIGN_VCENTER;
    else if (vertAlign == "BOTTOM")align |= ALIGN_BOTTOM;

    return align;
}

int32 UIYamlLoader::GetFittingOptionFromYamlNode( const YamlNode * fittingNode ) const
{
    int32 fitting = TextBlock::FITTING_DISABLED;

    const Vector<YamlNode*> & vec = fittingNode->AsVector();

    for( uint32 index = 0; index < vec.size(); ++index )
    {
        const String &value = vec[index]->AsString();
        if( value == "DISABLED" )
        {
            fitting = TextBlock::FITTING_DISABLED;
            break;
        }
        else if( value == "ENLARGE" )
        {
            fitting |= TextBlock::FITTING_ENLARGE;
        }
        else if( value == "REDUCE" )
        {
            fitting |= TextBlock::FITTING_REDUCE;
        }
        else if( value == "POINTS" )
        {
            fitting |= TextBlock::FITTING_POINTS;
        }
    }

    return fitting;
}

//Vector<String> UIYamlLoader::GetAlignNodeValue(int32 align)
YamlNode * UIYamlLoader::GetAlignNodeValue(int32 align) const
{
    YamlNode *alignNode = YamlNode::CreateArrayNode(YamlNode::AR_FLOW_REPRESENTATION);
    String horzAlign = "HCENTER";
    String vertAlign = "VCENTER";

    if (align == ALIGN_HJUSTIFY)
    {
        alignNode->Add("HJUSTIFY");
        return alignNode;
    }

    if (align & ALIGN_LEFT)
    {
        horzAlign = "LEFT";
    }
    else if (align & ALIGN_HCENTER)
    {
        horzAlign = "HCENTER";
    }
    else if (align & ALIGN_RIGHT)
    {
        horzAlign = "RIGHT";
    }

    if (align & ALIGN_TOP)
    {
        vertAlign = "TOP";
    }
    else if (align & ALIGN_VCENTER)
    {
        vertAlign = "VCENTER";
    }
    else if (align & ALIGN_BOTTOM)
    {
        vertAlign = "BOTTOM";
    }

    alignNode->Add(horzAlign);
    alignNode->Add(vertAlign);

    return alignNode;
}

YamlNode * UIYamlLoader::GetFittingOptionNodeValue( int32 fitting ) const
{
    YamlNode *fittingNode = YamlNode::CreateArrayNode(YamlNode::AR_FLOW_REPRESENTATION);

    if( fitting == TextBlock::FITTING_DISABLED )
    {
        fittingNode->Add("DISABLED");
    }
    else
    {
        if( fitting & TextBlock::FITTING_ENLARGE )
        {
            fittingNode->Add("ENLARGE");
        }
        if( fitting & TextBlock::FITTING_REDUCE )
        {
            fittingNode->Add("REDUCE");
        }
        if( fitting & TextBlock::FITTING_POINTS )
        {
            fittingNode->Add("POINTS");
        }
    }
    return fittingNode;
}

bool UIYamlLoader::GetBoolFromYamlNode(const YamlNode * node, bool defaultValue) const
{
    if (!node)return defaultValue;

    const String & value = node->AsString();
    if (value == "yes")return true;
    else if (value == "no")return false;
    else if (value == "true")return true;

    return false;
}

int32 HexCharToInt(char c)
{
    if ((c >= '0') && (c <= '9'))return c - '0';
    else if ((c >= 'a') && (c <= 'f'))return c - 'a' + 10;
    else if ((c >= 'A') && (c <= 'F'))return c - 'A' + 10;
    return 0;
}

Color UIYamlLoader::GetColorFromYamlNode(const YamlNode * node) const
{
    if (node->GetType() == YamlNode::TYPE_ARRAY)
    {
        if (node->GetCount() == 4)
            return node->AsColor();
        else return Color::White;
    }else
    {
        const String & color = node->AsString();

        int r = HexCharToInt(color[0]) * 16 + HexCharToInt(color[1]);
        int g = HexCharToInt(color[2]) * 16 + HexCharToInt(color[3]);
        int b = HexCharToInt(color[4]) * 16 + HexCharToInt(color[5]);
        int a = HexCharToInt(color[6]) * 16 + HexCharToInt(color[7]);

        return Color((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f);
    }
}


Font * UIYamlLoader::GetFontByName(const String & fontName) const
{
    return FontManager::Instance()->GetFont(fontName);
}

void UIYamlLoader::LoadFonts(const FilePath & yamlPathname)
{
    ScopedPtr<UIYamlLoader> loader( new UIYamlLoader() );
    YamlNode * rootNode = loader->CreateRootNode(yamlPathname);
    if (!rootNode)
    {
        // Empty YAML file.
        Logger::Warning("yaml file: %s is empty", yamlPathname.GetAbsolutePathname().c_str());
        return;
    }
    loader->LoadFontsFromNode(rootNode);
    SafeRelease(rootNode);
}

bool UIYamlLoader::SaveFonts(const FilePath & yamlPathname)
{
    bool res = false;

    //save used fonts
    const FontManager::TRACKED_FONTS& usedFonts = FontManager::Instance()->GetTrackedFont();
    ScopedPtr<YamlNode> fontsNode( new YamlNode(YamlNode::TYPE_MAP) );
    for (FontManager::TRACKED_FONTS::const_iterator iter = usedFonts.begin();
         iter != usedFonts.end();
         ++iter)
    {
        Font* font = (*iter);
        if (!font)
            continue;

        // The font should be stored once only.
        String fontName = FontManager::Instance()->GetFontName(font);
        Logger::FrameworkDebug("UIYamlLoader::SaveFonts fontName=%s for font=%p", fontName.c_str(), font);

        font = FontManager::Instance()->GetFont(fontName);
        if (!font)
            continue;
        Logger::FrameworkDebug("UIYamlLoader::SaveFonts font=%p for fontName=%s", font, fontName.c_str());

        if (fontsNode->AsMap().find(fontName) == fontsNode->AsMap().end())
        {
            fontsNode->AddNodeToMap( fontName, font->SaveToYamlNode() );
        }
    }

    res = YamlEmitter::SaveToYamlFile(yamlPathname, fontsNode, File::CREATE | File::WRITE);

    return res;
}

void UIYamlLoader::Load(UIControl * rootControl, const FilePath & yamlPathname, bool assertIfCustomControlNotFound)
{
    DefaultUIPackageBuilder builder;
    RefPtr<UIPackage> package(UIPackageLoader(&builder).LoadPackage(yamlPathname));
    if (!package.Valid())
        return;

    DVASSERT(package->GetControlsCount() == 1);
    UIControl *control = package->GetControl(0);
    DVASSERT(control);
    while (!control->GetChildren().empty())
    {
        rootControl->AddControl(control->GetChildren().front());
    }

    if (rootControl->GetSize() != control->GetSize())
        rootControl->UpdateLayout();
}

bool UIYamlLoader::Save(UIControl * rootControl, const FilePath & yamlPathname, bool skipRootNode)
{
    ScopedPtr<UIPackage> package(new UIPackage(yamlPathname));
    package->AddControl(rootControl);
    DVASSERT(false);
    return false;
    //return UIPackageLoader().SavePackage(package);
}

YamlNode *UIYamlLoader::CreateRootNode(const FilePath & yamlPathname)
{
    YamlParser * parser = YamlParser::Create(yamlPathname);
    if (!parser)
    {
        Logger::Error("Failed to open yaml file: %s", yamlPathname.GetAbsolutePathname().c_str());
        return NULL;
    }
    currentPath = yamlPathname.GetDirectory();
    YamlNode * rootNode = SafeRetain(parser->GetRootNode());
    SafeRelease(parser);
    return rootNode;
}

void UIYamlLoader::PostLoad(UIControl * rootControl)
{
    //Find ScrollBars and set delegates
    SetScrollBarDelegates(rootControl);
}

void UIYamlLoader::SetScrollBarDelegates(UIControl * rootControl)
{
    Map<UIScrollBar*,String>::iterator it = scrollsToLink.begin();
    for (; it!=scrollsToLink.end(); ++it)
    {
        UIControl * control = UIControlHelpers::GetControlByPath(it->second, rootControl);
        it->first->SetDelegate( dynamic_cast<UIScrollBarDelegate*>(control));
    }
    scrollsToLink.clear();
}

void UIYamlLoader::LoadFontsFromNode(const YamlNode * rootNode)
{
    for (MultiMap<String, YamlNode*>::const_iterator t = rootNode->AsMap().begin(); t != rootNode->AsMap().end(); ++t)
    {
        YamlNode * node = t->second;
        const YamlNode * typeNode = node->Get("type");
        if (!typeNode)continue;

        const String & type = typeNode->AsString();
        if (type == "FTFont")
        {
            // parse font
            const YamlNode * fontNameNode = node->Get("name");
            if (!fontNameNode)continue;

            float32 fontSize = 10.0f;
            const YamlNode * fontSizeNode = node->Get("size");
            if (fontSizeNode)fontSize = fontSizeNode->AsFloat();

            FTFont * font = FTFont::Create(fontNameNode->AsString());
            if (!font)
            {
                continue;
            }

            font->SetSize(fontSize);

            const YamlNode * fontVerticalSpacingNode = node->Get("verticalSpacing");
            if(fontVerticalSpacingNode)
            {
                font->SetVerticalSpacing(fontVerticalSpacingNode->AsInt32());
            }

            //fontMap[t->first] = font;
            FontManager::Instance()->SetFontName(font, t->first);
            SafeRelease(font);
        }
        else if(type == "GraphicsFont")
        {
            // parse font
            const YamlNode * fontNameNode = node->Get("sprite");
            if (!fontNameNode)continue;

            const YamlNode * definitionNode = node->Get("definition");
            if (!definitionNode)continue;

            GraphicsFont * font = GraphicsFont::Create(definitionNode->AsString(), fontNameNode->AsString());
            if (!font)
            {
                continue;
            }

            const YamlNode * fontSizeNode = node->Get("size");
            if (fontSizeNode)
            {
                font->SetSize(fontSizeNode->AsFloat());
            }

            const YamlNode * fontVerticalSpacingNode = node->Get("verticalSpacing");
            if(fontVerticalSpacingNode)
            {
                font->SetVerticalSpacing(fontVerticalSpacingNode->AsInt32());
            }

            const YamlNode * fontHorizontalSpacingNode = node->Get("horizontalSpacing");
            if(fontHorizontalSpacingNode)
            {
                font->SetHorizontalSpacing(fontHorizontalSpacingNode->AsInt32());
            }

            //fontMap[t->first] = font;
            FontManager::Instance()->SetFontName(font, t->first);
            SafeRelease(font);
        }
        else if (type == "DFFont")
        {
            // parse font
            const YamlNode * fontNameNode = node->Get("name");
            if (!fontNameNode)continue;

            float32 fontSize = 10.0f;
            const YamlNode * fontSizeNode = node->Get("size");
            if (fontSizeNode)fontSize = fontSizeNode->AsFloat();

            DFFont * font = DFFont::Create(fontNameNode->AsString());
            if (!font)
            {
                continue;
            }

            font->SetSize(fontSize);

            const YamlNode * fontVerticalSpacingNode = node->Get("verticalSpacing");
            if(fontVerticalSpacingNode)
            {
                font->SetVerticalSpacing(fontVerticalSpacingNode->AsInt());
            }

            //fontMap[t->first] = font;
            FontManager::Instance()->SetFontName(font, t->first);
        }
    }
}

void UIYamlLoader::SetAssertIfCustomControlNotFound(bool value)
{
    this->assertIfCustomControlNotFound = value;
}

const FilePath & UIYamlLoader::GetCurrentPath() const
{
    return currentPath;
}

void UIYamlLoader::AddScrollBarToLink(UIScrollBar* scroll, const String& delegatePath)
{
    scrollsToLink.insert(std::pair<UIScrollBar*,String>(scroll,delegatePath));
}

void UIYamlLoader::SetRootControl(UIControl *control)
{
    SafeRelease(mainRootControl);
    mainRootControl = SafeRetain(control);
}


}