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



#include "EditorFontManager.h"
#include "StringUtils.h"

#include "UI/UIYamlLoader.h"

static const String DEFAULT_FONT_NAME = "MyriadPro-Regular.otf";
static const String DEFAULT_FONT_PATH = "~res:/Fonts/MyriadPro-Regular.otf";

EditorFontManager::EditorFontManager()
{
	defaultFont = NULL;
	baseFont = NULL;
	Init();
}

EditorFontManager::~EditorFontManager()
{
	Reset();
    SafeRelease(baseFont);
}

void EditorFontManager::Init()
{
	baseFont = CreateDefaultFont(DEFAULT_FONT_PATH, DEFAULT_FONT_NAME);
    
    //TODO: get locales from existing font folders?
    locales.push_back("ko");
    locales.push_back("zh-Hans");
}

void EditorFontManager::OnProjectLoaded()
{
    // all fonts are loaded and registered in FontManager by now
    
    // OnProjectLoaded is used for backward-compatibility with versions, where fonts were part of ui yaml files
    
    const Map<Font*, String> registeredFonts = FontManager::Instance()->GetRegisteredFonts();
    Map<Font*, String>::const_iterator it = registeredFonts.begin();
    Map<Font*, String>::const_iterator endIt = registeredFonts.end();
    
    Logger::Debug("EditorFontManager::OnProjectLoaded registeredFonts.size()=%d", registeredFonts.size());
    
    int32 localesCount = locales.size();
    for(; it != endIt; ++it)
    {
        Font* font = it->first;
        String fontName = it->second;
        
        Logger::Debug("EditorFontManager::OnProjectLoaded defaultRegisteredFonts[%x] = %s", font, fontName.c_str());
        defaultRegisteredFonts[font] = fontName;
        
        Map<Font*, String>::const_iterator defaultRegisteredEndIt = defaultRegisteredFonts.end();
        Map<Font*, String>::const_iterator defaultRegisteredFindIt = defaultRegisteredFonts.find(font);
        if(defaultRegisteredFindIt == defaultRegisteredEndIt)
        {
            Map<String, Font*>::const_iterator defaultEndIt = defaultFonts.end();
            Map<String, Font*>::const_iterator defaultFindIt = defaultFonts.find(fontName);
            if(defaultFindIt != defaultEndIt)
            {
                font = defaultFindIt->second;
                Logger::Debug("EditorFontManager::OnProjectLoaded defaultRegisteredFonts[%x] = %s", font, fontName.c_str());
                defaultRegisteredFonts[font] = fontName;
            }
            else
            {
                defaultFonts[fontName] = SafeRetain(font);
            }
            
            for(int32 i = 0; i < localesCount; ++i)
            {
                String locale = locales[i];
                
                
                
                Map<Font*, String>::const_iterator localizedRegisteredEndIt = localizedRegisteredFonts[locale].end();
                Map<Font*, String>::const_iterator localizedRegisteredFindIt = localizedRegisteredFonts[locale].find(it->first);
                if(localizedRegisteredFindIt == localizedRegisteredEndIt)
                {
                    Font* localizedFont = font;
                    
                    Logger::Debug("EditorFontManager::OnProjectLoaded localizedRegisteredFonts[%s][%x] = %s", locale.c_str(), localizedFont, fontName.c_str());
                    localizedRegisteredFonts[locale][localizedFont] = fontName;
                    
                    Map<String, Font*>::const_iterator localizedEndIt = localizedFonts[locale].end();
                    Map<String, Font*>::const_iterator localizedFindIt = localizedFonts[locale].find(fontName);
                    if(localizedFindIt != localizedEndIt)
                    {
                        localizedFont = localizedFindIt->second;
                        Logger::Debug("EditorFontManager::OnProjectLoaded localizedRegisteredFonts[%s][%x] = %s", locale.c_str(), localizedFont, fontName.c_str());
                        localizedRegisteredFonts[locale][localizedFont] = fontName;
                    }
                    else
                    {
                        localizedFonts[locale][fontName] = SafeRetain(localizedFont);
                    }
                    
                }
                else
                {
                    if(it->second != localizedRegisteredFindIt->second)
                    {
                        Logger::Warning("EditorFontManager::OnProjectLoaded %s font %x is already registred, but as %s instead of %s", locale.c_str(), it->first, it->second.c_str(), localizedRegisteredFindIt->second.c_str());
                    }
                    else
                    {
                        Logger::Debug("EditorFontManager::OnProjectLoaded %s font %x is already registred as %s", locale.c_str(), it->first, it->second.c_str());
                    }
                }
            }
        }
        else
        {
            if(it->second != defaultRegisteredFindIt->second)
            {
                Logger::Warning("EditorFontManager::OnProjectLoaded default font %x is already registred, but as %s instead of %s", it->first, it->second.c_str(), defaultRegisteredFindIt->second.c_str());
            }
            else
            {
                Logger::Debug("EditorFontManager::OnProjectLoaded default font %x is already registred as %s", it->first, it->second.c_str());
            }
        }
    }
    
    Logger::Debug("EditorFontManager::OnProjectLoaded defaultRegisteredFonts.size()=%d", defaultRegisteredFonts.size());
}

