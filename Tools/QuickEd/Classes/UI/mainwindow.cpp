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


#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QColorDialog>
#include <QPushButton>

//////////////////////////////////////////////////////////////////////////
#include "fontmanagerdialog.h"
#include "FileSystem/FileSystem.h"
#include "Helpers/ResourcesManageHelper.h"
#include "Dialogs/localizationeditordialog.h"
#include "Grid/GridVisualizer.h"
#include "EditorFontManager.h"
//////////////////////////////////////////////////////////////////////////

#include "UI/FileSystemView/FileSystemDockWidget.h"
#include "UI/UIPackageLoader.h"
#include "Utils/QtDavaConvertion.h"
#include "Model/PackageHierarchy/PackageNode.h"

namespace {
    const QString APP_NAME = "QuickEd";
    const QString APP_COMPANY = "DAVA";
    const QString APP_GEOMETRY = "geometry";
    const QString APP_STATE = "windowstate";

    const char* COLOR_PROPERTY_ID = "color";
}
using namespace DAVA;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , backgroundFrameUseCustomColorAction(nullptr)
    , backgroundFrameSelectCustomColorAction(nullptr)
{
    ui->setupUi(this);
    ui->tabBar->setElideMode(Qt::ElideNone);
    setWindowTitle(ResourcesManageHelper::GetProjectTitle());

    ui->tabBar->setTabsClosable(true);
    ui->tabBar->setUsesScrollButtons(true);
    connect(ui->tabBar, &QTabBar::tabCloseRequested, this, &MainWindow::TabClosed);
    connect(ui->tabBar, &QTabBar::currentChanged, this, &MainWindow::CurrentTabChanged);

    setUnifiedTitleAndToolBarOnMac(true);

    connect(ui->actionFontManager, &QAction::triggered, this, &MainWindow::OnOpenFontManager);
    connect(ui->actionLocalizationManager, &QAction::triggered, this, &MainWindow::OnOpenLocalizationManager);

    connect(ui->fileSystemDockWidget, &FileSystemDockWidget::OpenPackageFile, this, &MainWindow::OpenPackageFile);

    InitMenu();
    RestoreMainWindowState();

    ui->fileSystemDockWidget->setEnabled(false);

    RebuildRecentMenu();

}

MainWindow::~MainWindow()
{
    SaveMainWindowState();
    delete ui;
}

void MainWindow::CreateUndoRedoActions(const QUndoGroup &undoGroup)
{
    QAction *undoAction = undoGroup.createUndoAction(this);
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setIcon(QIcon(":/Icons/edit_undo.png"));

    QAction *redoAction = undoGroup.createRedoAction(this);
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setIcon(QIcon(":/Icons/edit_redo.png"));

    ui->mainToolbar->addAction(undoAction);
    ui->mainToolbar->addAction(redoAction);
}

PackageWidget *MainWindow::GetPackageWidget() const
{
    return ui->packageWidget;
}

void MainWindow::OnProjectIsOpenChanged(bool arg)
{
    ui->fileSystemDockWidget->setEnabled(arg);
    this->setWindowTitle(ResourcesManageHelper::GetProjectTitle());
}

void MainWindow::OnCountChanged(int count)
{
    ui->actionSaveAllDocuments->setEnabled(count > 0);
    OnCurrentIndexChanged(ui->tabBar->currentIndex());
}

int MainWindow::CloseTab(int index)
{
    delete ui->tabBar->tabData(index).value<TabState*>();
    ui->tabBar->removeTab(index);
    return ui->tabBar->currentIndex();
}

void MainWindow::SetCurrentTab(int index)
{
    ui->tabBar->setCurrentIndex(index);
}

void MainWindow::SaveMainWindowState()
{
    QSettings settings(APP_COMPANY, APP_NAME);
    settings.setValue(APP_GEOMETRY, saveGeometry());
    settings.setValue(APP_STATE, saveState());
}

