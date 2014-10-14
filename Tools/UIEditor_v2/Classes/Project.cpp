//
//  Project.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 11.9.14.
//
//

#include "Project.h"
#include "DAVAEngine.h"
#include "EditorFontManager.h"
#include "UI/UIPackageLoader.h"
#include "UI/DefaultUIPackageBuilder.h"
#include "UIControls/EditorUIPackageBuilder.h"
#include "UIControls/LegacyEditorUIPackageLoader.h"
#include "UIControls/PackageHierarchy/PackageNode.h"

#include <QDir>

using namespace DAVA;

Project::Project()
{
    legacyData = new LegacyControlData();
}

Project::~Project()
{
    SafeRelease(legacyData);
}

bool Project::Open(const QString &path)
{
    // Attempt to create a project
	YamlParser* parser = YamlParser::Create(path.toStdString());
	if (!parser)
		return false;
    
    projectFile = path;
    QDir dir(path);
    dir.cdUp();
    projectDir = dir.absolutePath();
    
	YamlNode* projectRoot = parser->GetRootNode();
	if (!projectRoot)
	{
		SafeRelease(parser);
		return false;
	}
    
    // Build the list of file names to be locked.
    List<QString> fileNames;
    fileNames.push_back(path);
    
    LocalizationSystem::Instance()->Cleanup();
    
    FilePath bundleName(projectDir.toStdString());
    bundleName.MakeDirectoryPathname();
    
    List<FilePath> resFolders = FilePath::GetResourcesFolders();
    List<FilePath>::const_iterator searchIt = find(resFolders.begin(), resFolders.end(), bundleName);
    
    if(searchIt == resFolders.end())
    {
        FilePath::AddResourcesFolder(bundleName);
    }
    
    EditorFontManager::Instance()->SetProjectDataPath(bundleName.GetAbsolutePathname() + "Data/");
    
    const YamlNode *font = projectRoot->Get("font");
    
    // Get font node
	if (font)
	{
		// Get default font node
		const YamlNode *fontPath = font->Get("DefaultFontPath");
		if (fontPath)
		{
			// Get font values into array
			const Vector<YamlNode*> &fontPathArray = fontPath->AsVector();
			EditorFontManager::DefaultFontPath defaultFontPath("", "");
			// True type font
			if (fontPathArray.size() == 1)
			{
				defaultFontPath.fontPath = FilePath(fontPathArray[0]->AsString());
			}
			else if (fontPathArray.size() == 2) // Graphics font
			{
				defaultFontPath.fontPath = FilePath(fontPathArray[0]->AsString());
				defaultFontPath.fontSpritePath = FilePath(fontPathArray[1]->AsString());
			}
			EditorFontManager::Instance()->InitDefaultFontFromPath(defaultFontPath);
		}
        
        const YamlNode *localizationFontsPathNode = font->Get("DefaultFontsPath");
        if(localizationFontsPathNode)
        {
            FilePath localizationFontsPath(localizationFontsPathNode->AsString());
            if(localizationFontsPath.Exists())
            {
                EditorFontManager::Instance()->SetDefaultFontsPath(localizationFontsPath.GetAbsolutePathname());
            }
            else
            {
                EditorFontManager::Instance()->SetDefaultFontsPath(bundleName.GetAbsolutePathname() + "Data" + localizationFontsPath.GetAbsolutePathname().substr(5));
            }
        }
    }
    
    if(EditorFontManager::Instance()->GetDefaultFontsPath().IsEmpty())
    {
        EditorFontManager::Instance()->SetDefaultFontsPath(FilePath(bundleName.GetAbsolutePathname() + "Data/UI/Fonts/fonts.yaml"));
    }
    EditorFontManager::Instance()->LoadLocalizedFonts();
    FontManager::Instance()->PrepareToSaveFonts(true);
    
	const YamlNode* platforms = projectRoot->Get("platforms");
	for (uint32 i = 0; i < platforms->GetCount(); i++)
	{
		const String &platformName = platforms->GetItemKeyName(i);
        if (platformName.empty())
            continue;
        const YamlNode *platform = platforms->Get(platformName);
        float platformWidth = platform->Get("width")->AsFloat();
        float platformHeight = platform->Get("height")->AsFloat();
        
        const YamlNode *screens = platform->Get("screens");
        for (int j = 0; j < (int32) screens->GetCount(); j++)
        {
            const String &screenName = screens->Get(j)->AsString();
            LegacyControlData::Data data;
            data.name = screenName;
            data.isAggregator = false;
            data.size = Vector2(platformWidth, platformHeight);
            String key = "~res:/UI/" + platformName + "/" + screenName + ".yaml";
            legacyData->Put(key, data);
        }
        
        const YamlNode *aggregators = platform->Get("aggregators");
        for (int j = 0; j < (int32) aggregators->GetCount(); j++)
        {
            String aggregatorName = aggregators->GetItemKeyName(j);
            const YamlNode *aggregator = aggregators->Get(j);
            float aggregatorWidth = aggregator->Get("width")->AsFloat();
            float aggregatorHeight = aggregator->Get("height")->AsFloat();

            LegacyControlData::Data data;
            data.name = aggregatorName;
            data.isAggregator = false;
            data.size = Vector2(aggregatorWidth, aggregatorHeight);
            String key = "~res:/UI/" + platformName + "/" + aggregatorName + ".yaml";
            legacyData->Put(key, data);
        }
        
        if (i == 0)
        {
            const YamlNode *localizationPathNode = platform->Get("LocalizationPath");
            const YamlNode *localeNode = platform->Get("Locale");
            if (localizationPathNode && localeNode)
            {
                LocalizationSystem::Instance()->SetDirectory(localizationPathNode->AsString());
                LocalizationSystem::Instance()->SetCurrentLocale(localeNode->AsString());
                LocalizationSystem::Instance()->Init();
            }
        }
	}
    

    SafeRelease(parser);
    
    return true;
}

PackageNode *Project::OpenPackage(const QString &packagePath)
{
    FilePath path(packagePath.toStdString());
    String fwPath = path.GetFrameworkPath();

    //UIPackage *newPackage = LegacyEditorUIPackageLoader(legacyData).LoadPackage(path);
    //DefaultUIPackageBuilder builder;
    EditorUIPackageBuilder builder;
    UIPackage *newPackage = UIPackageLoader(&builder).LoadPackage(path);
    SafeRelease(newPackage);
    PackageNode *node = builder.GetPackageNode();
    return node;
}

bool Project::SavePackage(PackageNode *package)
{
    YamlNode *node = package->Serialize();
    YamlEmitter::SaveToYamlFile(package->GetPackage()->GetFilePath(), node);
    SafeRelease(node);
    return true;
}