void EditorFontManager::LoadLocalizedFonts()
{
    //TODO: load localized fonts
    FontManager::Instance()->Reset();
    
    int32 localesCount = locales.size();
    for(int32 i = 0; i < localesCount; ++i)
    {
        String locale = locales[i];
        UIYamlLoader::LoadFonts(GetLocalizedFontsPath(locale));
    
        //TODO: save loaded fonts to localizedFonts
        const Map<Font*, String> registeredFonts = FontManager::Instance()->GetRegisteredFonts();
        Map<Font*, String>::const_iterator it = registeredFonts.begin();
        Map<Font*, String>::const_iterator endIt = registeredFonts.end();
        for(; it != endIt; ++it)
        {
            localizedRegisteredFonts[locale][it->first] = it->second;
            localizedFonts[locale][it->second] = SafeRetain(it->first);
        }
        
        //TODO: remove logs
        {
            Map<String, Font*>::const_iterator it = localizedFonts[locale].begin();
            Map<String, Font*>::const_iterator endIt = localizedFonts[locale].end();
            for(; it != endIt; ++it)
            {
                Logger::Debug("EditorFontManager::LoadLocalizedFonts localizedFonts[%s][%s]=%x", locale.c_str(), it->first.c_str(), it->second);
            }
            Logger::Debug("EditorFontManager::LoadLocalizedFonts localizedFonts[%s].size()=%d", locale.c_str(), localizedFonts[locale].size());
        }
        
        FontManager::Instance()->Reset();
    }

    {
        UIYamlLoader::LoadFonts(GetDefaultFontsPath());
        
        const Map<Font*, String> registeredFonts = FontManager::Instance()->GetRegisteredFonts();
        Map<Font*, String>::const_iterator it = registeredFonts.begin();
        Map<Font*, String>::const_iterator endIt = registeredFonts.end();
        for(; it != endIt; ++it)
        {
            defaultRegisteredFonts[it->first] = it->second;
            defaultFonts[it->second] = SafeRetain(it->first);
            Logger::Debug("EditorFontManager::LoadLocalizedFonts defaultRegisteredFonts[%x] = %s", it->first, it->second.c_str());
        }
        
        Logger::Debug("EditorFontManager::LoadLocalizedFonts defaultRegisteredFonts.size()=%d", defaultRegisteredFonts.size());
        
        //TODO: remove logs
        {
            Map<String, Font*>::const_iterator it = defaultFonts.begin();
            Map<String, Font*>::const_iterator endIt = defaultFonts.end();
            for(; it != endIt; ++it)
            {
                Logger::Debug("EditorFontManager::LoadLocalizedFonts defaultFonts[%s] = %x", it->first.c_str(), it->second);
            }
            Logger::Debug("EditorFontManager::LoadLocalizedFonts defaultFonts.size()=%d", defaultFonts.size());
        }
    }
}

