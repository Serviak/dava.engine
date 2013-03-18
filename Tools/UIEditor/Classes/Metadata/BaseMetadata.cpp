	//
//  BaseMetadata.cpp
//  UIEditor
//
//  Created by Yuri Coder on 10/15/12.
//
//

#include "BaseMetadata.h"
#include "HierarchyTreeController.h"

using namespace DAVA;

const Vector2 BaseMetadata::INITIAL_CONTROL_SIZE = Vector2(100, 30);

BaseMetadata::BaseMetadata(QObject *parent) :
    QObject(parent),
    activeParamID(BaseMetadataParams::BaseMetadataID_UNKNOWN)
{
}

BaseMetadata::~BaseMetadata()
{
    treeNodeParams.clear();
}

void BaseMetadata::SetupParams(const METADATAPARAMSVECT& params)
{
    CleanupParams();
    treeNodeParams = params;

    // The Active Param needs to be defined explicitely.
    activeParamID = BaseMetadataParams::BaseMetadataID_UNKNOWN;
}

const METADATAPARAMSVECT& BaseMetadata::GetParams()
{
    return treeNodeParams;
}

void BaseMetadata::CleanupParams()
{
    treeNodeParams.clear();
    activeParamID = BaseMetadataParams::BaseMetadataID_UNKNOWN;
}

// Get the list of Params currently in context.
int BaseMetadata::GetParamsCount() const
{
    return this->treeNodeParams.size();
}

// Get the active Parameter in context.
BaseMetadataParams::METADATAPARAMID BaseMetadata::GetActiveParamID() const
{
    return this->activeParamID;
}

// Set the active Parameter in context.
void BaseMetadata::SetActiveParamID(BaseMetadataParams::METADATAPARAMID paramID)
{
    if (VerifyParamID(paramID))
    {
        this->activeParamID = paramID;
    }
}

bool BaseMetadata::VerifyActiveParamID() const
{
    return VerifyParamID(this->activeParamID);
}

bool BaseMetadata::VerifyParamID(BaseMetadataParams::METADATAPARAMID paramID) const
{
    if (treeNodeParams.empty())
    {
        Logger::Error("No Framework Controls defined to setup the Param ID! %i", paramID);
        return false;
    }
    
    if (paramID < 0 || paramID >= GetParamsCount())
    {
        Logger::Error("Param ID %i passed is less of 0 or more than Params Count!",
                      paramID, GetParamsCount());
        return false;
    }
    
    return true;
}

// Initialize the control(s) attached.
void BaseMetadata::InitializeControl(const String& controlName, const Vector2& position)
{
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        UIControl* control = this->treeNodeParams[i].GetUIControl();

        control->SetName(controlName);
        control->SetSize(INITIAL_CONTROL_SIZE);
        control->SetPosition(position);
        
        control->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    }
}

HierarchyTreeNode::HIERARCHYTREENODEID BaseMetadata::GetActiveTreeNodeID() const
{
    if (VerifyActiveParamID() == false)
    {
        return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
    }
    
    return treeNodeParams[activeParamID].GetTreeNodeID();
}

UIControl* BaseMetadata::GetActiveUIControl() const
{
    if (VerifyActiveParamID() == false)
    {
        return NULL;
    }
    
    return treeNodeParams[activeParamID].GetUIControl();
}

UIControl::eControlState BaseMetadata::GetUIControlState() const
{
    return this->uiControlState;
}

void BaseMetadata::SetUIControlState(UIControl::eControlState controlState)
{
    this->uiControlState = controlState;

	if(NULL != GetActiveUIControl())
	{
		GetActiveUIControl()->SetState(controlState);
	}
}

HierarchyTreeNode* BaseMetadata::GetTreeNode(BaseMetadataParams::METADATAPARAMID paramID) const
{
    if (VerifyParamID(paramID) == false)
    {
        Logger::Error("Param ID %i is invalid for the Metadata!", paramID);
        return NULL;
    }
    
    HierarchyTreeNode::HIERARCHYTREENODEID nodeID = treeNodeParams[paramID].GetTreeNodeID();
    HierarchyTreeNode* node = HierarchyTreeController::Instance()->GetTree().GetNode(nodeID);
    if (node == NULL)
    {
        Logger::Error("Unable to found Platform Tree Node for Node ID %i!", nodeID);
        return NULL;
    }
    
    return node;
}

HierarchyTreeNode* BaseMetadata::GetActiveTreeNode() const
{
    return GetTreeNode(GetActiveParamID());
}

UIControl::eControlState BaseMetadata::GetCurrentStateForLocalizedText() const
{
    // For non state-aware controls always return fixed state.
    return UIControl::STATE_NORMAL;
}

bool BaseMetadata::IsStateDirty(UIControl::eControlState controlState)
{
    // If at least one state for the Metadata attached is dirty - return true.
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        HierarchyTreeNode* treeNode = GetTreeNode(i);
        if (!treeNode)
        {
            continue;
        }
        
        // State is dirty if at least one property exist in the dirty map.
        if (treeNode->GetExtraData().IsStatePropertyDirtyMapEmpty(controlState) == false)
        {
            return true;
        }
    }

    return false;
}

bool BaseMetadata::IsStateDirtyForProperty(UIControl::eControlState controlState, const QString& propertyName)
{
    // If at least one state for the Metadata attached is dirty - return true.
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        HierarchyTreeNode* treeNode = GetTreeNode(i);
        if (!treeNode)
        {
            continue;
        }

        if (treeNode->GetExtraData().IsStatePropertyDirty(controlState, propertyName))
        {
            return true;
        }
    }
    
    return false;
}

void BaseMetadata::SetStateDirtyForProperty(UIControl::eControlState controlState, const QString& propertyName,
                                            bool value)
{
    // Perform set for all nodes.
    int paramsCount = this->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        HierarchyTreeNode* treeNode = GetTreeNode(i);
        if (!treeNode)
        {
            continue;
        }
        
        treeNode->GetExtraData().SetStatePropertyDirty(controlState, propertyName, value);
    }
}

bool BaseMetadata::IsActiveStateDirtyForProperty(const QString& propertyName)
{
    return IsStateDirtyForProperty(this->uiControlState, propertyName);
}

void BaseMetadata::SetActiveStateDirtyForProperty(const QString& propertyName, bool value)
{
    SetStateDirtyForProperty(this->uiControlState, propertyName, value);
}

// Get the "reference" state to compare all other with.
UIControl::eControlState BaseMetadata::GetReferenceState()
{
    return UIControl::STATE_NORMAL;
}

Color BaseMetadata::QTColorToDAVAColor(const QColor& qtColor) const
{
    return Color(qtColor.redF(), qtColor.greenF(), qtColor.blueF(), qtColor.alphaF());
}

QColor BaseMetadata::DAVAColorToQTColor(const Color& davaColor) const
{
    return QColor(davaColor.r * 0xFF, davaColor.g * 0xFF, davaColor.b * 0xFF, davaColor.a * 0xFF);
}