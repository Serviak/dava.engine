/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/
#include "DefaultScreen.h"
#include "ScreenWrapper.h"
#include "ControlCommands.h"
#include "ItemsCommand.h"
#include "CommandsController.h"
#include "CopyPasteController.h"
#include "HierarchyTreeAggregatorControlNode.h"

#include <QMenu>
#include <QAction>
#include <QApplication>

#define SIZE_CURSOR_DELTA 5
#define MIN_DRAG_DELTA 3
#define KEY_MOVE_DELTA 5

#define MOVE_SCREEN_KEY DVKEY_SPACE

const char* MENU_PROPERTY_ID = "id";

class UISelectorControl: public UIControl
{
public:
	UISelectorControl()
	{
		RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);		
	}
	
	void SystemDraw(const UIGeometricData &/*geometricData*/)
	{
		RenderHelper::Instance()->DrawRect(GetRect());
	}
};

DefaultScreen::DefaultScreen()
{
	scale = Vector2(1.f, 1.f);
	pos = Vector2(0, 0);
	
	inputState = InputStateSelection;
	lastSelectedControl = NULL;
	
	selectorControl = new UISelectorControl();
	
	copyControlsInProcess = false;
	mouseAlreadyPressed = false;
}

DefaultScreen::~DefaultScreen()
{
	SafeRelease(selectorControl);
}

void DefaultScreen::Update(float32 /*timeElapsed*/)
{
	CheckScreenMoveState();

	//update view port
	RenderManager::Instance()->SetDrawScale(scale);
	RenderManager::Instance()->SetDrawTranslate(pos);
}

void DefaultScreen::Draw(const UIGeometricData &geometricData)
{
	UIScreen::Draw(geometricData);
}

void DefaultScreen::SystemDraw(const UIGeometricData &geometricData)
{
	UIScreen::SystemDraw(geometricData);
	
	if (inputState == InputStateSelectorControl)
		selectorControl->SystemDraw(geometricData);
}

bool DefaultScreen::IsPointInside(const Vector2& /*point*/, bool /*expandWithFocus*/)
{
	//YZ:
	//all input must be pased to input handler
	return true;
}


Vector2 DefaultScreen::LocalToInternal(const Vector2& localPoint) const
{
	Vector2 point = -pos;
	point.x += localPoint.x / scale.x;
	point.y += localPoint.y / scale.y;
	return point;
}

void DefaultScreen::SetScale(const Vector2& scale)
{
	this->scale = scale;
}

void DefaultScreen::SetPos(const Vector2& pos)
{
	this->pos = pos;
}

void DefaultScreen::Input(DAVA::UIEvent* event)
{
	switch (event->phase)
	{
		case UIEvent::PHASE_BEGAN:
		{
			MouseInputBegin(event);
		}break;
		case UIEvent::PHASE_DRAG:
		{
			MouseInputDrag(event);
		}break;
		case UIEvent::PHASE_MOVE:
		{
			ScreenWrapper::Instance()->SetCursor(event->point, GetCursor(event->point));
		}break;
		case UIEvent::PHASE_ENDED:
		{
			MouseInputEnd(event);
		}break;
		case UIEvent::PHASE_KEYCHAR:
		{
			KeyboardInput(event);
		}break;
	}
}

bool DefaultScreen::SystemInput(UIEvent *currentInput)
{
	Input(currentInput);
	return true;
}

HierarchyTreeControlNode* DefaultScreen::GetSelectedControl(const Vector2& point, const HierarchyTreeNode* parent) const
{
	if (!parent)
		return NULL;
	
	HierarchyTreeControlNode* selectedNode = NULL;
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& items = parent->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (controlNode)
		{
			UIControl* control = controlNode->GetUIObject();
			if (control &&  control->IsPointInside(point))
				selectedNode = controlNode;
		}
		
		HierarchyTreeControlNode* childSelectNode = GetSelectedControl(point, node);
		if (childSelectNode)
			selectedNode = childSelectNode;
		
		if (selectedNode)
			return selectedNode;
	}
	
	return NULL;
}

DefaultScreen::SmartSelection::childsSet::~childsSet()
{
	for (std::vector<SmartSelection *>::iterator iter = begin();
		 iter != end();
		 ++iter)
	{
		SmartSelection* item = (*iter);
		SAFE_DELETE(item);
	}
}

DefaultScreen::SmartSelection::SmartSelection(HierarchyTreeNode::HIERARCHYTREENODEID id)
{
	//child = NULL;
	this->id = id;
	parent = NULL;
}

