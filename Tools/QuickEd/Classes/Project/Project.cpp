#include "Project.h"

#include "Document/Document.h"
#include "Document/DocumentGroup.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/YamlPackageSerializer.h"
#include "Project/EditorFontSystem.h"
#include "Project/EditorLocalizationSystem.h"
#include "UI/ProjectView.h"

#include "QtTools/ReloadSprites/SpritesPacker.h"
#include "QtTools/ProjectInformation/FileSystemCache.h"
#include "QtTools/FileDialogs/FindFileDialog.h"

#include "Engine/Engine.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/YamlEmitter.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlParser.h"
#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/UIControlSystem.h"
#include "Utils/Utils.h"

#include <QDir>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

using namespace DAVA;

class Project::SymLinkResolver
{
public:
    void Init(const QString &directory)
    {
        symlinks = FindSymLinks(directory);
    }
        
    QString ResolveSymLink(const QString &origFilePath) const
    {
        QString filePath = origFilePath;
        for(const auto &item : SymLinkResolver::symlinks.toStdVector())
        {
            if(filePath.startsWith(item.first))
            {
                filePath.replace(item.first, item.second);
                return ResolveSymLink(filePath);
            }
        }
        return filePath;
    }

private:
    QVector<QPair<QString, QString>> FindSymLinks(const QString &absDirPath)
    {
        QVector<QPair<QString, QString>> symlinks;
        QDirIterator dirIterator(absDirPath, QDir::NoDotAndDotDot | QDir::Dirs | QDir::Hidden, QDirIterator::Subdirectories);
        while (dirIterator.hasNext())
        {
            dirIterator.next();
            QFileInfo fileInfo(dirIterator.fileInfo());

            if (fileInfo.isSymLink())
            {
                symlinks.push_back(qMakePair(fileInfo.symLinkTarget(), fileInfo.absoluteFilePath()));
                symlinks += FindSymLinks(fileInfo.symLinkTarget());
            }
        }
            
        return symlinks;
    }
        
    QVector<QPair<QString, QString>> symlinks;
};

QString Project::ResolveFilePath(const QString &filePath) const
{
#if defined(__DAVAENGINE_MACOS__)
    return symLinkResolver->ResolveSymLink(filePath);
#endif
    
    return filePath;
}

Project::Project(MainWindow::ProjectView* view_, const ProjectProperties& properties_)
    : QObject(nullptr)
    , properties(properties_)
    , projectDirectory(QString::fromStdString(properties_.GetProjectDirectory().GetStringValue()))
    , projectName(QString::fromStdString(properties_.GetProjectFile().GetFilename()))
    , view(view_)
    , editorFontSystem(new EditorFontSystem(this))
    , editorLocalizationSystem(new EditorLocalizationSystem(this))
    , documentGroup(new DocumentGroup(this, view->GetDocumentGroupView()))
    , spritesPacker(new SpritesPacker())
    , projectStructure(new FileSystemCache(QStringList() << "yaml"))
#if defined(__DAVAENGINE_MACOS__)
    , symLinkResolver(new SymLinkResolver())
