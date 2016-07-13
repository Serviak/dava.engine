#pragma once

#include "ui_mainwindow.h"

#include "Classes/Qt/Main/ModificationWidget.h"
#include "Classes/Qt/Tools/QtWaitDialog/QtWaitDialog.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/Main/RecentMenuItems.h"
#include "Classes/Beast/BeastProxy.h"

#include "DAVAEngine.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QPointer>

namespace wgt
{
class IComponentContext;
}

class AddSwitchEntityDialog;
class Request;
class QtLabelWithActions;
class HangingObjectsHeight;
class DeveloperTools;
class VersionInfoWidget;
#if defined(NEW_PROPERTY_PANEL)
class PropertyPanel;
#endif
class DeviceListController;
class SpritesPackerModule;
class QtMainWindow : public QMainWindow, public DAVA::Singleton<QtMainWindow>
{
    Q_OBJECT

    static const int GLOBAL_INVALIDATE_TIMER_DELTA = 1000;

signals:
    void GlobalInvalidateTimeout();

    void TexturesReloaded();

public:
    explicit QtMainWindow(wgt::IComponentContext& ngtContext, QWidget* parent = 0);
    ~QtMainWindow();

    Ui::MainWindow* GetUI();
    SceneTabWidget* GetSceneWidget();
    SceneEditor2* GetCurrentScene();

    bool OpenScene(const QString& path);
    bool SaveScene(SceneEditor2* scene);
    bool SaveSceneAs(SceneEditor2* scene);

    void SetGPUFormat(DAVA::eGPUFamily gpu);
    DAVA::eGPUFamily GetGPUFormat();

    void WaitStart(const QString& title, const QString& message, int min = 0, int max = 100);
    void WaitSetMessage(const QString& messsage);
    void WaitSetValue(int value);
    void WaitStop();

    void BeastWaitSetMessage(const QString& messsage);
    bool BeastWaitCanceled();

    void EnableGlobalTimeout(bool enable);

    bool CanBeClosed();

    // qt actions slots
public slots:
    void OnProjectOpen();
    void OnProjectClose();
    void OnSceneNew();
    void OnSceneOpen();
    void OnSceneSave();
    void OnSceneSaveAs();
    void OnSceneSaveToFolder();
    void OnSceneSaveToFolderCompressed();
    void OnRecentFilesTriggered(QAction* recentAction);
    void OnRecentProjectsTriggered(QAction* recentAction);
    void ExportTriggered();
    void OnImportSpeedTreeXML();
    void RemoveSelection();

    void OnUndo();
    void OnRedo();

    void OnEditorGizmoToggle(bool show);
    void OnViewLightmapCanvas(bool show);
    void OnAllowOnSceneSelectionToggle(bool allow);
    void OnShowStaticOcclusionToggle(bool show);
    void OnEnableVisibilitySystemToggle(bool enabled);
    void OnRefreshVisibilitySystem();
    void OnFixVisibilityFrame();
    void OnReleaseVisibilityFrame();

    void OnEnableDisableShadows(bool enable);

    void OnReloadTextures();
    void OnReloadTexturesTriggered(QAction* reloadAction);

    void OnSelectMode();
    void OnMoveMode();
    void OnRotateMode();
    void OnScaleMode();
    void OnPivotCenterMode();
    void OnPivotCommonMode();
    void OnManualModifMode();
    void OnPlaceOnLandscape();
    void OnSnapToLandscape();
    void OnResetTransform();
    void OnLockTransform();
    void OnUnlockTransform();

    void OnCenterPivotPoint();
    void OnZeroPivotPoint();

    void OnMaterialEditor();
    void OnTextureBrowser();
    void OnSceneLightMode();

    void OnCubemapEditor();
    void OnImageSplitter();

    void OnAddLandscape();
    void OnAddVegetation();
    void OnLightDialog();
    void OnCameraDialog();
    void OnEmptyEntity();
    void OnAddWindEntity();
    void OnAddPathEntity();

    void OnUserNodeDialog();
    void OnSwitchEntityDialog();
    void OnParticleEffectDialog();
    void On2DCameraDialog();
    void On2DSpriteDialog();
    void OnAddEntityFromSceneTree();

    void OnShowSettings();
    void OnOpenHelp();

    void OnSaveHeightmapToImage();
    void OnSaveTiledTexture();
    void OnTiledTextureRetreived(DAVA::Landscape* landscape, DAVA::Texture* landscapeTexture);

    void OnConvertModifiedTextures();

    void OnCloseTabRequest(int tabIndex, Request* closeRequest);

    void OnBeastAndSave();

    void OnBuildStaticOcclusion();
    void OnInavalidateStaticOcclusion();

    void OnLandscapeEditorToggled(SceneEditor2* scene);
    void OnForceFirstLod(bool);
    void OnCustomColorsEditor();
    void OnHeightmapEditor();
    void OnRulerTool();
    void OnTilemaskEditor();
    void OnNotPassableTerrain();
    void OnWayEditor();