bool DefaultScreen::SmartSelection::IsEqual(const SmartSelection* item) const
{
	if (this->id != item->id)
		return false;

	if (this->childs.size() != item->childs.size())
		return false;
	
	DefaultScreen::SmartSelection::childsSet::const_iterator leftIter;
	for (leftIter = this->childs.begin();
		 leftIter != this->childs.end();
		 ++leftIter)
	{
		const DefaultScreen::SmartSelection* left = (*leftIter);
		const DefaultScreen::SmartSelection* right = NULL;
		for (DefaultScreen::SmartSelection::childsSet::const_iterator iter = item->childs.begin();
			 iter != item->childs.end();
			 ++iter)
		{
			if ((*iter)->id == left->id)
			{
				right = (*iter);
			}
		}
		if (!right)
			return false;
		
		if (!left->IsEqual(right))
			return false;
	}
	return true;
}

HierarchyTreeNode::HIERARCHYTREENODEID DefaultScreen::SmartSelection::GetFirst() const
{
	if (childs.size())
		return (*childs.begin())->GetFirst();

	return this->id;
}

HierarchyTreeNode::HIERARCHYTREENODEID DefaultScreen::SmartSelection::GetLast() const
{
	SelectionVector selection;
	FormatSelectionVector(selection);
	// The last element in selection array is on the top - so we should get it
	if (selection.size() > 0)
	{
		return selection[selection.size() - 1];
	}

	return this->id;
}

DefaultScreen::SmartSelection::SelectionVector DefaultScreen::SmartSelection::GetAll() const
{
	SelectionVector selection;
	FormatSelectionVector(selection);
	
	return selection;
}

void DefaultScreen::SmartSelection::FormatSelectionVector(SelectionVector &selection) const
{
	for (childsSet::const_iterator iter = childs.begin();
		 iter != childs.end();
		 ++iter)
	{
		const SmartSelection* item = (*iter);
		// Add item id to set before recursive call.
		// This will prevent adding value to array in reverse order (for controls with childs)
		selection.push_back(item->id);
		
		item->FormatSelectionVector(selection);
	}
}

HierarchyTreeNode::HIERARCHYTREENODEID DefaultScreen::SmartSelection::GetNext(HierarchyTreeNode::HIERARCHYTREENODEID id) const
{
	SelectionVector selection;
	FormatSelectionVector(selection);

	bool peakNext = false;
	// From top control to bottom control
	for (int32 i = selection.size() - 1; i >= 0; i--)
	{
		if (peakNext)
			return selection[i];
		
		if (selection[i] == id)
			peakNext = true;
	}
	return HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY;
}

void DefaultScreen::SmartGetSelectedControl(SmartSelection* list, const HierarchyTreeNode* parent, const Vector2& point)
{
	if (!parent)
		return;
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& items = parent->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (!controlNode)
			continue;

		UIControl* control = controlNode->GetUIObject();
		if (!control)
			continue;
		
		if (!control->GetVisible())
			continue;
		
		Rect controlRect = GetControlRect(controlNode);
		if (controlRect.PointInside(point))
		{
			SmartSelection* newList = new SmartSelection(node->GetId());
			list->childs.push_back(newList);
			SmartGetSelectedControl(newList, node, point);
		}
		else
		{
			SmartGetSelectedControl(list, node, point);
		}
	}
}

HierarchyTreeControlNode* DefaultScreen::GetSelectedControl(const Vector2& point)
{
	const HierarchyTreeController::SELECTEDCONTROLNODES &selectedNodes = HierarchyTreeController::Instance()->GetActiveControlNodes();
	for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = selectedNodes.begin();
		 iter != selectedNodes.end();
		 ++iter)
	{
		HierarchyTreeControlNode* control = (*iter);
			
		Rect controlRect = GetControlRect(control);
		if (controlRect.PointInside(point) || GetResizeType(control, point) != ResizeTypeNoResize)
		{
			return control;
		}
	}
	return NULL;
}

HierarchyTreeControlNode* DefaultScreen::SmartGetSelectedControl(const Vector2& point)
{
	SmartSelection root(HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY);
	SmartGetSelectedControl(&root, HierarchyTreeController::Instance()->GetActiveScreen(), point);
	// DF-1593 - Always select first control (on top)
	return dynamic_cast<HierarchyTreeControlNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(root.GetLast()));
}

