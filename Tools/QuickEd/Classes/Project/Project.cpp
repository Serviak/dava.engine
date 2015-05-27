#include "DAVAEngine.h"

#include <QDir>
#include <QApplication>
#include <QMessageBox>

#include "Project.h"
#include "EditorFontSystem.h"
#include "UI/UIPackageLoader.h"
#include "Model/EditorUIPackageBuilder.h"
#include "Model/LegacyEditorUIPackageLoader.h"
#include "Model/YamlPackageSerializer.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Helpers/ResourcesManageHelper.h"

using namespace DAVA;

Project::Project(QObject *parent)
    : QObject(parent)
    , editorFontSystem(new EditorFontSystem(this))
    , editorLocalizationSystem(new EditorLocalizationSystem(this))
    , isOpen(false)
{
    legacyData = new LegacyControlData();
}

Project::~Project()
{
    SafeRelease(legacyData);
}

bool Project::Open(const QString &path)
{
    bool result = OpenInternal(path);
    SetIsOpen(result);
    return result;
}

bool Project::OpenInternal(const QString &path)
{
    // Attempt to create a project
    YamlParser* parser = YamlParser::Create(path.toStdString());
    if (nullptr == parser)
        return false;

    QDir dir(path);
    dir.cdUp();

    YamlNode* projectRoot = parser->GetRootNode();
    if (nullptr == projectRoot)
    {
        SafeRelease(parser);
        return false;
    }

    FilePath::RemoveResourcesFolder(projectPath);
    editorLocalizationSystem->Cleanup();

    projectPath = dir.absolutePath().toStdString();
    projectPath.MakeDirectoryPathname();

    const auto &resFolders = FilePath::GetResourcesFolders();
    const auto &searchIt = find(resFolders.begin(), resFolders.end(), projectPath);

    if (searchIt == resFolders.end())
    {
        FilePath::AddResourcesFolder(projectPath);
    }

    const YamlNode *fontNode = projectRoot->Get("font");

    // Get font node
    if (nullptr != fontNode)
    {
        // Get default font node
        const YamlNode *defaultFontPath = fontNode->Get("DefaultFontsPath");
        if (nullptr != defaultFontPath)
        {
            FilePath localizationFontsPath(defaultFontPath->AsString());
            if (localizationFontsPath.Exists())
            {
                editorFontSystem->SetDefaultFontsPath(localizationFontsPath.GetDirectory());
            }
        }
    }

    if (editorFontSystem->GetDefaultFontsPath().IsEmpty())
    {
        editorFontSystem->SetDefaultFontsPath(FilePath(projectPath.GetAbsolutePathname() + "Data/UI/Fonts/"));
    }

    editorFontSystem->LoadLocalizedFonts();

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
        for (int j = 0; j < (int32)screens->GetCount(); j++)
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
        for (int j = 0; j < (int32)aggregators->GetCount(); j++)
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
                editorLocalizationSystem->InitLanguageWithDirectory(localizationPathNode->AsString(), localeNode->AsString());
            }
        }
    }

    SafeRelease(parser);

    return true;
}

bool Project::CheckAndUnlockProject(const QString& projectPath)
{
    if (!FileSystem::Instance()->IsFileLocked(projectPath.toStdString()))
    {
        // Nothing to unlock.
        return true;
    }

    if (QMessageBox::question(qApp->activeWindow(), tr("File is locked!"), tr("The project file %1 is locked by other user. Do you want to unlock it?")) == QMessageBox::No)
    {
        return false;
    }

    // Check whether it is possible to unlock project file.
    if (!FileSystem::Instance()->LockFile(projectPath.toStdString(), false))
    {
        QMessageBox::critical(qApp->activeWindow(), tr("Unable to unlock project file!"),
            tr("Unable to unlock project file %1. Please check whether the project is opened in another QuickEd and close it, if yes.").arg(projectPath));
        return false;
    }

    return true;
}

DAVA::RefPtr<PackageNode> Project::NewPackage(const QString &path)
{
    return DAVA::RefPtr<PackageNode>();
}

RefPtr<PackageNode> Project::OpenPackage(const QString &packagePath)
{
    FilePath path(packagePath.toStdString());
    String fwPath = path.GetFrameworkPath();

    EditorUIPackageBuilder builder;
    
    bool packageLoaded = UIPackageLoader().LoadPackage(path, &builder);
    if (!packageLoaded)
        packageLoaded = LegacyEditorUIPackageLoader(legacyData).LoadPackage(path, &builder);

    if (packageLoaded)
        return builder.BuildPackage();
    
    return RefPtr<PackageNode>();
}

bool Project::SavePackage(PackageNode *package)
{
    YamlPackageSerializer serializer;
    package->Serialize(&serializer);
    serializer.WriteToFile(package->GetPath());
    return true;
}

bool Project::IsOpen() const
{
    return isOpen;
}

void Project::SetIsOpen(bool arg)
{
    if (isOpen == arg)
    {
        //return; //TODO: implement this after we create CloseProject function
    }
    isOpen = arg;
    if (arg)
    {
        ResourcesManageHelper::SetProjectPath(QString::fromStdString(projectPath.GetAbsolutePathname()));
    }
    emit IsOpenChanged(arg);
}