void MainWindow::RestoreMainWindowState()
{
    QSettings settings(APP_COMPANY, APP_NAME);
    // Check settings befor applying it
    if (!settings.value(APP_GEOMETRY).isNull() && settings.value(APP_GEOMETRY).isValid())
    {
        restoreGeometry(settings.value(APP_GEOMETRY).toByteArray());
    }
    if (!settings.value(APP_STATE).isNull() && settings.value(APP_STATE).isValid())
    {
        restoreState(settings.value(APP_STATE).toByteArray());
    }
}

void MainWindow::OnCurrentIndexChanged(int arg)
{
    bool enabled = arg >= 0;
    ui->packageWidget->setEnabled(enabled);
    ui->propertiesWidget->setEnabled(enabled);
    ui->previewWidget->setEnabled(enabled);
    ui->libraryWidget->setEnabled(enabled);
    TabState *tabState = ui->tabBar->tabData(arg).value<TabState*>();
    ui->actionSaveDocument->setEnabled(nullptr != tabState && tabState->isModified); //set action enabled if new documend still modified
}

void MainWindow::OnCleanChanged(int index, bool val)
{
    DVASSERT(index >= 0);
    TabState *tabState = ui->tabBar->tabData(index).value<TabState*>();
    tabState->isModified = !val;

    QString tabText = tabState->tabText;
    if (!val)
    {
        tabText.append('*');
    }
    ui->tabBar->setTabText(index, tabText);

    if (index == ui->tabBar->currentIndex())
    {
        ui->actionSaveDocument->setEnabled(tabState->isModified);
    }
}

void MainWindow::OnOpenFontManager()
{
    FontManagerDialog fontManagerDialog(false, QString(), this);
    fontManagerDialog.exec();
}

void MainWindow::OnOpenLocalizationManager()
{
    LocalizationEditorDialog localizationManagerDialog(this);
    localizationManagerDialog.exec();
}

void MainWindow::OnShowHelp()
{
    FilePath docsPath = ResourcesManageHelper::GetDocumentationPath().toStdString() + "index.html";
    QString docsFile = QString::fromStdString("file:///" + docsPath.GetAbsolutePathname());
    QDesktopServices::openUrl(QUrl(docsFile));
}

void MainWindow::InitMenu()
{
    SetupViewMenu();

    connect(ui->actionSaveDocument, &QAction::triggered, this, &MainWindow::OnSaveDocument);
    connect(ui->actionSaveAllDocuments, &QAction::triggered, this, &MainWindow::SaveAllDocuments);
    connect(ui->actionOpen_project, &QAction::triggered, this, &MainWindow::OnOpenProject);
    connect(ui->actionClose_project, &QAction::triggered, this, &MainWindow::CloseProject);

    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::ActionExitTriggered);
    connect(ui->menuRecent, &QMenu::triggered, this, &MainWindow::RecentMenuTriggered);

    // Remap zoom in/out shorcuts for windows platform
#if defined(__DAVAENGINE_WIN32__)
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_Equal));
    shortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_Plus));
    ui->actionZoomIn->setShortcuts(shortcuts);
#endif

    //Help contents dialog
    connect(ui->actionHelp, &QAction::triggered, this, &MainWindow::OnShowHelp);

    // Pixelization.
    ui->actionPixelized->setChecked(EditorSettings::Instance()->IsPixelized());
    connect(ui->actionPixelized, &QAction::triggered, this, &MainWindow::OnPixelizationStateChanged);
    DisableActions();
}

void MainWindow::OnSaveDocument()
{
    int index = ui->tabBar->currentIndex();
    DVASSERT(index >= 0);
    emit SaveDocument(index);
}