void DefaultScreen::GetSelectedControl(HierarchyTreeNode::HIERARCHYTREENODESLIST& list, const Rect& rect, const HierarchyTreeNode* parent) const
{
	if (!parent)
		return;
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& items = parent->GetChildNodes();
	for (HierarchyTreeNode::HIERARCHYTREENODESCONSTITER iter = items.begin();
		 iter != items.end();
		 ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (!controlNode)
			continue;
		
		UIControl* control = controlNode->GetUIObject();
		if (!control->GetVisible())
			continue;
		
		Rect controlRect = GetControlRect(controlNode);
		if (controlRect.RectIntersects(rect))
			list.push_back(node);
		
		GetSelectedControl(list, rect, node);
	}
}

HierarchyTreeController::SELECTEDCONTROLNODES DefaultScreen::GetActiveMoveControls() const
{
	HierarchyTreeController::SELECTEDCONTROLNODES list = HierarchyTreeController::Instance()->GetActiveControlNodes();
	
	//YZ we need apply only for parent object
	HierarchyTreeController::SELECTEDCONTROLNODES selectedList;
	HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter;
	for (iter = list.begin(); iter != list.end(); ++iter)
	{
		HierarchyTreeControlNode* controlNode = (*iter);
		HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter2;
		bool isChild = false;
		for (iter2 = list.begin(); iter2 != list.end(); ++iter2)
		{
			HierarchyTreeControlNode* controlNode2 = (*iter2);
			if (controlNode2->IsHasChild(controlNode))
			{
				isChild = true;
				break;
			}
		}
		if (!isChild)
			selectedList.insert(controlNode);
	}

	return selectedList;
}

void DefaultScreen::ApplyMoveDelta(const Vector2& delta)
{
	const HierarchyTreeController::SELECTEDCONTROLNODES& selectedList = GetActiveMoveControls();
	
	HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter;
	for (iter = selectedList.begin(); iter != selectedList.end(); ++iter)
	{
		HierarchyTreeControlNode* controlNode = (*iter);
		UIControl* control = controlNode->GetUIObject();
		if (control)
		{
			
			control->SetPosition(startControlPos[control] + delta);
		}
	}
}

void DefaultScreen::ResetMoveDelta()
{
	ApplyMoveDelta(Vector2(0, 0));
}

void DefaultScreen::SaveControlsPostion()
{
	startControlPos.clear();
	const HierarchyTreeController::SELECTEDCONTROLNODES& selectedList = HierarchyTreeController::Instance()->GetActiveControlNodes();
	HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter;
	for (iter = selectedList.begin(); iter != selectedList.end(); ++iter)
	{
		HierarchyTreeControlNode* controlNode = (*iter);
		UIControl* control = controlNode->GetUIObject();
		if (control)
		{
			startControlPos[control] = control->GetPosition();
		}
	}
}

