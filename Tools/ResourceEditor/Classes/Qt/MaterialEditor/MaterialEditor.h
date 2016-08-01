#ifndef __MATERIAL_EDITOR_H__
#define __MATERIAL_EDITOR_H__

#include <QDialog>
#include <QPointer>
#include <QStandardItemModel>

#include "DAVAEngine.h"

#include "MaterialTemplateModel.h"
#include "Scene/SceneSignals.h"
#include "Tools/QtPosSaver/QtPosSaver.h"
#include "DockProperties/PropertyEditorStateHelper.h"

namespace Ui
{
class MaterialEditor;
}

class QtPropertyDataInspDynamic;

class LazyUpdater;
class MaterialEditor : public QDialog, public DAVA::Singleton<MaterialEditor>
{
    Q_OBJECT

private:
    typedef QMap<int, bool> ExpandMap;

public:
    MaterialEditor(QWidget* parent = 0);
    ~MaterialEditor();

    void SelectMaterial(DAVA::NMaterial* material);
    void SelectEntities(DAVA::NMaterial* material);

public slots:
    void sceneActivated(SceneEditor2* scene);
    void sceneDeactivated(SceneEditor2* scene);
    void commandExecuted(SceneEditor2* scene, const Command2* command, bool redo);
    void materialSelected(const QItemSelection& selected, const QItemSelection& deselected);

    void OnQualityChanged();

protected slots:
    void OnTemplateChanged(int index);
    void OnTemplateButton();
    void OnPropertyEdited(const QModelIndex&);
    void OnAddRemoveButton();

    void OnMaterialAddGlobal(bool checked);
    void OnMaterialRemoveGlobal(bool checked);
    void OnMaterialSave(bool checked);
    void OnMaterialLoad(bool checked);
    void OnMaterialPropertyEditorContextMenuRequest(const QPoint& pos);

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

    void SetCurMaterial(const QList<DAVA::NMaterial*>& materials);

    void FillTemplates(const QList<DAVA::NMaterial*>& materials);
    void RefreshMaterialProperties();

private slots:
    void onFilterChanged();
    void onCurrentExpandModeChange(bool mode);
    void onContextMenuPrepare(QMenu* menu);
    void autoExpand();
    void removeInvalidTexture();

    /// Tabbar handlers
    void onTabNameChanged(int index);
    void onCreateConfig(int index);
    void onCurrentConfigChanged(int index);
    void onTabRemove(int index);
    void onTabContextMenuRequested(const QPoint& pos);

private:
    enum
    {
        CHECKED_NOTHING = 0x0,

        CHECKED_TEMPLATE = 0x1,
        CHECKED_GROUP = 0x2,
        CHECKED_PROPERTIES = 0x4,
        CHECKED_TEXTURES = 0x8,

        CHECKED_ALL = 0xff
    };

    QString GetTemplatePath(DAVA::int32 index) const;
    DAVA::uint32 ExecMaterialLoadingDialog(DAVA::uint32 initialState, const QString& inputFile);

    void initActions();
    void initTemplates();
    void setTemplatePlaceholder(const QString& text);

    void StoreMaterialToPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* preset,
                               DAVA::SerializationContext* context) const;
    void StoreMaterialTextures(DAVA::NMaterial* material, const DAVA::InspMember* materialMember,
                               DAVA::KeyedArchive* texturesArchive, DAVA::SerializationContext* context) const;
    void StoreMaterialFlags(DAVA::NMaterial* material, const DAVA::InspMember* materialMember,
                            DAVA::KeyedArchive* flagsArchive) const;
    void StoreMaterialProperties(DAVA::NMaterial* material, const DAVA::InspMember* materialMember,
                                 DAVA::KeyedArchive* propertiesArchive) const;

    void UpdateMaterialFromPresetWithOptions(DAVA::NMaterial* material, DAVA::KeyedArchive* preset,
                                             DAVA::SerializationContext* context, DAVA::uint32 options);
    void UpdateMaterialPropertiesFromPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* properitesArchive);
    void UpdateMaterialFlagsFromPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* flagsArchive);
    void UpdateMaterialTexturesFromPreset(DAVA::NMaterial* material, DAVA::KeyedArchive* texturesArchive,
                                          const DAVA::FilePath& scenePath);

    QtPropertyData* AddSection(const DAVA::FastName& sectionName);

    void AddMaterialFlagIfNeed(DAVA::NMaterial* material, const DAVA::FastName& flagName);
    bool HasMaterialProperty(DAVA::NMaterial* material, const DAVA::FastName& paramName);

    void UpdateTabs();

private:
    class PropertiesBuilder;

    Ui::MaterialEditor* ui = nullptr;
    SceneEditor2* activeScene = nullptr;

    QtPosSaver posSaver;
    QList<DAVA::NMaterial*> curMaterials;
    QPointer<MaterialTemplateModel> templatesFilterModel;

    ExpandMap expandMap;
    PropertyEditorStateHelper* treeStateHelper = nullptr;

    DAVA::FilePath lastSavePath;
    DAVA::uint32 lastCheckState = 0;

    LazyUpdater* materialPropertiesUpdater;
    class ConfigNameValidator;
    ConfigNameValidator* validator;
};

#endif