#endif
{
    DAVA::FileSystem* fileSystem = DAVA::Engine::Instance()->GetContext()->fileSystem;
    if (fileSystem->IsDirectory(properties.GetAdditionalResourceDirectory().absolute))
    {
        FilePath::AddResourcesFolder(properties.GetAdditionalResourceDirectory().absolute);
    }

    FilePath::AddResourcesFolder(properties.GetConvertedResourceDirectory().absolute);
    FilePath::AddResourcesFolder(properties.GetResourceDirectory().absolute);

    if (!properties.GetFontsConfigsDirectory().absolute.IsEmpty()) //for support legacy empty project
    {
        editorFontSystem->SetDefaultFontsPath(properties.GetFontsConfigsDirectory().absolute);
        editorFontSystem->LoadLocalizedFonts();
    }

    if (!properties.GetTextsDirectory().absolute.IsEmpty()) //support legacy empty project
    {
        editorLocalizationSystem->SetDirectory(QDir(QString::fromStdString(properties.GetTextsDirectory().absolute.GetStringValue())));
    }

    if (!properties.GetDefaultLanguage().empty()) //support legacy empty project
    {
        editorLocalizationSystem->SetCurrentLocale(QString::fromStdString(properties.GetDefaultLanguage()));
    }

    FilePath uiDirectory = properties.GetUiDirectory().absolute;
    DVASSERT(fileSystem->IsDirectory(uiDirectory));
    uiResourcesPath = QString::fromStdString(uiDirectory.GetStringValue());

    projectStructure->TrackDirectory(uiResourcesPath);
    view->SetResourceDirectory(uiResourcesPath);

    view->SetProjectActionsEnabled(true);
    view->SetProjectPath(GetProjectPath());
    view->SetLanguages(GetAvailableLanguages(), GetCurrentLanguage());

    connect(editorLocalizationSystem.get(), &EditorLocalizationSystem::CurrentLocaleChanged, this, &Project::CurrentLanguageChanged, Qt::DirectConnection);
    connect(editorFontSystem.get(), &EditorFontSystem::FontPresetChanged, documentGroup.get(), &DocumentGroup::FontPresetChanged, Qt::DirectConnection);

    connect(view, &MainWindow::ProjectView::CurrentLanguageChanged, this, &Project::SetCurrentLanguage);
    connect(view, &MainWindow::ProjectView::RtlChanged, this, &Project::SetRtl);
    connect(view, &MainWindow::ProjectView::BiDiSupportChanged, this, &Project::SetBiDiSupport);
    connect(view, &MainWindow::ProjectView::GlobalStyleClassesChanged, this, &Project::SetGlobalStyleClasses);
    connect(view, &MainWindow::ProjectView::ReloadSprites, this, &Project::OnReloadSprites);
    connect(view, &MainWindow::ProjectView::FindFileInProject, this, &Project::OnFindFileInProject);

    connect(this, &Project::CurrentLanguageChanged, view, &MainWindow::ProjectView::SetCurrentLanguage);

    connect(spritesPacker.get(), &SpritesPacker::Finished, this, &Project::OnReloadSpritesFinished);

    spritesPacker->ClearTasks();
    for (const auto& gfxOptions : properties.GetGfxDirectories())
    {
        QDir gfxDirectory(QString::fromStdString(gfxOptions.directory.absolute.GetStringValue()));
        DVASSERT(gfxDirectory.exists());

        FilePath gfxOutDir = properties.GetConvertedResourceDirectory().absolute + gfxOptions.directory.relative;
        QDir gfxOutDirectory(QString::fromStdString(gfxOutDir.GetStringValue()));

        spritesPacker->AddTask(gfxDirectory, gfxOutDirectory);
    }

#if defined(__DAVAENGINE_MACOS__)
    symLinkResolver->Init(QString::fromStdString(properties.GetResourceDirectory().absolute.GetStringValue()));
#endif
}

Project::~Project()
{
    view->SetLanguages(QStringList(), QString());
    view->SetProjectPath(QString());
    view->SetProjectActionsEnabled(false);

    projectStructure->UntrackDirectory(uiResourcesPath);
    view->SetResourceDirectory(QString());

    editorLocalizationSystem->Cleanup();
    editorFontSystem->ClearAllFonts();
    FilePath::RemoveResourcesFolder(properties.GetResourceDirectory().absolute);
    FilePath::RemoveResourcesFolder(properties.GetAdditionalResourceDirectory().absolute);
    FilePath::RemoveResourcesFolder(properties.GetConvertedResourceDirectory().absolute);
}

Vector<ProjectProperties::ResDir> Project::GetLibraryPackages() const
{
    return properties.GetLibraryPackages();
}

std::tuple<ResultList, ProjectProperties> Project::ParseProjectPropertiesFromFile(const QString& projectFile)
{
    ResultList resultList;

    RefPtr<YamlParser> parser(YamlParser::Create(projectFile.toStdString()));
    if (parser.Get() == nullptr)
    {
        QString message = tr("Can not parse project file %1.").arg(projectFile);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());

        return std::make_tuple(resultList, ProjectProperties());
    }

    return ProjectProperties::Parse(projectFile.toStdString(), parser->GetRootNode());
}