void DefaultScreen::MoveControl(const Vector2& delta)
{
	ControlsMoveCommand* cmd = new ControlsMoveCommand(GetActiveMoveControls(), delta);
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

void DefaultScreen::DeleteSelectedControls()
{
	const HierarchyTreeController::SELECTEDCONTROLNODES& selectedControls = HierarchyTreeController::Instance()->GetActiveControlNodes();
	HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter;
	HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator innerIter;
	HierarchyTreeNode::HIERARCHYTREENODESLIST nodes;
	HierarchyTreeController::SELECTEDCONTROLNODES parentNodes(selectedControls);
	
	// DF-1273 - remove all child nodes of selected controls - we don't have to remove them here
	for (iter = selectedControls.begin(); iter != selectedControls.end(); ++iter)
	{
		HierarchyTreeNode *node = (*iter);
		for (innerIter = selectedControls.begin(); innerIter != selectedControls.end(); ++innerIter)
		{			
			if (node->IsHasChild(*innerIter))
			{
				parentNodes.erase(*innerIter);
			}
		}
	}
	// DF-1273 - put only "parent" nodes to delete
	for (iter = parentNodes.begin(); iter != parentNodes.end(); ++iter)
		nodes.push_back(*iter);

	if (!nodes.size())
		return;
	
	DeleteSelectedNodeCommand* cmd = new DeleteSelectedNodeCommand(nodes);
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

Qt::CursorShape DefaultScreen::GetCursor(const Vector2& point)
{
	if (inputState == InputStateScreenMove)
		return Qt::OpenHandCursor;
		
	if (inputState == InputStateSize)
		return ResizeTypeToQt(resizeType);
	
	Vector2 pos = LocalToInternal(point);
	
	Qt::CursorShape cursor = Qt::ArrowCursor;

	const HierarchyTreeController::SELECTEDCONTROLNODES& selectedControls = HierarchyTreeController::Instance()->GetActiveControlNodes();
	HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter;
	for (iter = selectedControls.begin(); iter != selectedControls.end(); ++iter)
	{
		//nodes.insert(*iter);
		const HierarchyTreeControlNode* node = (*iter);
		if (!node)
			continue;
		
		Rect rect = GetControlRect(node);
		
		if (!IsPointInsideRectWithDelta(rect, pos, SIZE_CURSOR_DELTA))
			continue;
		
		cursor = Qt::SizeAllCursor;
		
		ResizeType resize = GetResizeType(node, pos);
		if (resize == ResizeTypeNoResize)
			continue;
		
		return ResizeTypeToQt(resize);
	}

	return cursor;
}

DefaultScreen::ResizeType DefaultScreen::GetResizeType(const HierarchyTreeControlNode* selectedControlNode, const Vector2& pos) const
{
	UIControl* selectedControl = selectedControlNode->GetUIObject();
	if (!selectedControl)
	{
		return ResizeTypeNoResize;
	}
	 
	//check is resize
	Rect rect = GetControlRect(selectedControlNode);

	if (!IsPointInsideRectWithDelta(rect, pos, SIZE_CURSOR_DELTA))
		return ResizeTypeNoResize;
	
	bool horLeft = false;
	bool horRight = false;
	bool verTop = false;
	bool verBottom = false;
	if ((pos.x >= rect.x - SIZE_CURSOR_DELTA) && (pos.x <= (rect.x + SIZE_CURSOR_DELTA)))
		horLeft = true;
	if ((pos.x <= (rect.x + rect.dx + SIZE_CURSOR_DELTA)) && (pos.x >= (rect.x + rect.dx - SIZE_CURSOR_DELTA)))
		horRight = true;
	if ((pos.y >= rect.y - SIZE_CURSOR_DELTA) && (pos.y <= (rect.y + SIZE_CURSOR_DELTA)))
		verTop = true;
	if ((pos.y <= (rect.y + rect.dy + SIZE_CURSOR_DELTA)) && (pos.y >= (rect.y + rect.dy - SIZE_CURSOR_DELTA)))
		verBottom = true;
	
	if (horLeft && verTop)
		return ResizeTypeLeftTop;
	if (horRight && verBottom)
		return ResizeTypeRightBottom;
	if (horLeft && verBottom)
		return ResizeTypeLeftBottom;
	if (horRight && verTop)
		return ResizeTypeRigthTop;
	if (horLeft)
		return ResizeTypeLeft;
	if (horRight)
		return ResizeTypeRight;
	if (verTop)
		return ResizeTypeTop;
	if (verBottom)
		return ResizeTypeBottom;
	
	return ResizeTypeNoResize;
}

Qt::CursorShape DefaultScreen::ResizeTypeToQt(ResizeType resize)
{
	if (resize == ResizeTypeLeftTop || resize == ResizeTypeRightBottom)
		return Qt::SizeFDiagCursor;
	if (resize == ResizeTypeRigthTop || resize == ResizeTypeLeftBottom)
		return Qt::SizeBDiagCursor;
	if (resize == ResizeTypeLeft || resize == ResizeTypeRight)
		return Qt::SizeHorCursor;
	if (resize == ResizeTypeTop || resize == ResizeTypeBottom)
		return Qt::SizeVerCursor;

	return Qt::ArrowCursor;
}

bool DefaultScreen::IsPointInsideRectWithDelta(const Rect& rect, const Vector2& point, int32 pointDelta) const
{
    if ((point.x >= (rect.x - pointDelta)) && (point.x <= (rect.x + rect.dx + pointDelta))
		&& (point.y >= (rect.y - pointDelta)) && (point.y <= (rect.y + rect.dy + pointDelta)))
			return true;
	return false;
}

void DefaultScreen::ApplySizeDelta(const Vector2& delta)
{
	if (!lastSelectedControl)
		return;
	
	Rect rect = resizeRect;
	
	switch (resizeType)
	{
		case ResizeTypeLeft:
		{
			rect.x += delta.x;
			rect.dx -= delta.x;
		}break;
		case ResizeTypeRight:
		{
			rect.dx += delta.x;
		}break;
		case ResizeTypeTop:
		{
			rect.y += delta.y;
			rect.dy -= delta.y;
		}break;
		case ResizeTypeBottom:
		{
			rect.dy += delta.y;
		}break;
		case ResizeTypeLeftTop:
		{
			rect.x += delta.x;
			rect.dx -= delta.x;
			rect.y += delta.y;
			rect.dy -= delta.y;
		}break;
		case ResizeTypeLeftBottom:
		{
			rect.x += delta.x;
			rect.dx -= delta.x;
			rect.dy += delta.y;
		}break;
		case ResizeTypeRigthTop:
		{
			rect.dx += delta.x;
			rect.y += delta.y;
			rect.dy -= delta.y;
		}break;
		case ResizeTypeRightBottom:
		{
			rect.dx += delta.x;
			rect.dy += delta.y;
		}break;
			
		default:break;
	}
		
	lastSelectedControl->GetUIObject()->SetRect(rect);

}

void DefaultScreen::ResetSizeDelta()
{
	if (!lastSelectedControl)
		return;
	
	lastSelectedControl->GetUIObject()->SetRect(resizeRect);
}

void DefaultScreen::ResizeControl()
{
	if (!IsNeedApplyResize())
		return;
	
	if (!lastSelectedControl)
		return;
	
	Rect rect = lastSelectedControl->GetUIObject()->GetRect();
	if (rect.dx < 0)
	{
		rect.x += rect.dx;
		rect.dx = -rect.dx;
	}
	
	if (rect.dy < 0)
	{
		rect.y += rect.dy;
		rect.dy = -rect.dy;
	}

	ResetSizeDelta();
	ControlResizeCommand* cmd = new ControlResizeCommand(lastSelectedControl->GetId(), resizeRect, rect);
	CommandsController::Instance()->ExecuteCommand(cmd);
	SafeRelease(cmd);
}

bool DefaultScreen::IsNeedApplyResize() const
{
	if (!lastSelectedControl)
		return false;

	Rect rect = lastSelectedControl->GetUIObject()->GetRect();
	return rect != resizeRect;
}

void DefaultScreen::ApplyMouseSelection(const Vector2& rectSize)
{
	Rect rect = selectorControl->GetRect();
	rect.dx = rectSize.x;
	rect.dy = rectSize.y;
	selectorControl->SetRect(rect);
	if (rect.dx < 0)
	{
		rect.x += rect.dx;
		rect.dx = -rect.dx;
	}
	if (rect.dy < 0)
	{
		rect.y += rect.dy;
		rect.dy = -rect.dy;
	}
	
	//update selecetion
	HierarchyTreeController::SELECTEDCONTROLNODES oldNodes = HierarchyTreeController::Instance()->GetActiveControlNodes();
	HierarchyTreeNode::HIERARCHYTREENODESLIST newNodes;
	GetSelectedControl(newNodes, rect, HierarchyTreeController::Instance()->GetActiveScreen());
	
	for(HierarchyTreeController::SELECTEDCONTROLNODES::iterator iter = oldNodes.begin(); iter != oldNodes.end(); ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
        
        bool controlNodeFound = (controlNode && std::find(newNodes.begin(), newNodes.end(), controlNode) != newNodes.end());
        //HierarchyTreeNode* listNode = (*iter);
		if (!controlNodeFound)
			HierarchyTreeController::Instance()->UnselectControl(controlNode);
	}
	
	for(HierarchyTreeNode::HIERARCHYTREENODESLIST::iterator iter = newNodes.begin(); iter != newNodes.end(); ++iter)
	{
		HierarchyTreeNode* node = (*iter);
		HierarchyTreeControlNode* controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
		if (controlNode && oldNodes.find(controlNode) == oldNodes.end())
		{
			HierarchyTreeController::Instance()->SelectControl(controlNode);
		}
	}
}

void DefaultScreen::MouseInputBegin(const DAVA::UIEvent* event)
{
	if (this->mouseAlreadyPressed)
	{
		// No need to handle Multiple Mouse Input Begins - it causes strange issues under Windows.
		return;
	}
	this->mouseAlreadyPressed = true;

	if (event->tid == UIEvent::BUTTON_1 && CheckEnterScreenMoveState())
	{
		return;
	}

	if (inputState == InputStateScreenMove)
		return;
	
	lastSelectedControl = NULL;
	useMouseUpSelection = true;
	
	Vector2 localPoint = event->point;
	Vector2 point = LocalToInternal(localPoint);

	HierarchyTreeControlNode* selectedControlNode = GetSelectedControl(point);
	
	if (!selectedControlNode)
	{
		selectedControlNode = SmartGetSelectedControl(point);
		//If smart selection was used - we don't need additional mouseUp selection
		useMouseUpSelection = false;
	}
	
	while (selectedControlNode)
	{
		UIControl* selectedControl = selectedControlNode->GetUIObject();
		if (!selectedControl)
		{
			selectedControlNode = NULL;
			break;
		}

		ResizeType resize = GetResizeType(selectedControlNode, point);
		if (resize != ResizeTypeNoResize &&
			HierarchyTreeController::Instance()->IsControlSelected(selectedControlNode))
		{
			resizeType = resize;
			lastSelectedControl = selectedControlNode;
			resizeRect = selectedControl->GetRect();
			inputState = InputStateSize;
		}
		else
		{
			if (HierarchyTreeController::Instance()->IsNodeActive(selectedControlNode))
			{
				//Don't check active controls size anymore - we have to be able to deselect any control
				if (/*HierarchyTreeController::Instance()->GetActiveControlNodes().size() > 1 &&*/
					InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_SHIFT))
				{
					lastSelectedControl = selectedControlNode;
					//If controls was selected with SHIFT key pressed - we don't need mouseUp selection
					useMouseUpSelection = false;
				}
			}
			else
			{
				if (!InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_SHIFT))
					HierarchyTreeController::Instance()->ResetSelectedControl();
					
				HierarchyTreeController::Instance()->SelectControl(selectedControlNode);
			}
			inputState = InputStateSelection;
			SaveControlsPostion();
		}
		
		break;
	}
		
	inputPos = event->point;
	if (!selectedControlNode)
	{
		HierarchyTreeController::Instance()->ResetSelectedControl();
		inputState = InputStateSelectorControl;
		selectorControl->SetRect(Rect(point.x, point.y, 0, 0));
	}
}