void MainWindow::SetupViewMenu()
{
    // Setup the common menu actions.
    ui->menuView->addAction(ui->propertiesWidget->toggleViewAction());
    ui->menuView->addAction(ui->fileSystemDockWidget->toggleViewAction());
    ui->menuView->addAction(ui->packageWidget->toggleViewAction());
    ui->menuView->addAction(ui->libraryWidget->toggleViewAction());
    ui->menuView->addAction(ui->consoleDockWidget->toggleViewAction());

    ui->menuView->addSeparator();
    ui->menuView->addAction(ui->mainToolbar->toggleViewAction());
    
    // Setup the Background Color menu.
    QMenu* setBackgroundColorMenu = new QMenu("Background Color");
    ui->menuView->addSeparator();
    ui->menuView->addMenu(setBackgroundColorMenu);

    static const struct
    {
        QColor color;
        QString colorName;
    } colorsMap[] =
    {
        { Qt::black, "Black" },
        { QColor(0x33, 0x33, 0x33, 0xFF), "Default" },
        { QColor(0x53, 0x53, 0x53, 0xFF), "Dark Gray" },
        { QColor(0xB8, 0xB8, 0xB8, 0xFF), "Medium Gray" },
        { QColor(0xD6, 0xD6, 0xD6, 0xFF), "Light Gray" },
    };
    
    Color curBackgroundColor = EditorSettings::Instance()->GetCurrentBackgroundFrameColor();
    int32 itemsCount = COUNT_OF(colorsMap);
    
    bool isCustomColor = true;
    for (int32 i = 0; i < itemsCount; i ++)
    {
        QAction* colorAction = new QAction(colorsMap[i].colorName, setBackgroundColorMenu);
        colorAction->setProperty(COLOR_PROPERTY_ID, colorsMap[i].color);
        
        Color curColor = QColorToColor(colorsMap[i].color);
        if (curColor == curBackgroundColor)
        {
            isCustomColor = false;
        }

        colorAction->setCheckable(true);
        colorAction->setChecked(curColor == curBackgroundColor);
        
        backgroundFramePredefinedColorActions.append(colorAction);
        setBackgroundColorMenu->addAction(colorAction);
    }
    
    backgroundFrameUseCustomColorAction = new QAction("Custom", setBackgroundColorMenu);
    backgroundFrameUseCustomColorAction->setProperty(COLOR_PROPERTY_ID, ColorToQColor(curBackgroundColor));
    backgroundFrameUseCustomColorAction->setCheckable(true);
    backgroundFrameUseCustomColorAction->setChecked(isCustomColor);
    setBackgroundColorMenu->addAction(backgroundFrameUseCustomColorAction);
    
    setBackgroundColorMenu->addSeparator();
    
    backgroundFrameSelectCustomColorAction = new QAction("Select Custom Color...", setBackgroundColorMenu);
    setBackgroundColorMenu->addAction(backgroundFrameSelectCustomColorAction);
    
    connect(setBackgroundColorMenu, SIGNAL(triggered(QAction*)), this, SLOT(SetBackgroundColorMenuTriggered(QAction*)));

    // Another actions below the Set Background Color.
    ui->menuView->addAction(ui->actionZoomIn);
    ui->menuView->insertSeparator(ui->actionZoomIn);
    ui->menuView->addAction(ui->actionZoomOut);
}

void MainWindow::DisableActions()
{
    ui->actionSaveAllDocuments->setEnabled(false);
    ui->actionSaveDocument->setEnabled(false);

    ui->actionClose_project->setEnabled(false);
    ui->actionFontManager->setEnabled(false);
    ui->actionLocalizationManager->setEnabled(false);

    // Reload.
    ui->actionRepack_And_Reload->setEnabled(false);
}

void MainWindow::RebuildRecentMenu()
{
    ui->menuRecent->clear();
    // Get up to date count of recent project actions
    int32 projectCount = EditorSettings::Instance()->GetLastOpenedCount();
    QStringList projectList;

    for (int32 i = 0; i < projectCount; ++i)
    {
        projectList << QDir::toNativeSeparators(QString(EditorSettings::Instance()->GetLastOpenedFile(i).c_str()));
    }
    projectList.removeDuplicates();
    for (auto &projectPath : projectList)
    {
        QAction *recentProject = new QAction(projectPath, this);
        recentProject->setData(projectPath);
        ui->menuRecent->addAction(recentProject);
    }
    ui->menuRecent->setEnabled(projectCount > 0);
}