void EditorFontManager::SaveLocalizedFonts()
{
    //TODO: save localized fonts
    int32 localesCount = locales.size();
    for(int32 i = 0; i < localesCount; ++i)
    {
        String locale = locales[i];
        
        FontManager::Instance()->Reset();
        
        //TODO: load localized fonts into FontManager
        
        //TODO: remove logs
        {
            Map<String, Font*>::const_iterator it = localizedFonts[locale].begin();
            Map<String, Font*>::const_iterator endIt = localizedFonts[locale].end();
            for(; it != endIt; ++it)
            {
                Logger::Debug("EditorFontManager::SaveLocalizedFonts localizedFonts[%s][%s]=%x", locale.c_str(), it->first.c_str(), it->second);
            }
            Logger::Debug("EditorFontManager::SaveLocalizedFonts localizedFonts[%s].size()=%d", locale.c_str(), localizedFonts[locale].size());
        }
        
        FontManager::Instance()->RegisterFonts(localizedRegisteredFonts[locale]);
        
        FontManager::Instance()->PrepareToSaveFonts(true);
        
        UIYamlLoader::SaveFonts(EditorFontManager::Instance()->GetLocalizedFontsPath(locale));
    }
    
    FontManager::Instance()->Reset();
    
    //TODO: load localized font into FontManager
    {
        Map<Font*, String>::const_iterator drIt = defaultRegisteredFonts.begin();
        Map<Font*, String>::const_iterator drEndIt = defaultRegisteredFonts.end();
        for(; drIt != drEndIt; ++drIt)
        {
            Logger::Debug("EditorFontManager::SaveLocalizedFonts defaultRegisteredFonts %x %s", drIt->first, drIt->second.c_str());
        }
        Logger::Debug("EditorFontManager::SaveLocalizedFonts defaultRegisteredFonts %d", defaultRegisteredFonts.size());
    }
    
    FontManager::Instance()->RegisterFonts(defaultRegisteredFonts);
    
    FontManager::Instance()->PrepareToSaveFonts(true);
    
    //TODO: remove logs
    {
        Map<String, Font*>::const_iterator it = defaultFonts.begin();
        Map<String, Font*>::const_iterator endIt = defaultFonts.end();
        for(; it != endIt; ++it)
        {
            Logger::Debug("EditorFontManager::SaveLocalizedFonts defaultFonts[%s] = %x", it->first.c_str(), it->second);
        }
        Logger::Debug("EditorFontManager::SaveLocalizedFonts defaultFonts.size()=%d", defaultFonts.size());
    }
    
    UIYamlLoader::SaveFonts(EditorFontManager::Instance()->GetDefaultFontsPath());
}

void EditorFontManager::ClearLocalizedFonts()
{
    ClearFonts(defaultFonts);
    defaultRegisteredFonts.clear();
    Logger::Debug("EditorFontManager::ClearLocalizedFonts defaultFonts.size()=%d", defaultFonts.size());
    
    int32 localesCount = locales.size();
    for(int32 i = 0; i < localesCount; ++i)
    {
        String locale = locales[i];
        Map<String, Map<String, Font*> >::iterator findFontsIt = localizedFonts.find(locale);
        if(findFontsIt != localizedFonts.end())
        {
            Map<String, Font*> &localeFonts = findFontsIt->second;
            Logger::Debug("EditorFontManager::ClearLocalizedFonts locale=%s localeFonts.size()=%d", locale.c_str(), localeFonts.size());
            ClearFonts(localeFonts);
            localizedRegisteredFonts[locale].clear();
        }
    }
}

void EditorFontManager::ClearFonts(Map<String, Font*>& fonts)
{
    Map<String, Font*>::iterator it = fonts.begin();
    Map<String, Font*>::const_iterator endIt = fonts.end();
    for(; it != endIt; ++it)
    {
        SafeRelease(it->second);
    }
    fonts.clear();
}

void EditorFontManager::Reset()
{
	defaultFont = NULL;
	baseFont = NULL;

    FontManager::Instance()->Reset();
    
    ClearLocalizedFonts();
    
//	for (FONTSMAP::iterator iter = fonts.begin(); iter != fonts.end(); ++iter)
//	{
//        Font* font = iter->second;
//		SafeRelease(font);
//	}
    
    ResetLocalizedFontsPath();
}

