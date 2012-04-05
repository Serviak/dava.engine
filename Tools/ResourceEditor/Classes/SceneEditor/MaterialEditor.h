/*
 *  MaterialEditor.h
 *  TemplateProjectMacOS
 *
 *  Created by Alexey Prosin on 12/23/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MATERIAL_EDITOR
#define MATERIAL_EDITOR

#include "DAVAEngine.h"
#include "DraggableDialog.h"
#include "MaterialPropertyControl.h"

using namespace DAVA;

class ComboBox;
class MaterialEditor: public DraggableDialog, public UIListDelegate, public NodesPropertyDelegate
{
public:
    
    MaterialEditor();
    virtual ~MaterialEditor();
    
    void SetWorkingScene(Scene *newWorkingScene, SceneNode *newWorkingSceneNode);
    void EditMaterial(Scene *newWorkingScene, Material *newWorkingMaterial);
    
    void OnButton(BaseObject * object, void * userData, void * callerData);
    
    virtual void WillAppear();
    virtual void WillDisappear();
    virtual void UpdateInternalMaterialsVector();
    virtual void UpdateNodeMaterialsVector();

    virtual int32 ElementsCount(UIList *forList);
	virtual UIListCell *CellAtIndex(UIList *forList, int32 index);
	virtual int32 CellWidth(UIList *forList, int32 index)//calls only for horizontal orientation
	{return 20;};
	virtual int32 CellHeight(UIList *forList, int32 index);//calls only for vertical orientation
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);
    
    void SelectMaterial(int materialIndex);
    void PreparePropertiesForMaterialType(int materialType);

    //NodesPropertyDelegate
    virtual void NodesPropertyChanged();
    
protected:
    
    Vector<Material*> materials;
    UIList *materialsList;

    MaterialPropertyControl *materialProps;
    
    Scene *workingScene;
    SceneNode *workingSceneNode;
    Material *workingMaterial;
    Vector<Material*> workingNodeMaterials;
    
    int selectedMaterial;
    UIListCell *lastSelection;
    
    UIStaticText *noMaterials;
    
    //===============
    enum eDisplayMode
    {
        EDM_ALL = 0,
        EDM_SELECTED
    };
    eDisplayMode displayMode;
    
    UIButton *btnAll;
    UIButton *btnSelected;
    
    void OnAllPressed(BaseObject * object, void * userData, void * callerData);
    void OnSelectedPressed(BaseObject * object, void * userData, void * callerData);
    
    void UdpateButtons(bool showButtons);
    void RefreshList();
    Material *GetMaterial(int32 index);
};

#endif