int MainWindow::AddTab(const QString &tabText)
{
    int index = ui->tabBar->addTab(tabText);
    TabState* tabState = new TabState(tabText);
    ui->tabBar->setTabData(index, QVariant::fromValue<TabState*>(tabState));
    return index;
}

void MainWindow::SetDocumentToWidgets(Document *document)
{
    ui->propertiesWidget->SetDocument(document);
    ui->packageWidget->SetDocument(document);
    ui->previewWidget->SetDocument(document);
    ui->libraryWidget->SetDocument(document);
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    emit CloseRequested();
    ev->ignore();
}

void MainWindow::OnProjectOpened(Result result, QString projectPath)
{
    if (result)
    {
        UpdateProjectSettings(projectPath);

        RebuildRecentMenu();
        ui->fileSystemDockWidget->SetProjectDir(projectPath);
        ui->fileSystemDockWidget->setEnabled(true);
    }
    else
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Error while loading project"), result.errors.join('\n'));
    }
}

void MainWindow::OnOpenProject()
{
    QString projectPath = QFileDialog::getOpenFileName(this, tr("Select a project file"),
                                                        ResourcesManageHelper::GetDefaultDirectory(),
                                                        tr( "Project (*.uieditor)"));
    if (projectPath.isEmpty())
    {
        return;
    }
    projectPath = QDir::toNativeSeparators(projectPath);
    
    emit ActionOpenProjectTriggered(projectPath);
}

void MainWindow::UpdateProjectSettings(const QString& projectPath)
{
    // Add file to recent project files list
    EditorSettings::Instance()->AddLastOpenedFile(projectPath.toStdString());
    
    // Save to settings default project directory
    QFileInfo fileInfo(projectPath);
    QString projectDir = fileInfo.absoluteDir().absolutePath();
    EditorSettings::Instance()->SetProjectPath(projectDir.toStdString());

    // Update window title
    this->setWindowTitle(ResourcesManageHelper::GetProjectTitle(projectPath));
    
    // Apply the pixelization value.
    Texture::SetPixelization(EditorSettings::Instance()->IsPixelized());
}

void MainWindow::OnPixelizationStateChanged()
{
    bool isPixelized = ui->actionPixelized->isChecked();
    EditorSettings::Instance()->SetPixelized(isPixelized);

    Texture::SetPixelization(isPixelized);
}

void MainWindow::SetBackgroundColorMenuTriggered(QAction* action)
{
    Color newColor;

    if (action == backgroundFrameSelectCustomColorAction)
    {
        // Need to select new Background Frame color.
        QColor curColor = ColorToQColor(EditorSettings::Instance()->GetCustomBackgroundFrameColor());
        QColor color = QColorDialog::getColor(curColor, this, "Select color", QColorDialog::ShowAlphaChannel);
        if (color.isValid() == false)
        {
            return;
        }

        newColor = QColorToColor(color);
        EditorSettings::Instance()->SetCustomBackgroundFrameColor(newColor);
    }
    else if (action == backgroundFrameUseCustomColorAction)
    {
        // Need to use custom Background Frame Color set up earlier.
        newColor = EditorSettings::Instance()->GetCustomBackgroundFrameColor();
    }
    else
    {
        // Need to use predefined Background Frame Color.
        newColor = QColorToColor(action->property(COLOR_PROPERTY_ID).value<QColor>());
    }

    EditorSettings::Instance()->SetCurrentBackgroundFrameColor(newColor);
    //ScreenWrapper::Instance()->SetBackgroundFrameColor(newColor);
    
    // Update the check marks.
    bool colorFound = false;
    foreach (QAction* colorAction, backgroundFramePredefinedColorActions)
    {
        Color color = QColorToColor(colorAction->property(COLOR_PROPERTY_ID).value<QColor>());
        if (color == newColor)
        {
            colorAction->setChecked(true);
            colorFound = true;
        }
        else
        {
            colorAction->setChecked(false);
        }
    }

    // In case we don't found current color in predefined ones - select "Custom" menu item.
    backgroundFrameUseCustomColorAction->setChecked(!colorFound);
}
