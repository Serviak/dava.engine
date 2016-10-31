#include "EditorCore.h"

#include "UI/mainwindow.h"
#include "Document/DocumentGroup.h"
#include "Document/Document.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ReloadSprites/SpritesPacker.h"
#include "QtTools/Utils/Utils.h"

#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/UIControlSystem.h"
#include "Utils/Utils.h"
#include "UI/FileSystemView/FileSystemModel.h"
#include "UI/Package/PackageModel.h"
#include "QtTools/FileDialogs/FileDialog.h"
#include "Base/Result.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/FileSystem.h"
#include "Project/Project.h"
#include "UI/FileSystemView/FileSystemDockWidget.h"
#include "UI/Library/LibraryWidget.h"
#include "UI/Preview/PreviewWidget.h"
#include "UI/Package/PackageWidget.h"
#include "UI/Properties/PropertiesWidget.h"

#include "DAVAVersion.h"
#include "TextureCompression/PVRConverter.h"
#include "Engine/Engine.h"
#include "UI/Layouts/UILayoutSystem.h"
#include "UI/Input/UIInputSystem.h"
#include "QtTools/Utils/MessageHandler.h"
#include "QtTools/Utils/Themes/Themes.h"
#include "Engine/Qt/NativeServiceQt.h"
#include "version.h"

using namespace DAVA;

namespace EditorCoreDetails
{
static const char* EDITOR_TITLE = "DAVA Framework - QuickEd | %s-%s [%u bit]";
static const DAVA::String DOCUMENTATION_DIRECTORY("~doc:/Help/");
//static const DAVA::String EDITOR_TITLE("QuickEd");

void InitPVRTexTool()
{
#if defined(__DAVAENGINE_MACOS__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI";
#elif defined(__DAVAENGINE_WIN32__)
    const DAVA::String pvrTexToolPath = "~res:/PVRTexToolCLI.exe";
#endif
    DAVA::PVRConverter::Instance()->SetPVRTexTool(pvrTexToolPath);
}
};

REGISTER_PREFERENCES_ON_START(EditorCore,
                              PREF_ARG("isUsingAssetCache", false),
                              PREF_ARG("projectsHistory", DAVA::String()),
                              PREF_ARG("projectsHistorySize", static_cast<DAVA::uint32>(5))
                              )

EditorCore::EditorCore(DAVA::Engine& engine)
    : QObject()
    , cacheClient()
{
    using namespace DAVA;
    EngineContext* context = engine.GetContext();

    FileSystem* fs = context->fileSystem;
    fs->SetCurrentDocumentsDirectory(fs->GetUserDocumentsPath() + "QuickEd/");
    fs->CreateDirectory(fs->GetCurrentDocumentsDirectory(), true);

    UIControlSystem* uiControlSystem = context->uiControlSystem;
    uiControlSystem->GetLayoutSystem()->SetAutoupdatesEnabled(false);

    UIInputSystem* inputSystem = uiControlSystem->GetInputSystem();
    inputSystem->BindGlobalShortcut(KeyboardShortcut(Key::LEFT), UIInputSystem::ACTION_FOCUS_LEFT);
    inputSystem->BindGlobalShortcut(KeyboardShortcut(Key::RIGHT), UIInputSystem::ACTION_FOCUS_RIGHT);
    inputSystem->BindGlobalShortcut(KeyboardShortcut(Key::UP), UIInputSystem::ACTION_FOCUS_UP);
    inputSystem->BindGlobalShortcut(KeyboardShortcut(Key::DOWN), UIInputSystem::ACTION_FOCUS_DOWN);

    inputSystem->BindGlobalShortcut(KeyboardShortcut(Key::TAB), UIInputSystem::ACTION_FOCUS_NEXT);
    inputSystem->BindGlobalShortcut(KeyboardShortcut(Key::TAB, UIEvent::Modifier::SHIFT_DOWN), UIInputSystem::ACTION_FOCUS_PREV);

    context->logger->SetLogFilename("QuickEd.txt");

    Renderer::SetDesiredFPS(60);

    const char* settingsPath = "QuickEdSettings.archive";
    FilePath localPrefrencesPath(fs->GetCurrentDocumentsDirectory() + settingsPath);
    PreferencesStorage::Instance()->SetupStoragePath(localPrefrencesPath);

    EditorCoreDetails::InitPVRTexTool();

    qInstallMessageHandler(DAVAMessageHandler);

    Q_INIT_RESOURCE(QtToolsResources);
    Themes::InitFromQApplication();

    mainWindow.reset(new MainWindow());

    DAVA::RenderWidget* renderWidget = engine.GetNativeService()->GetRenderWidget();
    mainWindow->InjectRenderWidget(renderWidget);

    //we need to register preferences when whole class is initialized
    PreferencesStorage::Instance()->RegisterPreferences(this);

    mainWindow->setWindowIcon(QIcon(":/icon.ico"));
    mainWindow->SetRecentProjects(GetRecentProjects());

    mainWindow->SetEditorTitle(ReadEditorTitle());

    connect(mainWindow.get(), &MainWindow::CanClose, this, &EditorCore::CloseProject);
    connect(mainWindow.get(), &MainWindow::NewProject, this, &EditorCore::OnNewProject);
    connect(mainWindow.get(), &MainWindow::OpenProject, this, &EditorCore::OnOpenProject);
    connect(mainWindow.get(), &MainWindow::RecentProject, this, &EditorCore::OpenProject);
    connect(mainWindow.get(), &MainWindow::CloseProject, this, &EditorCore::OnCloseProject);
    connect(mainWindow.get(), &MainWindow::Exit, this, &EditorCore::OnExit);
    connect(mainWindow.get(), &MainWindow::ShowHelp, this, &EditorCore::OnShowHelp);

    UnpackHelp();

    mainWindow->show();
}

