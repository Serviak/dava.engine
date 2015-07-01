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


#include "Platform/Qt5/QtLayer.h"
#include "Utils/PointerSerializer.h"
#include "UI/mainwindow.h"
#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "DocumentGroup.h"
#include "Document.h"
#include "EditorCore.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "SharedData.h"
#include "QtTools/ConsoleWidget/LogWidget.h"
#include <QSettings>
#include <QVariant>
#include <QByteArray>


using namespace DAVA;
namespace
{
    const QString CONSOLE_STATE = "console state";
}

EditorCore::EditorCore(QObject *parent)
    : QObject(parent)
    , Singleton<EditorCore>()
    , project(new Project(this))
    , documentGroup(new DocumentGroup(this))
    , mainWindow(new MainWindow())
    , dialogReloadSprites(new DialogReloadSprites(mainWindow))
    , logWidget(new LogWidget(mainWindow))
{
    mainWindow->setWindowIcon(QIcon(":/icon.ico"));
    mainWindow->CreateUndoRedoActions(documentGroup->GetUndoGroup());
    
    // Relod Sprites
    {
        QAction* actionReloadSprites = dialogReloadSprites->GetActionReloadSprites();
        mainWindow->menuTools->addAction(actionReloadSprites);
        mainWindow->toolBarPlugins->addAction(actionReloadSprites);
        connect(dialogReloadSprites, &DialogReloadSprites::StarPackProcess, this, &EditorCore::CloseAllDocuments);
    }
    // Console dock
    {
        mainWindow->consoleDockWidget->setWidget(logWidget);
        connect(logWidget, &LogWidget::ItemClicked, this, &EditorCore::OnNewItemSelected);
        QSettings settings(QApplication::organizationName(), QApplication::applicationName());
        const auto var = settings.value(CONSOLE_STATE);
        if (var.canConvert<QByteArray>())
        {
            logWidget->Deserialize(var.toByteArray());
        }
        connect(mainWindow->packageWidget, &PackageWidget::GotResult, logWidget, &LogWidget::AddResultList);
    }
    
    connect(project, &Project::ProjectPathChanged, this, &EditorCore::OnProjectPathChanged);
    connect(mainWindow, &MainWindow::TabClosed, this, &EditorCore::CloseOneDocument);
    connect(mainWindow, &MainWindow::CurrentTabChanged, this, &EditorCore::OnCurrentTabChanged);
    connect(mainWindow, &MainWindow::CloseProject, this, &EditorCore::CloseProject);
    connect(mainWindow, &MainWindow::ActionExitTriggered, this, &EditorCore::Exit);
    connect(mainWindow, &MainWindow::CloseRequested, this, &EditorCore::Exit);
    connect(mainWindow, &MainWindow::RecentMenuTriggered, this, &EditorCore::RecentMenu);
    connect(mainWindow, &MainWindow::ActionOpenProjectTriggered, this, &EditorCore::OpenProject);
    connect(mainWindow, &MainWindow::OpenPackageFile, this, &EditorCore::OnOpenPackageFile);
    connect(mainWindow, &MainWindow::SaveAllDocuments, this, &EditorCore::SaveAllDocuments);
    connect(mainWindow, &MainWindow::SaveDocument, this, static_cast<void(EditorCore::*)(int)>(&EditorCore::SaveDocument));
    connect(documentGroup, &DocumentGroup::DocumentChanged, mainWindow->libraryWidget, &LibraryWidget::OnDocumentChanged);

    connect(documentGroup, &DocumentGroup::DocumentChanged, mainWindow->propertiesWidget, &PropertiesWidget::OnDocumentChanged);
    connect(documentGroup, &DocumentGroup::SharedDataChanged, mainWindow->propertiesWidget, &PropertiesWidget::OnDataChanged);

    connect(documentGroup, &DocumentGroup::DocumentChanged, mainWindow->packageWidget, &PackageWidget::OnDocumentChanged);
    connect(documentGroup, &DocumentGroup::SharedDataChanged, mainWindow->packageWidget, &PackageWidget::OnDataChanged);

    connect(documentGroup, &DocumentGroup::DocumentChanged, mainWindow->previewWidget, &PreviewWidget::OnDocumentChanged);
    connect(documentGroup, &DocumentGroup::SharedDataChanged, mainWindow->previewWidget, &PreviewWidget::OnDataChanged);
    
    connect(project->GetEditorLocalizationSystem(), &EditorLocalizationSystem::LocaleChanged, this, &EditorCore::UpdateLanguage);

    qApp->installEventFilter(this);
}
    