Font* EditorFontManager::CreateDefaultFont(const String& fontPath, const String& fontName)
{
	Font* font = SafeRetain(GetLocalizedFont(fontName));
	if (font)
		return font;
	
    static const float defaultFontSize = 12.0f;
	font = FTFont::Create(fontPath);
	if (font)
	{
		font->SetSize(defaultFontSize);
		
//		fonts[fontName] = font;
//        FontManager::Instance()->RegisterFont(font);
        
        //If font was successfully loaded - emit the signal
        emit FontLoaded();
	}
	
	return font;
}

Font* EditorFontManager::GetDefaultFont() const
{
	return defaultFont ? defaultFont : baseFont;
}

void EditorFontManager::SetDefaultFont(Font *font)
{
	defaultFont = font->Clone();
}

void EditorFontManager::ResetDefaultFont()
{
	defaultFont = NULL;
}

void EditorFontManager::SetDefaultFontsPath(const FilePath& path)
{
    //TODO: use current locale
    defaultFontsPath = path;
}

const FilePath& EditorFontManager::GetDefaultFontsPath()
{
    return defaultFontsPath;
}

FilePath EditorFontManager::GetLocalizedFontsPath(const String &locale)
{
    FilePath localizedFontsPath(defaultFontsPath);
    //localizedFontsPath.ReplaceDirectory(defaultFontsPath.GetDirectory() + (LocalizationSystem::Instance()->GetCurrentLocale() + "/"));
    localizedFontsPath.ReplaceDirectory(defaultFontsPath.GetDirectory() + (locale + "/"));
    
    //TODO: get localized fonts path
    return localizedFontsPath;
}

void EditorFontManager::ResetLocalizedFontsPath()
{
    // TODO: reset localized fonts path (unload localized fonts)
    defaultFontsPath = FilePath();
}

//Font* EditorFontManager::GetFont(const String& name) const
//{
//	FONTSMAP::const_iterator iter = fonts.find(name);
//	if (iter != fonts.end())
//	{
//		return iter->second;
//	}
//	return NULL;
//}

const Map<String, Font*> &EditorFontManager::GetLocalizedFonts(const String& locale) const
{
    Map<String, Map<String, Font*> >::const_iterator findIt = localizedFonts.find(locale);
    if(findIt != localizedFonts.end())
    {
        return findIt->second;
    }
    return defaultFonts;
}

String EditorFontManager::GetLocalizedFontName(Font* font, const String& locale) const
{
    String fontName; //TODO: what if localized font is not found? try default?
    
    const Map<String, Font*> &fonts = GetLocalizedFonts(locale);
    Map<String, Font*>::const_iterator it = fonts.begin();
    Map<String, Font*>::const_iterator endIt = fonts.end();

    for(; it != endIt; ++it)
    {
        if(it->second == font)
        {
            fontName = it->first;
            break;
        }
    }
    return fontName;
}

Font* EditorFontManager::GetLocalizedFont(const String& fontName, const String& locale) const
{
    Font* font = NULL;
    const Map<String, Font*> &fonts = GetLocalizedFonts(locale);
    Map<String, Font*>::const_iterator findIt = fonts.find(fontName);
    Map<String, Font*>::const_iterator endIt = fonts.end();
    if(findIt != endIt)
    {
        font = findIt->second;
    }
    return font;
}