bool Project::EmitProjectPropertiesToFile(const ProjectProperties& properties)
{
    RefPtr<YamlNode> node = ProjectProperties::Emit(properties);

    return YamlEmitter::SaveToYamlFile(properties.GetProjectFile(), node.Get());
}

const QStringList& Project::GetFontsFileExtensionFilter()
{
    static const QStringList filter(QStringList() << "*.ttf"
                                                  << "*.otf"
                                                  << "*.fon"
                                                  << "*.fnt"
                                                  << "*.def"
                                                  << "*.df");
    return filter;
}

const QString& Project::GetGraphicsFileExtension()
{
    static const QString filter(".psd");
    return filter;
}

const QString& Project::Get3dFileExtension()
{
    static const QString filter(".sc2");
    return filter;
}

const QString& Project::GetUiFileExtension()
{
    static const QString extension(".yaml");
    return extension;
}

QStringList Project::GetAvailableLanguages() const
{
    return editorLocalizationSystem->GetAvailableLocales();
}

QString Project::GetCurrentLanguage() const
{
    return editorLocalizationSystem->GetCurrentLocale();
}

void Project::SetCurrentLanguage(const QString& newLanguageCode)
{
    editorLocalizationSystem->SetCurrentLocale(newLanguageCode);
    editorFontSystem->RegisterCurrentLocaleFonts();

    documentGroup->LanguageChanged();
}

const QStringList& Project::GetDefaultPresetNames() const
{
    return editorFontSystem->GetDefaultPresetNames();
}

EditorFontSystem* Project::GetEditorFontSystem() const
{
    return editorFontSystem.get();
}

void Project::SetRtl(bool isRtl)
{
    UIControlSystem::Instance()->SetRtl(isRtl);

    documentGroup->RtlChanged();
}

void Project::SetBiDiSupport(bool support)
{
    UIControlSystem::Instance()->SetBiDiSupportEnabled(support);

    documentGroup->BiDiSupportChanged();
}

void Project::SetGlobalStyleClasses(const QString& classesStr)
{
    Vector<String> tokens;
    Split(classesStr.toStdString(), " ", tokens);

    UIControlSystem::Instance()->GetStyleSheetSystem()->ClearGlobalClasses();
    for (String& token : tokens)
    {
        UIControlSystem::Instance()->GetStyleSheetSystem()->AddGlobalClass(FastName(token));
    }

    documentGroup->GlobalStyleClassesChanged();
}

QString Project::GetResourceDirectory() const
{
    return QString::fromStdString(properties.GetResourceDirectory().absolute.GetStringValue());
}

void Project::OnReloadSprites()
{
    if (!documentGroup->TryCloseAllDocuments())
    {
        return;
    }

    view->ExecDialogReloadSprites(spritesPacker.get());
}

void Project::OnReloadSpritesFinished()
{
    Sprite::ReloadSprites();
}

bool Project::TryCloseAllDocuments()
{
    bool hasUnsaved = documentGroup->HasUnsavedDocuments();

    if (hasUnsaved)
    {
        int ret = QMessageBox::question(
        view->mainWindow,
        tr("Save changes"),
        tr("Some files has been modified.\n"
           "Do you want to save your changes?"),
        QMessageBox::SaveAll | QMessageBox::NoToAll | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
        {
            return false;
        }
        else if (ret == QMessageBox::SaveAll)
        {
            documentGroup->SaveAllDocuments();
        }
    }

    documentGroup->CloseAllDocuments();

    return true;
}

void Project::OnFindFileInProject()
{
    QString filePath = FindFileDialog::GetFilePath(projectStructure.get(), "yaml", view->mainWindow);
    if (filePath.isEmpty())
    {
        return;
    }
    view->SelectFile(filePath);
    documentGroup->AddDocument(filePath);
}

void Project::SetAssetCacheClient(DAVA::AssetCacheClient* newCacheClient)
{
    spritesPacker->SetCacheClient(newCacheClient, "QuickEd.ReloadSprites");
}

QString Project::GetProjectPath() const
{
    return QString::fromStdString(properties.GetProjectFile().GetStringValue());
}

const QString& Project::GetProjectDirectory() const
{
    return projectDirectory;
}

const QString& Project::GetProjectName() const
{
    return projectName;
}
