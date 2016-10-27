#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Base/Introspection.h"
#include "Logger/Logger.h"
#include "Render/RenderBase.h"
#include "ui_mainwindow.h"

#include "Preferences/PreferencesRegistrator.h"

#if defined(__DAVAENGINE_MACOS__)
#include "QtTools/Utils/ShortcutChecker.h"
#endif //__DAVAENGINE_MACOS__

#include <QtGui>
#include <QtWidgets>

namespace DAVA
{
class RenderWidget;
}

class PackageWidget;
class PropertiesWidget;
class LibraryWidget;

class LocalizationEditorDialog;
class Document;
class DocumentGroup;
class SpritesPacker;
class LoggerOutputObject;
class Project;

class MainWindow : public QMainWindow, public Ui::MainWindow, public DAVA::InspBase, public DAVA::TrackedObject
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    void AttachDocumentGroup(DocumentGroup* documentGroup, DAVA::RenderWidget* renderWidget);

    void OnProjectOpened(const DAVA::ResultList& resultList, const Project* project);
    void ExecDialogReloadSprites(SpritesPacker* packer);
    bool IsInEmulationMode() const;
    QComboBox* GetComboBoxLanguage();
    void RebuildRecentMenu(const QStringList& lastProjectsPathes);

signals:
    void CloseProject();
    void ActionExitTriggered();
    void RecentMenuTriggered(QAction*);
    void ActionOpenProjectTriggered(QString projectPath);
    void OpenPackageFile(QString path);
    void RtlChanged(bool isRtl);
    void BiDiSupportChanged(bool support);
    void GlobalStyleClassesChanged(const QString& classesStr);
    void ReloadSprites(DAVA::eGPUFamily gpu);
    void EmulationModeChanged(bool emulationMode);
    bool CanClose();

public slots:
    void OnDocumentChanged(Document* document);

private slots:
    void OnShowHelp();
    void OnOpenProjectAction();
    void OnPixelizationStateChanged(bool isPixelized);

    void OnRtlChanged(int arg);
    void OnBiDiSupportChanged(int arg);
    void OnGlobalClassesChanged(const QString& str);
    void OnLogOutput(DAVA::Logger::eLogLevel ll, const QByteArray& output);
    void OnEditorPreferencesTriggered();

private:
    void InitLanguageBox();
    void FillComboboxLanguages(const Project* core);
    void InitRtlBox();
    void InitBiDiSupportBox();
    void InitGlobalClasses();
    void InitEmulationMode();
    void InitMenu();
    void SetupViewMenu();
    void SetupBackgroundMenu();
    void UpdateProjectSettings();
    void OnPreferencesPropertyChanged(const DAVA::InspMember* member, const DAVA::VariantType& value);

    bool IsPixelized() const;
    void SetPixelized(bool pixelized);
    void closeEvent(QCloseEvent* event) override;

    bool eventFilter(QObject* object, QEvent* event) override;

    DAVA::String GetState() const;
    void SetState(const DAVA::String& array);

    DAVA::String GetGeometry() const;
    void SetGeometry(const DAVA::String& array);

    DAVA::String GetConsoleState() const;
    void SetConsoleState(const DAVA::String& array);

    QCheckBox* emulationBox = nullptr;
    LoggerOutputObject* loggerOutput = nullptr; //will be deleted by logger. Isn't it fun?
    qint64 acceptableLoggerFlags = ~0; //all flags accepted

    QComboBox* comboboxLanguage = nullptr;
    QString currentProjectPath;

    const DAVA::InspMember* backgroundIndexMember = nullptr;
    DAVA::Set<const DAVA::InspMember*> backgroundColorMembers;
    QActionGroup* backgroundActions = nullptr;
#if defined(__DAVAENGINE_MACOS__)
    ShortcutChecker shortcutChecker;
#endif //__DAVAENGINE_MACOS__

public:
    INTROSPECTION(MainWindow,
                  PROPERTY("isPixelized", "MainWindowInternal/IsPixelized", IsPixelized, SetPixelized, DAVA::I_PREFERENCE)
                  PROPERTY("state", "MainWindowInternal/State", GetState, SetState, DAVA::I_PREFERENCE)
                  PROPERTY("geometry", "MainWindowInternal/Geometry", GetGeometry, SetGeometry, DAVA::I_PREFERENCE)
                  PROPERTY("consoleState", "MainWindowInternal/ConsoleState", GetConsoleState, SetConsoleState, DAVA::I_PREFERENCE)
                  )
};

#endif // MAINWINDOW_H
