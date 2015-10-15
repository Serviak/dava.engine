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


#include "SceneSignals.h"
#include "SelectionSystem.h"
#include "CameraSystem.h"
#include "CollisionSystem.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Render/RenderBase.h"

#include <QApplication>

ENUM_DECLARE(SelectionSystemDrawMode)
{
	ENUM_ADD(SS_DRAW_SHAPE);
	ENUM_ADD(SS_DRAW_CORNERS);
	ENUM_ADD(SS_DRAW_BOX);
	ENUM_ADD(SS_DRAW_NO_DEEP_TEST);
}

SceneSelectionSystem::SceneSelectionSystem(DAVA::Scene * scene, SceneCollisionSystem *collSys)
	: DAVA::SceneSystem(scene)
	, selectionAllowed(true)
	, componentMaskForSelection(ALL_COMPONENTS_MASK)
	, applyOnPhaseEnd(false)
	, invalidSelectionBoxes(false)
	, collisionSystem(collSys)
	, selectionHasChanges(false)
	, curPivotPoint(ST_PIVOT_COMMON_CENTER)
{
    DAVA::RenderStateData selectionStateData;
    DAVA::RenderManager::Instance()->GetRenderStateData(DAVA::RenderState::RENDERSTATE_3D_BLEND, selectionStateData);
	
	selectionStateData.state =	DAVA::RenderStateData::STATE_BLEND |
								DAVA::RenderStateData::STATE_COLORMASK_ALL;
	selectionStateData.sourceFactor = DAVA::BLEND_SRC_ALPHA;
	selectionStateData.destFactor = DAVA::BLEND_ONE_MINUS_SRC_ALPHA;
	
	selectionNormalDrawState = DAVA::RenderManager::Instance()->CreateRenderState(selectionStateData);

	selectionStateData.state =	DAVA::RenderStateData::STATE_BLEND |
								DAVA::RenderStateData::STATE_COLORMASK_ALL |
								DAVA::RenderStateData::STATE_DEPTH_TEST;
	
	selectionDepthDrawState = DAVA::RenderManager::Instance()->CreateRenderState(selectionStateData);

    scene->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::SWITCH_CHANGED);
}

SceneSelectionSystem::~SceneSelectionSystem()
{
	if(GetScene())
	{
		GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, DAVA::EventSystem::SWITCH_CHANGED);
	}
}

void SceneSelectionSystem::ImmediateEvent(DAVA::Entity * entity, DAVA::uint32 event)
{
    if(DAVA::EventSystem::SWITCH_CHANGED == event)
    {
        for(DAVA::uint32 i = 0; i < curSelections.Size(); i++)
        {
            DAVA::Entity *selectedEntity = curSelections.GetEntity(i);

            // if switched entity selected - update it bounding box
            if(selectedEntity == entity)
            {
                invalidSelectionBoxes = true;
            }
        }
    }
}

void SceneSelectionSystem::Process(DAVA::float32 timeElapsed)
{
	ForceEmitSignals();

	if (IsLocked())
	{
		return;
	}

    // if boxes are invalid we should request them from collision system
    // and store them in selection entityGroup
    if(invalidSelectionBoxes)
    {
        for(DAVA::uint32 i = 0; i < curSelections.Size(); i++)
        {
            EntityGroupItem *item = curSelections.GetItem(i);
            item->bbox = GetSelectionAABox(curSelections.GetEntity(i));
        }

        invalidSelectionBoxes = false;
    }
}

void SceneSelectionSystem::ForceEmitSignals()
{
	if (selectionHasChanges)
	{
		// emit signals
		::SceneSignals::Instance()->EmitSelectionChanged(GetScene(), &curSelections, &curDeselections);

		selectionHasChanges = false;
		curDeselections.Clear();
	}
}

void SceneSelectionSystem::Input(DAVA::UIEvent *event)
{
	if (IsLocked() || !selectionAllowed || (0 == componentMaskForSelection))
	{
		return;
	}

    if (DAVA::UIEvent::PHASE_BEGAN == event->phase)
    {
        if (event->tid == DAVA::UIEvent::BUTTON_1)
        {
            const EntityGroup* collisionEntities = collisionSystem->ObjectsRayTestFromCamera();
            EntityGroup selectableItems = GetSelecetableFromCollision(collisionEntities);

            DAVA::Entity *firstEntity = selectableItems.GetEntity(0);
            DAVA::Entity *nextEntity = selectableItems.GetEntity(0);

            int curKeyModifiers = QApplication::keyboardModifiers();
            if (curKeyModifiers & Qt::ControlModifier)
            {
                AddSelection(firstEntity);
            }
            else if (curKeyModifiers & Qt::AltModifier)
            {
                RemSelection(firstEntity);
            }
            else
            {
                // if new selection is NULL or is one of already selected items
                // we should change current selection only on phase end
                if (nextEntity == NULL || NULL != curSelections.IntersectedEntity(&selectableItems))
                {
                    applyOnPhaseEnd = true;
                    lastSelection = nextEntity;
                }
                else
                {
                    SetSelection(nextEntity);
                }
            }
        }
    }
    else if(DAVA::UIEvent::PHASE_ENDED == event->phase)
	{
		if(event->tid == DAVA::UIEvent::BUTTON_1)
		{
			if(applyOnPhaseEnd)
			{
				applyOnPhaseEnd = false;
				SetSelection(lastSelection);
			}
		}
	}
}