EditorCore::~EditorCore()
{
    QSettings settings(QApplication::organizationName(), QApplication::applicationName());
    settings.setValue(CONSOLE_STATE, logWidget->Serialize());
    delete mainWindow;
}

void EditorCore::Start()
{
    mainWindow->show();
}

void EditorCore::OnCleanChanged(bool clean)
{
    QUndoStack *undoStack = qobject_cast<QUndoStack*>(sender());
    for (int i = 0; i < documents.size(); ++i)
    {
        if (undoStack == documents.at(i)->GetUndoStack())
        {
            mainWindow->OnCleanChanged(i, clean);
        }
    }
}

void EditorCore::OnOpenPackageFile(const QString &path)
{
    if (!path.isEmpty())
    {
        int index = GetIndexByPackagePath(path);
        if (index == -1)
        {
            DAVA::RefPtr<PackageNode> package = project->OpenPackage(path);
            if (nullptr != package)
            {
                index = CreateDocument(package.Get());
            }
        }
        mainWindow->SetCurrentTab(index);
    }
}

void EditorCore::OnProjectPathChanged(const QString &projectPath)
{
    QRegularExpression searchOption("gfx\\d*$", QRegularExpression::CaseInsensitiveOption);
    dialogReloadSprites->GetSpritesPacker()->ClearTasks();
    QDirIterator it(projectPath + "/DataSource");
    while (it.hasNext())
    {
        const QFileInfo &fileInfo = it.fileInfo();
        it.next();
        if (fileInfo.isDir())
        {
            QString outputPath = fileInfo.absoluteFilePath();
            if (!outputPath.contains(searchOption))
            {
                continue;
            }
            outputPath.replace(outputPath.lastIndexOf("DataSource"), QString("DataSource").size(), "Data");
            QDir outputDir(outputPath);
            dialogReloadSprites->GetSpritesPacker()->AddTask(fileInfo.absoluteFilePath(), outputDir);
        }
    }
}

bool EditorCore::CloseAllDocuments()
{
    for (auto index = documents.size() - 1; index >= 0; --index)
    {
        if (!CloseOneDocument(index))
            return false;
    }
    return true;
}

bool EditorCore::CloseOneDocument(int index)
{
    DVASSERT(index >= 0);
    DVASSERT(index < documents.size());
    const Document *document = documents.at(index);
    QUndoStack *undoStack = document->GetUndoStack();
    if (!undoStack->isClean())
    {
        QMessageBox::StandardButton ret = QMessageBox::question(
            qApp->activeWindow(),
            tr("Save changes"),
            tr("The file %1 has been modified.\n"
               "Do you want to save your changes?").arg(document->GetPackageFilePath().GetBasename().c_str()),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save
            );
        if (ret == QMessageBox::Save)
        {
            SaveDocument(index);
        }
        else if (ret == QMessageBox::Cancel)
        {
            return false;
        }
    }
    CloseDocument(index);
    return true;
}

void EditorCore::SaveDocument(int index)
{
    DVASSERT(index >= 0);
    DVASSERT(index < documents.size());
    SaveDocument(documents[index]);
}

void EditorCore::SaveAllDocuments()
{
    for (auto &document : documents)
    {
        DVVERIFY(project->SavePackage(document->GetPackage()));
        document->GetUndoStack()->setClean();
    }
}

void EditorCore::Exit()
{
    if (CloseProject())
    {
        QCoreApplication::exit();
    }
}

void EditorCore::RecentMenu(QAction *recentProjectAction)
{
    QString projectPath = recentProjectAction->data().toString();

    if (projectPath.isEmpty())
    {
        return;
    }
    OpenProject(projectPath);
}

