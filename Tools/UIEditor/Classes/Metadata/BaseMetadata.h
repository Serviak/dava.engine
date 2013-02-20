//
//  BaseMetadata.h
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#ifndef __UIEditor__BaseMetadata__
#define __UIEditor__BaseMetadata__

#include <QObject>
#include <QPoint>
#include <QColor>

#include "HierarchyTreeNode.h"
#include "UI/UIControl.h"

#include "BaseMetadataParams.h"

namespace DAVA {

// Base class for Metadata.
class BaseMetadata : public QObject
{
    Q_OBJECT
    
public:
    // ExtraData update flags.
    enum eExtraDataUpdateStyle
    {
        // Update the ExtraData with control values.
        UPDATE_EXTRADATA_FROM_CONTROL,
        
        // Update Control with raw ExtraData values.
        UPDATE_CONTROL_FROM_EXTRADATA_RAW,
        
        // Update Control with Localized Extradata values.
        UPDATE_CONTROL_FROM_EXTRADATA_LOCALIZED
    };
    
    BaseMetadata(QObject *parent = 0);
    ~BaseMetadata();

    // Setup/cleanup the Tree Node Metadata Params.
    void SetupParams(const METADATAPARAMSVECT& params);
    void CleanupParams();
    
    // Initialize the control(s) attached.
    virtual void InitializeControl(const String& controlName, const Vector2& position);

    // Get the list of Params attached.
    int GetParamsCount() const;
    const METADATAPARAMSVECT& GetParams();
    
    // Get/Set the active Parameter in context. All the properties operations will be executed
    // on this control.
    BaseMetadataParams::METADATAPARAMID GetActiveParamID() const;
    void SetActiveParamID(BaseMetadataParams::METADATAPARAMID paramID);
    
    // UI Control State.
    UIControl::eControlState GetUIControlState() const;
    void SetUIControlState(UIControl::eControlState controlState);

    // Apply move for all controls.
    virtual void ApplyMove(const Vector2&) {};
    
    // Apply resize for all controls.
    virtual void ApplyResize(const Rect& /*originalRect*/, const Rect& /*newRect*/) {};

    // Accessors to the Tree Node.
    HierarchyTreeNode* GetTreeNode(BaseMetadataParams::METADATAPARAMID paramID) const;
    HierarchyTreeNode* GetActiveTreeNode() const;

    // In case the control is not state-aware its Localized Text should be always
    // bound to the first state. This is defined by this virtual function.
    virtual UIControl::eControlState GetCurrentStateForLocalizedText() const;

    // Get the "reference" state to compare all other with.
    virtual UIControl::eControlState GetReferenceState();

    // Helper method to check whether at least one control's property differs from the reference ones.	
    bool IsStateDirty(UIControl::eControlState controlState);

    // The same for particular property and state.
    bool IsStateDirtyForProperty(UIControl::eControlState controlState, const QString& propertyName);
    void SetStateDirtyForProperty(UIControl::eControlState controlState, const QString& propertyName,
                                                bool value);

    // Helper for Active State.
    bool IsActiveStateDirtyForProperty(const QString& propertyName);
    void SetActiveStateDirtyForProperty(const QString& propertyName, bool value);
    
    // Helper for Colors.
    Color QTColorToDAVAColor(const QColor& qtColor) const;
    QColor DAVAColorToQTColor(const Color& davaColor) const;

    // Fill ExtraData from attached Control values. Specific for each classes.
    virtual void UpdateExtraData(HierarchyTreeNodeExtraData& /*extraData*/, eExtraDataUpdateStyle /*updateStyle*/) {};

protected:
    // Initialization constants.
    static const Vector2 INITIAL_CONTROL_SIZE;

    // Verify whether Param ID is OK.
    bool VerifyParamID(BaseMetadataParams::METADATAPARAMID paramID) const;

    // Verify whether Active Param ID is OK.
    bool VerifyActiveParamID() const;

    // Helper to access active Tree Node ID.
    HierarchyTreeNode::HIERARCHYTREENODEID GetActiveTreeNodeID() const;
    
    // Helper to access active UI Control.
    UIControl* GetActiveUIControl() const;

    // Get the UI control class name.
    virtual QString GetUIControlClassName() { return QString(); };
    
    // List of Params.
    METADATAPARAMSVECT treeNodeParams;

    // Active Parameter.
    BaseMetadataParams::METADATAPARAMID activeParamID;
    
    // UI Control State.
    UIControl::eControlState uiControlState;
};

}

#endif /* defined(__UIEditor__BaseMetadata__) */