void DefaultScreen::CopySelectedControls()
{
	//Set copy flag
	copyControlsInProcess = true;
	HierarchyTreeNode* parentConrol = NULL;
	//Get current selected controls on screen
	const HierarchyTreeController::SELECTEDCONTROLNODES &selectedNodes = HierarchyTreeController::Instance()->GetActiveControlNodes();
	//Get firt parent control from list of selected controls
	for (HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter = selectedNodes.begin();
		 iter != selectedNodes.end();
		 ++iter)
	{
		HierarchyTreeControlNode* control = (*iter);
		if (!control)
		{
			continue;
		}
		//skip child control when copy parent
		if (CopyPasteController::Instance()->ControlIsChild(selectedNodes, control))
		{
			continue;
		}

		//Set parent control to which will be copied all selected items
		parentConrol = control->GetParent();
		break;
	}
	
	if (parentConrol)
	{	//Copy selected items
		CopyPasteController::Instance()->CopyControls(selectedNodes);
		CopyPasteController::Instance()->Paste(parentConrol);
		SaveControlsPostion();
	}
}

void DefaultScreen::MouseInputDrag(const DAVA::UIEvent* event)
{
	if (!this->mouseAlreadyPressed)
	{
		// If the Mouse Button isn't pressed - ignore drag messages, there may be fake ones.
		return;
	}

	HandleScreenMove(event);

	Vector2 delta = GetInputDelta(event->point);
	
	if (inputState == InputStateSelection)
	{
		if ((inputPos - event->point).Length() > MIN_DRAG_DELTA)
		{
			inputState = InputStateDrag;
		}
	}
	
	if (inputState == InputStateDrag)
	{
		//If control key is pressed - we are going to copy control(s)
		if (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_CTRL) && !copyControlsInProcess)
		{
			CopySelectedControls();
		}	
	
		ApplyMoveDelta(delta);
	}
	
	if (inputState == InputStateSize)
	{
		ApplySizeDelta(delta);
	}
	
	if (inputState == InputStateSelectorControl)
	{
		ApplyMouseSelection(delta);
	}
}