EditorCore::~EditorCore()
{
    DVASSERT(project == nullptr);

    DisableCacheClient();

    PreferencesStorage::Instance()->UnregisterPreferences(this);
}

void EditorCore::OnRenderingInitialized()
{
    mainWindow->OnWindowCreated();
    QString lastProjectPath = GetLastProject();
    if (!lastProjectPath.isEmpty())
    {
        OpenProject(lastProjectPath);
    }
}

void EditorCore::OnShowHelp()
{
    FilePath docsPath = EditorCoreDetails::DOCUMENTATION_DIRECTORY + "index.html";
    QString docsFile = QString::fromStdString("file:///" + docsPath.GetAbsolutePathname());
    QDesktopServices::openUrl(QUrl(docsFile));
}

void EditorCore::OpenProject(const QString& path)
{
    if (!CloseProject())
    {
        return;
    }

    ResultList resultList;
    std::unique_ptr<Project> newProject;

    std::tie(newProject, resultList) = CreateProject(path, mainWindow.get());

    if (newProject.get())
    {
        project = std::move(newProject);
        AddRecentProject(project->GetProjectPath());

        mainWindow->SetRecentProjects(GetRecentProjects());

        connect(this, &EditorCore::AssetCacheChanged, project.get(), &Project::SetAssetCacheClient);
        connect(this, &EditorCore::TryCloseDocuments, project.get(), &Project::TryCloseAllDocuments);

        if (assetCacheEnabled)
        {
            EnableCacheClient();
        }

        project->SetAssetCacheClient(cacheClient.get());
    }

    mainWindow->ShowResultList(tr("Error while loading project"), resultList);
}

std::tuple<std::unique_ptr<Project>, ResultList> EditorCore::CreateProject(const QString& path, MainWindow* mainWindow)
{
    ResultList resultList;

    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        QString message = tr("%1 does not exist.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        return std::make_tuple(nullptr, resultList);
    }

    if (!fileInfo.isFile())
    {
        QString message = tr("%1 is not a file.").arg(path);
        resultList.AddResult(Result::RESULT_ERROR, message.toStdString());
        return std::make_tuple(nullptr, resultList);
    }

    Project::Settings settings;
    std::tie(settings, resultList) = Project::ParseProjectSettings(path);
    if (settings.projectFile.isEmpty())
    {
        return std::make_tuple(nullptr, resultList);
    }

    return std::make_tuple(std::make_unique<Project>(mainWindow->GetProjectView(), settings), resultList);
}

std::tuple<QString, DAVA::ResultList> EditorCore::CreateNewProject()
{
    DAVA::ResultList resultList;

    QString projectDirPath = QFileDialog::getExistingDirectory(qApp->activeWindow(), tr("Select directory for new project"));
    if (projectDirPath.isEmpty())
    {
        return std::make_tuple(QString(), resultList);
    }
    bool needOverwriteProjectFile = true;
    QDir projectDir(projectDirPath);
    const QString projectFileName = Project::GetProjectFileName();
    QString fullProjectFilePath = projectDir.absoluteFilePath(projectFileName);
    if (QFile::exists(fullProjectFilePath))
    {
        resultList.AddResult(Result::RESULT_FAILURE, String("Project file exists!"));
        return std::make_tuple(QString(), resultList);
    }
    QFile projectFile(fullProjectFilePath);
    if (!projectFile.open(QFile::WriteOnly | QFile::Truncate)) // create project file
    {
        resultList.AddResult(Result::RESULT_ERROR, String("Can not open project file ") + fullProjectFilePath.toUtf8().data());
        return std::make_tuple(QString(), resultList);
    }
    if (!projectDir.mkpath(projectDir.canonicalPath() + Project::GetUIRelativePath()))
    {
        resultList.AddResult(Result::RESULT_ERROR, String("Can not create Data/UI folder"));
        return std::make_tuple(QString(), resultList);
    }

    return std::make_tuple(fullProjectFilePath, resultList);
}