void SceneSelectionSystem::Draw()
{
	if (IsLocked())
	{
		return;
	}

	if(curSelections.Size() > 0)
	{
        DAVA::int32 drawMode = SS_DRAW_SHAPE;
        DAVA::RenderManager::SetDynamicParam(DAVA::PARAM_WORLD, &DAVA::Matrix4::IDENTITY, (DAVA::pointer_size) &DAVA::Matrix4::IDENTITY);
        DAVA::UniqueHandle renderState = (!(drawMode & SS_DRAW_NO_DEEP_TEST)) ? selectionDepthDrawState : selectionNormalDrawState;

		for (DAVA::uint32 i = 0; i < curSelections.Size(); i++)
		{
            DAVA::AABBox3 selectionBox = curSelections.GetBbox(i);

			// draw selection share
			if(drawMode & SS_DRAW_SHAPE)
			{
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f));
				DAVA::RenderHelper::Instance()->DrawBox(selectionBox, 1.0f, renderState);
			}
			// draw selection share
			else if(drawMode & SS_DRAW_CORNERS)
			{
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f));
				DAVA::RenderHelper::Instance()->DrawCornerBox(selectionBox, 1.0f, renderState);
			}

			// fill selection shape
			if(drawMode & SS_DRAW_BOX)
			{
				DAVA::RenderManager::Instance()->SetColor(DAVA::Color(1.0f, 1.0f, 1.0f, 0.15f));
				DAVA::RenderHelper::Instance()->FillBox(selectionBox, renderState);
			}
		}
	}
}

void SceneSelectionSystem::SetSelection(const EntityGroup &newSelection)
{
	if (!IsLocked())
	{
		Clear();

		auto count = newSelection.Size();
		for (decltype(count) i = 0; i < count; ++i)
		{
			auto entity = newSelection.GetEntity(i);
			if (IsEntitySelectable(entity) && !curSelections.ContainsEntity(entity))
			{
				curSelections.Add(entity, GetSelectionAABox(entity));
				selectionHasChanges = true;
			}
		}

		if (selectionHasChanges)
		{
			invalidSelectionBoxes = true;
		}
	}
}


void SceneSelectionSystem::SetSelection(DAVA::Entity *entity)
{
	if(!IsLocked())
	{
		Clear();

		// add new selection
		if(NULL != entity)
		{
			AddSelection(entity);
		}
	}
}

void SceneSelectionSystem::AddSelection(DAVA::Entity *entity)
{
    if(!IsLocked())
    {
        if(IsEntitySelectable(entity) && !curSelections.ContainsEntity(entity))
        {
            EntityGroupItem selectableItem;
            
            selectableItem.entity = entity;
            selectableItem.bbox = GetSelectionAABox(entity);
            curSelections.Add(selectableItem);
            
            selectionHasChanges = true;
            
            invalidSelectionBoxes = true;
        }
    }
}

void SceneSelectionSystem::AddSelection(const EntityGroup &entities)
{
    if(!IsLocked())
    {
        for (size_t i = 0; i < entities.Size(); ++i)
        {
            const auto entity = entities.GetEntity(i);
            if (IsEntitySelectable(entity) && !curSelections.ContainsEntity(entity))
            {
                EntityGroupItem selectableItem;

                selectableItem.entity = entity;
                selectableItem.bbox = GetSelectionAABox(entity);
                curSelections.Add(selectableItem);
                selectionHasChanges = true;
                invalidSelectionBoxes = true;
            }
        }
    }
}

bool SceneSelectionSystem::IsEntitySelectable(DAVA::Entity *entity) const
{
    if(!IsLocked() && entity)
    {
        return (componentMaskForSelection & entity->GetAvailableComponentFlags());
    }
    
    return false;
}

void SceneSelectionSystem::RemSelection(DAVA::Entity *entity)
{
	if(!IsLocked())
	{
		if(curSelections.ContainsEntity(entity))
		{
			curSelections.Rem(entity);
			curDeselections.Add(entity);

			selectionHasChanges = true;
		}
	}
}

void SceneSelectionSystem::RemSelection(const EntityGroup& entities)
{
    if (!IsLocked())
    {
        for (size_t i = 0; i < entities.Size(); ++i)
        {
            auto entity = entities.GetEntity(i);
            if (curSelections.ContainsEntity(entity))
            {
                curSelections.Rem(entity);
                curDeselections.Add(entity);

                selectionHasChanges = true;
            }
        }
    }
}