void DefaultScreen::MouseInputEnd(const DAVA::UIEvent* event)
{
	// Mouse input is finished.
	this->mouseAlreadyPressed = false;

	//Always reset copy flag
	copyControlsInProcess = false;
	if (inputState == InputStateDrag)
	{
		Vector2 delta = GetInputDelta(event->point);
		ResetMoveDelta();
		MoveControl(delta);
		startControlPos.clear();

	}
	
	if (inputState == InputStateSize)
	{
		ResizeControl();
	}
	
	if (lastSelectedControl && inputState == InputStateSelection)
	{
		HierarchyTreeController::Instance()->ChangeItemSelection(lastSelectedControl);
	}
	
	//Use additional control selection. This will allow to scroll through multiple contols
	//located in the same area
	bool bResetControlPosition = true;
	if ((inputState == InputStateSelection) && useMouseUpSelection)
	{		
		Vector2 localPoint = event->point;
		Vector2 point = LocalToInternal(localPoint);
			
		switch(event->tid)
		{
			case UIEvent::BUTTON_1: // For left mouse button we use standard selection
				HandleMouseLeftButtonClick(point);
				break;
			case UIEvent::BUTTON_2: // For right mouse button we should show context menu with avaiable controls at current point
			{
				HandleMouseRightButtonClick(point);
				bResetControlPosition = false;
				break;
			}
			default: // Do nothing for other buttons
				break;
		}
	}
	
	CheckExitScreenMoveState();

	// We don't have to reset control for right mouse click. Control position is not changed and we don't select another control
	// So we have to keep previous "selection" for that case
	if (bResetControlPosition)
	{
		lastSelectedControl = NULL;
		inputState = InputStateSelection;
		startControlPos.clear();
	}
}