    void OnObjectsTypeChanged(QAction* action);
    void OnObjectsTypeChanged(int type);

    void OnHangingObjects();
    void OnHangingObjectsHeight(double value);

    void OnMaterialLightViewChanged(bool);
    void OnCustomQuality();

    void OnReloadShaders();

    void OnSwitchWithDifferentLODs(bool checked);

    void OnGenerateHeightDelta();

    void OnBatchProcessScene();

    void OnSnapCameraToLandscape(bool);

    void SetupTitle();

    void RestartParticleEffects();
    bool SetVisibilityToolEnabledIfPossible(bool);
    void SetLandscapeInstancingEnabled(bool);

protected:
    bool eventFilter(QObject* object, QEvent* event) override;

    void SetupMainMenu();
    void SetupThemeActions();
    void SetupToolBars();
    void SetupStatusBar();
    void SetupDocks();
    void SetupActions();
    void SetupShortCuts();

    void StartGlobalInvalidateTimer();

    void RunBeast(const QString& outputPath, BeastProxy::eBeastMode mode);

    bool IsAnySceneChanged();

    void DiableUIForFutureUsing();
    void SynchronizeStateWithUI();

    bool SelectCustomColorsTexturePath();

    static void SetActionCheckedSilently(QAction* action, bool checked);

    void OpenProject(const DAVA::FilePath& projectPath);

    void OnSceneSaveAsInternal(bool saveWithCompressed);

    void SaveAllSceneEmitters(SceneEditor2* scene) const;

private slots:
    void ProjectOpened(const QString& path);
    void ProjectClosed();

    void SceneUndoRedoStateChanged(SceneEditor2* scene);
    void SceneCommandExecuted(SceneEditor2* scene, const DAVA::Command* command, bool redo);
    void SceneActivated(SceneEditor2* scene);
    void SceneDeactivated(SceneEditor2* scene);
    void SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected);

    void OnGlobalInvalidateTimeout();
    void EditorLightEnabled(bool enabled);
    void OnSnapToLandscapeChanged(SceneEditor2* scene, bool isSpanToLandscape);
    void UnmodalDialogFinished(int);

    void DebugVersionInfo();
    void DebugColorPicker();
    void DebugDeviceList();
    void OnConsoleItemClicked(const QString& data);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QtWaitDialog* waitDialog;
    QtWaitDialog* beastWaitDialog;
    QPointer<QDockWidget> dockActionEvent;
    QPointer<QDockWidget> dockConsole;

    bool globalInvalidate;

    ModificationWidget* modificationWidget;

    QComboBox* objectTypesWidget;

    AddSwitchEntityDialog* addSwitchEntityDialog;
    HangingObjectsHeight* hangingObjectsWidget;

    void EnableSceneActions(bool enable);
    void EnableProjectActions(bool enable);
    void UpdateConflictingActionsState(bool enable);
    void UpdateModificationActionsState();
    void UpdateWayEditor(const DAVA::Command* command, bool redo);

    void LoadViewState(SceneEditor2* scene);
    void LoadModificationState(SceneEditor2* scene);
    void LoadEditorLightState(SceneEditor2* scene);
    void LoadGPUFormat();
    void LoadLandscapeEditorState(SceneEditor2* scene);
    void LoadObjectTypes(SceneEditor2* scene);
    void LoadHangingObjects(SceneEditor2* scene);
    void LoadMaterialLightViewMode();

    bool SaveTilemask(bool forAllTabs = true);

    // Landscape editor specific
    // TODO: remove later -->
    bool LoadAppropriateTextureFormat();
    bool IsSavingAllowed();
    // <--

    //Need for any debug functionality
    QPointer<DeveloperTools> developerTools;
    QPointer<VersionInfoWidget> versionInfoWidget;

    QPointer<DeviceListController> deviceListController;

    RecentMenuItems recentFiles;
    RecentMenuItems recentProjects;

    wgt::IComponentContext& ngtContext;
#if defined(NEW_PROPERTY_PANEL)
    std::unique_ptr<PropertyPanel> propertyPanel;
#endif
    std::unique_ptr<SpritesPackerModule> spritesPacker;

private:
    struct EmitterDescriptor
    {
        EmitterDescriptor(DAVA::ParticleEmitter* _emitter, DAVA::ParticleLayer* layer, DAVA::FilePath path, DAVA::String name)
            : emitter(_emitter)
            , ownerLayer(layer)
            , yamlPath(path)
            , entityName(name)
        {
        }

        DAVA::ParticleEmitter* emitter = nullptr;
        DAVA::ParticleLayer* ownerLayer = nullptr;
        DAVA::FilePath yamlPath;
        DAVA::String entityName;
    };

    void CollectEmittersForSave(DAVA::ParticleEmitter* topLevelEmitter, DAVA::List<EmitterDescriptor>& emitters, const DAVA::String& entityName) const;
};