String EditorFontManager::SetLocalizedFont(const String& fontOriginalName, Font* font, const String& fontName, bool replaceExisting, const String& locale)
{
    String newFontName = fontName;
    if(!font)
    {
        Logger::Warning("EditorFontManager::SetLocalizedFont (locale=%s) attempted to set font=NULL for %s (former %s)", locale.c_str(), fontName.c_str(), fontOriginalName.c_str());
        return newFontName;
    }
    
    Map<String, Map<String, Font*> >::iterator findFontsIt = localizedFonts.find(locale);
    Map<String, Font*> *fonts = &defaultFonts;
    if(findFontsIt != localizedFonts.end())
    {
        fonts = &findFontsIt->second;
    }
    
    Vector<Font*> prevFonts;
    
    bool isSameFontName = (fontOriginalName == newFontName);
    
    Map<String, Font*>::const_iterator endIt = fonts->end();
    
    Map<String, Font*>::iterator findIt = fonts->find(newFontName);
    Font* newFont = SafeRetain(font);
    
    if(replaceExisting)
    {
        Map<String, Font*>::iterator findOriginalIt = fonts->find(fontOriginalName);
        while(findOriginalIt != endIt)
        {
            prevFonts.push_back(SafeRetain(findOriginalIt->second));
            if(!isSameFontName)
            {
                Logger::Debug("EditorFontManager::SetLocalizedFont (locale=%s) erase fonts[%s] = %x", locale.c_str(), findOriginalIt->first.c_str(), findOriginalIt->second);
            }
            SafeRelease(findOriginalIt->second);
            if(isSameFontName)
            {
                findOriginalIt->second = SafeRetain(newFont);
            }
            else
            {
                fonts->erase(findOriginalIt);
            }
            
            findOriginalIt = fonts->find(fontOriginalName);
        }
        
        if(!isSameFontName)
        {
            // rename existing font
            if(findIt != fonts->end())
            {
                Logger::Warning("EditorFontManager::SetLocalizedFont (locale=%s) name %s is already used by font %x (will be replaced by %x)",  locale.c_str(), findIt->first.c_str(), findIt->second, newFont);
                
                SafeRelease(findIt->second);
                findIt->second = newFont;
            }
            else
            {
                // add new font
                Logger::Debug("EditorFontManager::SetLocalizedFont (locale=%s) fonts[%s] = %x", locale.c_str(), newFontName.c_str(), newFont);
                (*fonts)[newFontName] = newFont;
            }
        }
    }
    else
    {
        // we are creating a copy of font, so generate a valid name for it
        if(findIt != fonts->end())
        {
            int32 nameIndex = 0;
            while(fonts->find(newFontName) != fonts->end())
            {
                newFontName = Format("%s_%d", fontName.c_str(), ++nameIndex);
            }
        }
        // add new font
        Logger::Debug("EditorFontManager::SetLocalizedFont (locale=%s) fonts[%s] = %x", locale.c_str(), newFontName.c_str(), newFont);
        (*fonts)[newFontName] = newFont;
    }
    
    //TODO: remove logs
    {
        const Map<String, Font*> &localeFonts = GetLocalizedFonts(locale);
        
        Map<String, Font*>::const_iterator lit = localeFonts.begin();
        Map<String, Font*>::const_iterator lendIt = localeFonts.end();
        for(; lit != lendIt; ++lit)
        {
            Logger::Debug("EditorFontManager::SetLocalizedFont localeFonts[%s]=%x", lit->first.c_str(), lit->second);
        }
    }
    
    //TODO: also set font to FontManager?
    if(replaceExisting)
    {
        int32 prevFontsCount = prevFonts.size();
        for(int32 i = 0; i < prevFontsCount; ++i)
        {
            if(prevFonts[i])
            {
                //TODO: for all texts that use prevFont set font
                Logger::Debug("EditorFontManager::SetLocalizedFont //TODO: for all texts that use prevFont set font");
                
                //FontManager::Instance()->UnregisterFont(prevFont);
                
                if(locale == "default")
                {
                    defaultRegisteredFonts[prevFonts[i]] = newFontName;
                    FontManager::Instance()->SetFontName(prevFonts[i], newFontName);
                    Logger::Debug("EditorFontManager::SetLocalizedFont FontManager::Instance()->SetFontName(prevFonts[%d]=%x, %s)", i, prevFonts[i], newFontName.c_str());
                }
                else
                {
                    localizedRegisteredFonts[locale][prevFonts[i]] = newFontName;
                }
                Logger::Debug("EditorFontManager::SetLocalizedFont set registered font %x name: %s", prevFonts[i], newFontName.c_str());
                
                SafeRelease(prevFonts[i]);
            }
        }
        
        // also rename all registered fonts that have fontOriginalName
        Map<Font*, String>::iterator drIt = defaultRegisteredFonts.begin();
        Map<Font*, String>::const_iterator drEndIt = defaultRegisteredFonts.end();
        for(; drIt != drEndIt; ++drIt)
        {
            if(drIt->second == fontOriginalName)
            {
                drIt->second = newFontName;
                Logger::Debug("EditorFontManager::SetLocalizedFont renamed: defaultRegisteredFonts[%x] = %s", drIt->first, drIt->second.c_str());
            }
        }
    }
    
    if(locale == "default")
    {
        defaultRegisteredFonts[newFont] = newFontName;
        FontManager::Instance()->RegisterFont(newFont);
        FontManager::Instance()->SetFontName(newFont, newFontName);
    }
    else
    {
        localizedRegisteredFonts[locale][newFont] = newFontName;
    }
    Logger::Debug("EditorFontManager::SetLocalizedFont (locale=%s) registered font %x with name %s", locale.c_str(), newFont, newFontName.c_str());
    
    return newFontName;
}