void DefaultScreen::HandleMouseRightButtonClick(const Vector2& point)
{
	SmartSelection selection(HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY);
	SmartGetSelectedControl(&selection, HierarchyTreeController::Instance()->GetActiveScreen(), point);
	ShowControlContextMenu(selection);
}

void DefaultScreen::HandleMouseLeftButtonClick(const Vector2& point)
{
	HierarchyTreeControlNode* selectedControlNode = SmartGetSelectedControl(point);
	if (selectedControlNode)
	{
		if (!HierarchyTreeController::Instance()->IsNodeActive(selectedControlNode))
		{
			if (!InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_SHIFT))
				HierarchyTreeController::Instance()->ResetSelectedControl();

				HierarchyTreeController::Instance()->SelectControl(selectedControlNode);
		}
		SaveControlsPostion();
	}
}

void DefaultScreen::ShowControlContextMenu(const SmartSelection& selection)
{
	QMenu menu;
	// Get the whole selection vector available
	const DefaultScreen::SmartSelection::SelectionVector selectionVector = selection.GetAll();
	for (int32 i = selectionVector.size() - 1; i >= 0; i--)
	{
		HierarchyTreeNode *node = HierarchyTreeController::Instance()->GetTree().GetNode(selectionVector[i]);
		HierarchyTreeControlNode *controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
			
		if (!controlNode)
			continue;
		// Create menu action and put nodeId to it as property
		QAction* controlAction = new QAction(controlNode->GetName(), &menu);
		controlAction->setProperty(MENU_PROPERTY_ID, selectionVector[i]);
		menu.addAction(controlAction);
	}
	
  	connect(&menu, SIGNAL(triggered(QAction*)), this, SLOT(ControlContextMenuTriggered(QAction*)));
	menu.exec(QCursor::pos());
}

void DefaultScreen::ControlContextMenuTriggered(QAction* action)
{
	// Get controlNode for specific action triggered
	int nodeId = action->property(MENU_PROPERTY_ID).toInt();
	HierarchyTreeNode *node = HierarchyTreeController::Instance()->GetTree().GetNode(nodeId);
	HierarchyTreeControlNode *controlNode = dynamic_cast<HierarchyTreeControlNode*>(node);
			
	if (controlNode)
	{
		if (!HierarchyTreeController::Instance()->IsNodeActive(controlNode))
		{
			HierarchyTreeController::Instance()->ResetSelectedControl();
			HierarchyTreeController::Instance()->SelectControl(controlNode);			
		}
		
		SaveControlsPostion();
	}
}

void DefaultScreen::KeyboardInput(const DAVA::UIEvent* event)
{
	switch (event->tid)
	{
		case DVKEY_ESCAPE:
		{
			if (inputState == InputStateDrag)
			{
				ResetMoveDelta();
				inputState = InputStateSelection;
			}
			if (inputState == InputStateSize)
			{
				ResetSizeDelta();
				inputState = InputStateSelection;
			}
		}break;
		case DVKEY_UP:
		{
			MoveControl(Vector2(0, -KEY_MOVE_DELTA));
		}break;
		case DVKEY_DOWN:
		{
			MoveControl(Vector2(0, KEY_MOVE_DELTA));
		}break;
		case DVKEY_LEFT:
		{
			MoveControl(Vector2(-KEY_MOVE_DELTA, 0));
		}break;
		case DVKEY_RIGHT:
		{
			MoveControl(Vector2(KEY_MOVE_DELTA, 0));
		}break;
		case DVKEY_DELETE:
		case DVKEY_BACKSPACE:
		{
			DeleteSelectedControls();
		}break;
		case DVKEY_C:
		{
			if (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_CTRL))
			{
				CopyPasteController::Instance()->CopyControls(HierarchyTreeController::Instance()->GetActiveControlNodes());
			}
		}break;
		case DVKEY_V:
		{
			if (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_CTRL))
			{
                HierarchyTreeController::SELECTEDCONTROLNODES selectedList = HierarchyTreeController::Instance()->GetActiveControlNodes();                
                if (selectedList.size() == 0)
                {
                    // No controls selected - paste to screen.
                    HierarchyTreeScreenNode* activeScreenNode = HierarchyTreeController::Instance()->GetActiveScreen();
                    if (activeScreenNode)
                    {
                        CopyPasteController::Instance()->Paste(activeScreenNode);
                        break;
                    }
                }

                // Get the first non-null control node and paste to it.
                HierarchyTreeController::SELECTEDCONTROLNODES::const_iterator iter;
                for (iter = selectedList.begin(); iter != selectedList.end(); ++iter)
                {
                    HierarchyTreeControlNode* controlNode = (*iter);
                    if (controlNode)
                    {
                        CopyPasteController::Instance()->Paste(controlNode);
                        break;
                    }
                }
  			}		
            break;
        }
            
		default:
			break;
	}
	
}