void SceneSelectionSystem::Clear()
{
	if(!IsLocked())
	{
		while(curSelections.Size() > 0)
		{
			DAVA::Entity *entity = curSelections.GetEntity(0);

			curSelections.Rem(entity);
			curDeselections.Add(entity);

			selectionHasChanges = true;
		}
	}
}

EntityGroup SceneSelectionSystem::GetSelection() const
{
	return curSelections;
}

size_t SceneSelectionSystem::GetSelectionCount() const
{
	return curSelections.Size();
}

DAVA::Entity* SceneSelectionSystem::GetSelectionEntity(int index) const
{
	return curSelections.GetEntity(index);
}

bool SceneSelectionSystem::IsEntitySelected(DAVA::Entity *entity)
{
    return curSelections.ContainsEntity(entity);
}

bool SceneSelectionSystem::IsEntitySelectedHierarchically(DAVA::Entity *entity)
{
    while (entity)
    {
        if (curSelections.ContainsEntity(entity))
            return true;

        entity = entity->GetParent();
    }
    return false;
}

void SceneSelectionSystem::SelectedItemsWereModified()
{
	// don't change selection on phase end
	applyOnPhaseEnd = false;
}

void SceneSelectionSystem::SetPivotPoint(ST_PivotPoint pp)
{
	curPivotPoint = pp;
}

ST_PivotPoint SceneSelectionSystem::GetPivotPoint() const
{
	return curPivotPoint;
}


void SceneSelectionSystem::SetLocked(bool lock)
{
	SceneSystem::SetLocked(lock);
}

EntityGroup SceneSelectionSystem::GetSelecetableFromCollision(const EntityGroup *collisionEntities)
{
	EntityGroup ret;

	if(NULL != collisionEntities)
	{
		for(size_t i = 0; i < collisionEntities->Size(); ++i)
		{
			DAVA::Entity *entity = collisionEntities->GetEntity(i);
            
            if(componentMaskForSelection & entity->GetAvailableComponentFlags())
            {
                EntityGroupItem item;
                
                item.entity = GetSelectableEntity(entity);
                item.bbox = GetSelectionAABox(item.entity);
                
                ret.Add(item);
            }
		}
	}

	return ret;
}

DAVA::Entity* SceneSelectionSystem::GetSelectableEntity(DAVA::Entity* entity)
{
	DAVA::Entity *selectableEntity = NULL;
	
	if(NULL != entity)
	{
		// find most top solid entity
		DAVA::Entity *parent = entity;
		while(NULL != parent)
		{
			if(parent->GetSolid())
			{
				selectableEntity = parent;
			}

			parent = parent->GetParent();
		}

		// still not found?
		if(NULL == selectableEntity)
		{
			// let it current entity to be tread as solid
			selectableEntity = entity;
		}
	}

	return selectableEntity;
}

DAVA::AABBox3 SceneSelectionSystem::GetSelectionAABox(int index) const
{
	return curSelections.GetBbox(index);
}

DAVA::AABBox3 SceneSelectionSystem::GetSelectionAABox(DAVA::Entity *entity) const
{
	DAVA::Matrix4 beginTransform;
	beginTransform.Identity();

	return GetSelectionAABox(entity, beginTransform);
}

DAVA::AABBox3 SceneSelectionSystem::GetSelectionAABox(DAVA::Entity *entity, const DAVA::Matrix4 &transform) const
{
	DAVA::AABBox3 ret = DAVA::AABBox3(DAVA::Vector3(0, 0, 0), 0);

	if(NULL != entity)
	{
		// we will get selection bbox from collision system 
		DAVA::AABBox3 entityBox = collisionSystem->GetBoundingBox(entity);

		// add childs boxes into entity box
		for (DAVA::int32 i = 0; i < entity->GetChildrenCount(); i++)
		{
			DAVA::Entity *childEntity = entity->GetChild(i);
			DAVA::AABBox3 childBox = GetSelectionAABox(childEntity, childEntity->GetLocalTransform());

			if(entityBox.IsEmpty())
			{
				entityBox = childBox;
			}
			else
			{
				if(!childBox.IsEmpty())
				{
					entityBox.AddAABBox(childBox);
				}
			}
		}

		// we should return box with specified transformation
		if(!entityBox.IsEmpty())
		{
			entityBox.GetTransformedBox(transform, ret);
		}
	}

	return ret;
}

void SceneSelectionSystem::SetSelectionComponentMask(DAVA::uint64 mask)
{
    componentMaskForSelection = mask;

    if(curSelections.Size() != 0)
    {
        Clear();
    }
    else
    {
        selectionHasChanges = true; // magic to say to selectionModel() of scene tree to reset selection
    }
}

void SceneSelectionSystem::Activate()
{
    SetLocked(false);
}

void SceneSelectionSystem::Deactivate()
{
    SetLocked(true);
}


