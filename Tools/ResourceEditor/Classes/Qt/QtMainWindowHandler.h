#ifndef __QT_MAIN_WINDOW_HANDLER_H__
#define __QT_MAIN_WINDOW_HANDLER_H__

#include <QObject>
#include <QPoint>

#include "DAVAEngine.h"
#include "../Constants.h"
#include "Classes/SceneEditor/EditorSettings.h"

class Command;
class QMenu;
class QAction;
class QTreeView;
class QtMainWindowHandler: public QObject, public DAVA::Singleton<QtMainWindowHandler>
{
    Q_OBJECT
    
public:
    QtMainWindowHandler(QObject *parent = 0);
    virtual ~QtMainWindowHandler();

    void RegisterNodeActions(DAVA::int32 count, ...);
    void RegisterViewportActions(DAVA::int32 count, ...);
    void RegisterDockActions(DAVA::int32 count, ...);
    
    void SetResentMenu(QMenu *menu);
    void SetResentAncorAction(QAction *ancorAction);

    //MENU FILE
    void MenuFileWillShow();

	void SetDefaultFocusWidget(QWidget *widget);
	void RestoreDefaultFocus();
    
public slots:
    //menu
    void MenuToolsWillShow();

    void CreateNodeTriggered(QAction *nodeAction);
    void ViewportTriggered(QAction *viewportAction);
    void FileMenuTriggered(QAction *resentScene);

    //File
    void NewScene();
    void OpenScene();
    void OpenProject();
    void OpenResentScene(DAVA::int32 index);
    void SaveScene();
    void ExportAsPNG();
    void ExportAsPVR();
    void ExportAsDXT();
    
    //View
    void RestoreViews();
    void ToggleSceneInfo();

    //tools
    void Materials();
    void ConvertTextures();
    void HeightmapEditor();
    void TilemapEditor();
    void ShowSettings();
    void BakeScene();
    void Beast();
    
    //ViewOptions
    void ToggleNotPassableTerrain();
    
    
    //scene graph
    void RefreshSceneGraph();

    //
    void ReloadTexturesFromFileSystem();

	//particles editor
	void OpenParticleEditorConfig();
	void SaveParticleEditorConfig();
	void OpenParticleEditorSprite();
	
    
private:
    //create node
    void CreateNode(ResourceEditor::eNodeType type);
    //viewport
    void SetViewport(ResourceEditor::eViewportType type);
    
    void Execute(Command *command);
    
    void RegisterActions(QAction **actions, DAVA::int32 count, va_list &vl);
    
    void ClearActions(int32 count, QAction **actions);
    
private:
    
    QAction *resentSceneActions[EditorSettings::RESENT_FILES_COUNT];
    QAction *nodeActions[ResourceEditor::NODE_COUNT];
    QAction *viewportActions[ResourceEditor::VIEWPORT_COUNT];
    QAction *hidablewidgetActions[ResourceEditor::HIDABLEWIDGET_COUNT];

    QMenu *menuResentScenes;
    QAction *resentAncorAction;

	QWidget *defaultFocusWidget;
};

#endif // __QT_MAIN_WINDOW_HANDLER_H__