Vector2 DefaultScreen::GetInputDelta(const Vector2& point, bool applyScale) const
{
	Vector2 delta = point - inputPos;

	if (applyScale)
	{
		delta.x /= scale.x;
		delta.y /= scale.y;
	}

	return delta;
}

void DefaultScreen::BacklightControl(const Vector2& position)
{
	Vector2 pos = LocalToInternal(position);
	
	HierarchyTreeControlNode* newSelectedNode = SmartGetSelectedControl(pos);
	if (newSelectedNode)
	{
		if (!HierarchyTreeController::Instance()->IsNodeActive(newSelectedNode))
		{
			HierarchyTreeController::Instance()->ResetSelectedControl();
			HierarchyTreeController::Instance()->SelectControl(newSelectedNode);
		}
	}
	else if (HierarchyTreeController::Instance()->GetActiveControlNodes().size())
	{
		HierarchyTreeController::Instance()->ResetSelectedControl();
	}
}

bool DefaultScreen::IsDropEnable(const DAVA::Vector2 &position) const
{
	Vector2 pos = LocalToInternal(position);
	HierarchyTreeAggregatorControlNode* newSelectedNode = dynamic_cast<HierarchyTreeAggregatorControlNode*>(GetSelectedControl(pos, HierarchyTreeController::Instance()->GetActiveScreen()));
	if (newSelectedNode)
		return false;
	return true;
}

Rect DefaultScreen::GetControlRect(const HierarchyTreeControlNode* controlNode) const
{
	Rect rect;
	
	if (!controlNode)
		return rect;
	
	UIControl* control = controlNode->GetUIObject();
	if (!control)
		return rect;
	
	rect = control->GetRect();
	rect += controlNode->GetParentDelta(true);

	return rect;
}

bool DefaultScreen::CheckEnterScreenMoveState()
{
	// If we are here - the Mouse Down event just happened. Check whether we are
	// in appropriate state and the key is pressed.
	if (inputState == InputStateSelection && IsMoveScreenKeyPressed())
	{
		inputState = InputStateScreenMove;
		ScreenWrapper::Instance()->RequestUpdateCursor();
		inputPos = Vector2(-1, -1);

		return true;
	}
	
	return false;
}

void DefaultScreen::CheckScreenMoveState()
{
	// This is called on every frame. If the move sceeen key is released -
	// treat this as "reset Screen Move state".
	if (inputState == InputStateScreenMove && !IsMoveScreenKeyPressed())
	{
		inputState = InputStateSelection;
	}
}

void DefaultScreen::CheckExitScreenMoveState()
{
	// Reset in any case.
	if (inputState == InputStateScreenMove)
	{
		inputState = InputStateSelection;
		ScreenWrapper::Instance()->RequestUpdateCursor();
	}
}

bool DefaultScreen::IsMoveScreenKeyPressed()
{
	//return InputSystem::Instance()->GetKeyboard()->IsKeyPressed(MOVE_SCREEN_KEY);
	Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
	return (modifiers & Qt::AltModifier);
}

void DefaultScreen::HandleScreenMove(const DAVA::UIEvent* event)
{
	if (inputState == InputStateScreenMove)
	{
		const Vector2& pos = event->point;
		if (Abs(inputPos.x + 1) < 0.1f && (inputPos.y + 1) < 0.1f)
		{
			inputPos = pos;
		}

		// In this particular case don't take Scale into account.
		Vector2 delta = GetInputDelta(pos, false);
		ScreenWrapper::Instance()->RequestViewMove(-delta);
		inputPos = pos;
	}
}