void EditorCore::OnCurrentTabChanged(int index)
{
    documentGroup->SetActiveDocument(index == -1 ? nullptr : documents.at(index));
}

void EditorCore::UpdateLanguage()
{
    project->GetEditorFontSystem()->RegisterCurrentLocaleFonts();
    for(auto &document : documents)
    {
        document->RefreshAllControlProperties();
    }
}

void EditorCore::OnNewItemSelected(const DAVA::PointerSerializer &arg)
{
    documentGroup->GetActiveDocument()->SetSelectedItem(arg);
}

void EditorCore::OpenProject(const QString &path)
{
    if (!CloseProject())
    {
        return;
    }

    if (!project->CheckAndUnlockProject(path))
    {
        return;
    }
    ResultList resultList;
    if (!project->Open(path))
    {
        resultList.AddResult(Result::RESULT_ERROR, "Error while loading project");
    }
    mainWindow->OnProjectOpened(resultList, path);
}

bool EditorCore::CloseProject()
{
    bool hasUnsaved = false;
    for (auto &doc : documents)
    {
        if (!doc->GetUndoStack()->isClean())
        {
            hasUnsaved = true;
            break;
        }
    }
    if (hasUnsaved)
    {
        int ret = QMessageBox::question(
            qApp->activeWindow(),
            tr("Save changes"),
            tr("Some files has been modified.\n"
                "Do you want to save your changes?"),
            QMessageBox::SaveAll | QMessageBox::NoToAll | QMessageBox::Cancel
            );
        if (ret == QMessageBox::Cancel)
        {
            return false;
        }
        else if (ret == QMessageBox::SaveAll)
        {
            SaveAllDocuments();
        }
    }

    while(!documents.isEmpty())
    {
        CloseDocument(0);
    }
    return true;
}

void EditorCore::CloseDocument(int index)
{
    DVASSERT(index >= 0);
    DVASSERT(index < documents.size());
    int newIndex = mainWindow->CloseTab(index);

    //sync document list with tab list
    Document *detached = documents.takeAt(index);
    documentGroup->SetActiveDocument(newIndex == -1 ? nullptr : documents.at(newIndex));
    documentGroup->RemoveDocument(detached);
    delete detached; //some widgets hold this document inside :(
}

int EditorCore::CreateDocument(PackageNode *package)
{
    Document *document = new Document(package, this);
    connect(document->GetUndoStack(), &QUndoStack::cleanChanged, this, &EditorCore::OnCleanChanged);
    documents.push_back(document);
    documentGroup->AddDocument(document);
    int index = mainWindow->AddTab(document->GetPackageFilePath());
    OnCurrentTabChanged(index);
    return index;
}

void EditorCore::SaveDocument(Document *document)
{
    DVASSERT(document);
    if (document->GetUndoStack()->isClean())
    {
        return;
    }
    DVVERIFY(project->SavePackage(document->GetPackage())); //TODO:log here
    document->GetUndoStack()->setClean();
}

int EditorCore::GetIndexByPackagePath(const QString &fileName) const
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    DAVA::FilePath davaPath(canonicalFilePath.toStdString());

    for (int index = 0; index < documents.size(); ++index)
    {
        if (documents.at(index)->GetPackageFilePath() == davaPath)
        {
            return index;
        }
    }
    return -1;
}

bool EditorCore::eventFilter( QObject *obj, QEvent *event )
{
    QEvent::Type eventType = event->type();

    if ( qApp == obj )
    {
        if ( QEvent::ApplicationStateChange == eventType )
        {
            QApplicationStateChangeEvent* stateChangeEvent = static_cast<QApplicationStateChangeEvent*>( event );
            Qt::ApplicationState state = stateChangeEvent->applicationState();
            switch ( state )
            {
            case Qt::ApplicationInactive:
            {
                if ( DAVA::QtLayer::Instance() )
                {
                    DAVA::QtLayer::Instance()->OnSuspend();
                }
                break;
            }
            case Qt::ApplicationActive:
            {
                if ( DAVA::QtLayer::Instance() )
                {
                    DAVA::QtLayer::Instance()->OnResume();
                }
                break;
            }
            default:
                break;
            }
        }
    }

    return QObject::eventFilter( obj, event );
}