#ifndef __EDITOR_BODY_CONTROL_H__
#define __EDITOR_BODY_CONTROL_H__

#include "DAVAEngine.h"
#include "CameraController.h"
#include "EditMatrixControl.h"
#include "../EditorScene.h"

#include "../Constants.h"
#include "NodesPropertyControl.h"

#include "ModificationsPanel.h"

#include "LandscapeEditorBase.h"

#include "GraphBase.h"



using namespace DAVA;


class SceneGraph;
class SceneInfoControl;
class BeastManager;
class LandscapeEditorColor;
class LandscapeEditorHeightmap;
class LandscapeToolsSelection;
class EditorBodyControl: 
        public UIControl, 
        public GraphBaseDelegate,
        public LandscapeEditorDelegate,
        public ModificationsPanelDelegate
{
    enum eConst
    {
        SCENE_OFFSET = 10, 
    };

    enum ePropertyShowState
    {
        EPSS_HIDDEN = 0,
        EPSS_ONSCREEN,
    };
    
public:
    EditorBodyControl(const Rect & rect);
    virtual ~EditorBodyControl();
    
    virtual void WillAppear();
	virtual void Update(float32 timeElapsed);
    virtual void Input(UIEvent * touch);
	virtual void Draw(const UIGeometricData &geometricData);

    virtual void SetSize(const Vector2 &newSize);

    void ReloadRootScene(const String &pathToFile);
    void ReloadNode(SceneNode *node, const String &pathToFile);
    
	void BeastProcessScene();
    virtual void DrawAfterChilds(const UIGeometricData &geometricData);
	    
    EditorScene * GetScene();
    void AddNode(SceneNode *node);
    
    void RemoveSelectedSGNode();
    SceneNode *GetSelectedSGNode(); //Scene Graph node
    
    void Refresh();
    
    
    void SetViewportSize(ResourceEditor::eViewportType viewportType);
    bool ControlsAreLocked();

	void PushDebugCamera();
	void PopDebugCamera();

    void ToggleSceneInfo();

    void GetCursorVectors(Vector3 * from, Vector3 * dir, const Vector2 &point);
    
    bool ToggleLandscapeEditor(int32 landscapeEditorMode);
    
    void RecreteFullTilingTexture();

    //LandscapeEditorDelegate
    virtual void LandscapeEditorStarted();  //Show LE Controls
    virtual void LandscapeEditorFinished(); //Hide LE Controls
    
    //ModificationsPanelDelegate
    virtual void OnPlaceOnLandscape();

    //GraphBaseDelegate
    virtual bool LandscapeEditorActive();
    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect);
    
    bool TileMaskEditorEnabled();
    
    void SetScene(EditorScene *newScene);
    void SetCameraController(CameraController *newCameraController);
    
    void SelectNodeQt(SceneNode *node);
    void OnReloadRootNodesQt();
    
    
	SceneGraph * GetSceneGraph() { return sceneGraph; }

protected:

    void InitControls();
    void PropcessIsSolidChanging();
    
	void CreateModificationPanel();
    void ReleaseModificationPanel();
	void PrepareModMatrix(const Vector2 & point);

	void PlaceOnLandscape();
	void PlaceOnLandscape(SceneNode *node);
	
	Vector3 GetIntersection(const Vector3 & start, const Vector3 & dir, const Vector3 & planeN, const Vector3 & planePos);
	void InitMoving(const Vector2 & point);
	
	
    //scene controls
    EditorScene * scene;
	Camera * activeCamera;
    UI3DView * scene3dView;
    CameraController * cameraController;

    // touch
    float32 currentTankAngle;
	bool inTouch;
	Vector2 touchStart;
	
	float32 startRotationInSec;

	//beast
	BeastManager * beastManager;

	Matrix4 startTransform;
	Matrix4 currTransform;
	Vector3 rotationCenter;
	bool isDrag;
    ModificationsPanel *modificationPanel;
	
	float32 axisSign[3];
	
	float32 moveKf;
    
    String mainFilePath;
    
    
    void SelectNodeAtTree(SceneNode *node);

	//for moving object
	Vector3 startDragPoint;
	Vector3 planeNormal;
	
	Matrix4 translate1, translate2;

	SceneNode * mainCam;
	SceneNode * debugCam;
    
    struct AddedNode
    {
        SceneNode *nodeToAdd;
        SceneNode *nodeToRemove;
        SceneNode *parent;
    };
    Vector<AddedNode> nodesToAdd;
	
	
    ResourceEditor::eViewportType currentViewportType;
    
    SceneInfoControl *sceneInfoControl;

	void PackLightmaps();
    
    //Landscape Editor
    bool savedModificatioMode;
    void CreateLandscapeEditor();
    void ReleaseLandscapeEditor();
    
    LandscapeEditorColor *landscapeEditorColor;
    LandscapeEditorHeightmap *landscapeEditorHeightmap;
    LandscapeEditorBase *currentLandscapeEditor;
    LandscapeToolsSelection *landscapeToolsSelection;
    
    //graps
    SceneGraph *sceneGraph;
    GraphBase *currentGraph;
    ePropertyShowState propertyShowState;
};



#endif // __EDITOR_BODY_CONTROL_H__