EditorFontManager::DefaultFontPath EditorFontManager::GetDefaultFontPath()
{
	FilePath defFontPath;
	FilePath defFontSpritePath;

	if (defaultFont)
	{
		Font::eFontType fontType = defaultFont->GetFontType();		
        switch (fontType)
        {
            case Font::TYPE_FT:
            {
                FTFont *ftFont = dynamic_cast<FTFont*>(defaultFont);
				FilePath ftFontPath = ftFont->GetFontPath();
				// Don't save standart default font
				if (ftFontPath.GetAbsolutePathname().find(DEFAULT_FONT_NAME) == String::npos)
				{
					// Set font path
					defFontPath = ftFontPath;
				}
                break;
            }
            case Font::TYPE_GRAPHICAL:
            {
                GraphicsFont *gFont = dynamic_cast<GraphicsFont*>(defaultFont);
                // Try to get font sprite
                Sprite *fontSprite = gFont->GetFontSprite();
				// Save font only if sprite is available
                if (fontSprite)
                {
					// Set font definition and sprite relative path
					defFontPath = gFont->GetFontDefinitionName();
					defFontSpritePath = fontSprite->GetRelativePathname();
                }
				break;
            }
        }	
	}
	
	return DefaultFontPath(defFontPath, defFontSpritePath);
}

void EditorFontManager::InitDefaultFontFromPath(const EditorFontManager::DefaultFontPath& defaultFontPath)
{
	FilePath fontPath = defaultFontPath.fontPath;
	FilePath fontSpritePath = defaultFontPath.fontSpritePath;
	Font* loadedFont = NULL;
	// Create font from loaded paths
	if (!fontPath.IsEmpty())
	{
		// Grpahics font
		if (!fontSpritePath.IsEmpty())
		{
            fontSpritePath.TruncateExtension();
			loadedFont = GraphicsFont::Create(fontPath, fontSpritePath);
		}
		else // True type font
		{
			loadedFont = FTFont::Create(fontPath);
		}
	}
	// Set default font only if font was really created
	if (loadedFont)
	{
		// Reset default font
		if (defaultFont)
		{
			defaultFont = NULL;
		}
		defaultFont = loadedFont;
	}	
}

QString EditorFontManager::GetDefaultFontName() const
{		
	if (defaultFont)
	{
		Font::eFontType fontType = defaultFont->GetFontType();
		switch (fontType)
		{
			case Font::TYPE_FT:
			{
				FTFont *ftFont = dynamic_cast<FTFont*>(defaultFont);
				return QString::fromStdString(ftFont->GetFontPath().GetAbsolutePathname());
			}
			case Font::TYPE_GRAPHICAL:
			{
				GraphicsFont *gFont = dynamic_cast<GraphicsFont*>(defaultFont);
				return QString::fromStdString(gFont->GetFontDefinitionName().GetAbsolutePathname());
			}
		}
	}
	return QString::fromStdString(DEFAULT_FONT_PATH);
}

//const EditorFontManager::FONTSMAP& EditorFontManager::GetAllFonts() const
//{
//	return fonts;
//}

//QString EditorFontManager::GetFontName(Font* font) const
//{
//    if (font == NULL)
//    {
//        return QString();
//    }
//
//    EditorFontManager::FONTSMAP fonts = GetAllFonts();
//    for (EditorFontManager::FONTSMAP::const_iterator iter = fonts.begin(); iter != fonts.end(); ++iter)
//    {
//        if (font->IsEqual(iter->second))
//        {
//            return iter->first.c_str();
//        }
//    }
//
//    return QString();
//}