void EditorCore::OnCloseProject()
{
    CloseProject();
}

bool EditorCore::CloseProject()
{
    if (project == nullptr)
    {
        return true;
    }

    if (!TryCloseDocuments())
    {
        return false;
    }

    disconnect(this, &EditorCore::AssetCacheChanged, project.get(), &Project::SetAssetCacheClient);
    disconnect(this, &EditorCore::TryCloseDocuments, project.get(), &Project::TryCloseAllDocuments);

    DisableCacheClient();
    project = nullptr;
    return true;
}

void EditorCore::OnExit()
{
    if (CloseProject())
    {
        qApp->quit();
    }
}

bool EditorCore::IsUsingAssetCache() const
{
    return assetCacheEnabled;
}

void EditorCore::SetUsingAssetCacheEnabled(bool enabled)
{
    if (enabled)
    {
        EnableCacheClient();
        assetCacheEnabled = true;
    }
    else
    {
        DisableCacheClient();
        assetCacheEnabled = false;
    }
}

void EditorCore::EnableCacheClient()
{
    DisableCacheClient();
    cacheClient.reset(new AssetCacheClient());
    DAVA::AssetCache::Error connected = cacheClient->ConnectSynchronously(connectionParams);
    if (connected != AssetCache::Error::NO_ERRORS)
    {
        cacheClient.reset();
        Logger::Warning("Asset cache client was not started! Error №%d", connected);
    }
    else
    {
        Logger::Info("Asset cache client started");
        emit AssetCacheChanged(cacheClient.get());
    }
}

void EditorCore::DisableCacheClient()
{
    if (cacheClient != nullptr && cacheClient->IsConnected())
    {
        cacheClient->Disconnect();
        cacheClient.reset();
        emit AssetCacheChanged(cacheClient.get());
    }
}

const QStringList& EditorCore::GetRecentProjects() const
{
    return recentProjects;
}

QString EditorCore::GetLastProject() const
{
    if (!recentProjects.empty())
    {
        return recentProjects.last();
    }

    return QString();
}

void EditorCore::AddRecentProject(const QString& projectPath)
{
    recentProjects.removeAll(projectPath);
    recentProjects += projectPath;
    while (static_cast<DAVA::uint32>(recentProjects.size()) > projectsHistorySize)
    {
        recentProjects.removeFirst();
    }
}

String EditorCore::GetRecentProjectsAsString() const
{
    return recentProjects.join('\n').toStdString();
}

void EditorCore::SetRecentProjectsFromString(const String& history)
{
    recentProjects = QString::fromStdString(history).split("\n", QString::SkipEmptyParts);
}

void EditorCore::OnOpenProject()
{
    QString defaultPath;
    if (project.get() != nullptr)
    {
        defaultPath = project->GetProjectDirectory() + project->GetProjectName();
    }
    else
    {
        defaultPath = QDir::currentPath();
    }

    QString projectPath = QFileDialog::getOpenFileName(mainWindow.get(), tr("Select a project file"),
                                                       defaultPath,
                                                       tr("Project (*.uieditor)"));

    if (projectPath.isEmpty())
    {
        return;
    }
    projectPath = QDir::toNativeSeparators(projectPath);

    OpenProject(projectPath);
}

void EditorCore::OnNewProject()
{
    ResultList resultList;
    QString newProjectPath;

    std::tie(newProjectPath, resultList) = CreateNewProject();

    if (!newProjectPath.isEmpty())
    {
        OpenProject(newProjectPath);
        return;
    }

    mainWindow->ShowResultList(tr("Error while creating project"), resultList);
}

void EditorCore::UnpackHelp()
{
    FilePath docsPath(EditorCoreDetails::DOCUMENTATION_DIRECTORY);
    FileSystem* fs = FileSystem::Instance();
    if (!fs->Exists(docsPath))
    {
        try
        {
            ResourceArchive helpRA("~res:/Help.docs");

            fs->DeleteDirectory(docsPath);
            fs->CreateDirectory(docsPath, true);

            helpRA.UnpackToFolder(docsPath);
        }
        catch (std::exception& ex)
        {
            Logger::Error("%s", ex.what());
            DVASSERT(false && "can't unpack help docs to documents dir");
        }
    }
}

QString EditorCore::ReadEditorTitle() const
{
    using namespace EditorCoreDetails;
    return QString::fromStdString(DAVA::Format(EDITOR_TITLE, DAVAENGINE_VERSION, APPLICATION_BUILD_VERSION, static_cast<DAVA::uint32>(sizeof(DAVA::pointer_size) * 8)));
}
