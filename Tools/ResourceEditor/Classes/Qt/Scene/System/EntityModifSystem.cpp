#include "Scene/System/EntityModifSystem.h"
#include "Scene/EntityGroup.h"

#include "Scene/SceneEditorProxy.h"
#include "Scene/System/SceneCameraSystem.h"
#include "Scene/System/SceneCollisionSystem.h"
#include "Scene/System/SceneSelectionSystem.h"

EntityModificationSystem::EntityModificationSystem(DAVA::Scene * scene, SceneCollisionSystem *colSys, SceneCameraSystem *camSys)
	: DAVA::SceneSystem(scene)
	, collisionSystem(colSys)
	, cameraSystem(camSys)
	, inModifState(false)
	, curMode(MODIF_MOVE)
	, curAxis(AXIS_XZ)
	, curPivotPoint(SELECTION_CENTER)
	, curHood(camSys)
{
	curHood.SetType(curMode);
}

EntityModificationSystem::~EntityModificationSystem()
{ }

void EntityModificationSystem::Update(DAVA::float32 timeElapsed)
{

}

void EntityModificationSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	if(NULL != collisionSystem)
	{
		// current selected entities
		SceneSelectionSystem *selectionSystem = ((SceneEditorProxy *) GetScene())->selectionSystem;
		const EntityGroup *selectedEntities = selectionSystem->GetSelection();

		// current cursor point, when looking from current camera
		DAVA::Vector3 camPosition = cameraSystem->GetCameraPosition();
		DAVA::Vector3 camToPointDirection = cameraSystem->GetPointDirection(event->point);

		if(NULL != selectedEntities && selectedEntities->Size() > 0)
		{
			// set initial hood position
			DAVA::Vector3 p;

			switch(curPivotPoint)
			{
			case SELECTION_CENTER:
				p = selectedEntities->GetCommonBbox().GetCenter();
				break;
			default:
				p = selectedEntities->GetBbox(0).GetCenter();
				break;
			}

			curHood.SetPosition(selectedEntities->GetCommonBbox().GetCenter());
		}
		else
		{
			curHood.SetPosition(DAVA::Vector3(0, 0, 0));
		}

		// if we are not in modification state, try to find some selected item
		// that have mouse cursor at the top of it
		if(!inModifState)
		{
			// get intersected items in the line from camera to current mouse position
			DAVA::Vector3 traceTo = camPosition + camToPointDirection * 1000.0f;

			// send this ray to hood
			curHood.RayTest(camPosition, traceTo);

			// send this ray to collision system and get collision objects
			const EntityGroup *collisionEntities = collisionSystem->RayTest(camPosition, traceTo);

			// check if one of got collision objects is intersected with selected items
			// if so - we can start modification
			DAVA::Entity* selectedCollisionEntity = GetFirstIntersectedEntity(selectedEntities, collisionEntities);
			if(NULL != selectedCollisionEntity)
			{
				// TODO:
				// emit signal about mouse is over selected object
				// to set specific cursor icon
				// ...
				// 

				if(DAVA::UIEvent::PHASE_BEGAN == event->phase)
				{
					if(event->tid == DAVA::UIEvent::BUTTON_1)
					{
						// go to modification state
						inModifState = true;

						// set entities to be modified
						BeginModification(selectedEntities);

						// init some values, needed for modifications
						modifStartPos3d = CamCursorPosToModifPos(camPosition, camToPointDirection, modifEntitiesCenter);
						modifStartPos2d = event->point;

						// set initial hood position
						curHood.SetPosition(modifEntitiesCenter);
					}
				}
			}
		}
		// or we are already in modification state
		else
		{
			// phase still continue
			if(event->phase == DAVA::UIEvent::PHASE_DRAG)
			{

				switch (curMode)
				{
				case MODIF_MOVE:
					{
						DAVA::Vector3 curPos = CamCursorPosToModifPos(camPosition, camToPointDirection, modifEntitiesCenter);
						Move(curPos);
					}
					break;
				case MODIF_ROTATE:
					{
						Rotate(event->point);
					}
					break;
				case MODIF_SCALE:
					{
						Scale(event->point);
					}
					break;
				default:
					break;
				}
			}
			// phase ended
			else if(event->phase == DAVA::UIEvent::PHASE_ENDED)
			{
				if(event->tid == DAVA::UIEvent::BUTTON_1)
				{
					// TODO:
					// apply modification commands
					// ...
					// 

					inModifState = false;
					EndModification();
				}
			}
		}
	}
}

void EntityModificationSystem::Draw()
{
	// current selected entities
	SceneSelectionSystem *selectionSystem = ((SceneEditorProxy *) GetScene())->selectionSystem;
	const EntityGroup *selectedEntities = selectionSystem->GetSelection();

	if(NULL != selectedEntities && selectedEntities->Size() > 0)
	{
		curHood.Draw();
	}
}

void EntityModificationSystem::BeginModification(const EntityGroup *entities)
{
	// clear any priv. selection
	EndModification();

	if(NULL != entities)
	{
		for(size_t i = 0; i < entities->Size(); ++i)
		{
			DAVA::Entity *en = entities->Get(i);
			if(NULL != en)
			{
				EntityToModify etm;
				etm.entity = en;
				etm.originalCenter = en->GetWorldTransform().GetTranslationVector();
				etm.originalTransform = en->GetLocalTransform();
				etm.moveToZeroPos.CreateTranslation(-etm.originalCenter);
				etm.moveFromZeroPos.CreateTranslation(etm.originalCenter);
				modifEntities.push_back(etm);

				// calc common for all selected entities bbox
				modifEntitiesBbox.AddAABBox(collisionSystem->GetBoundingBox(en));
			}
		}

		// now we has common bbox
		// center of this bbox will modification center, common for all entities
		modifEntitiesCenter = modifEntitiesBbox.GetCenter();

		// prepare translation matrix's, used before and after rotation
		moveToZeroPosRelativeCenter.CreateTranslation(-modifEntitiesCenter);
		moveFromZeroPosRelativeCenter.CreateTranslation(modifEntitiesCenter);

		// remember axis vector we are rotating around
		switch(curAxis)
		{
		case AXIS_X:
		case AXIS_XY:
			rotateAround = DAVA::Vector3(1, 0, 0);
			break;
		case AXIS_YZ:
		case AXIS_Y:
			rotateAround = DAVA::Vector3(0, 1, 0);
			break;
		case AXIS_XZ:
		case AXIS_Z:
			rotateAround = DAVA::Vector3(0, 0, 1);
			break;
		}

		// 2d axis projection we are rotating around
		DAVA::Vector2 rotateAxis = Cam2dProjection(modifEntitiesCenter, modifEntitiesCenter + rotateAround);

		// real rotate should be done in direction of 2dAxis normal,
		// so calculate this normal
		rotateNormal = DAVA::Vector2(-rotateAxis.y, rotateAxis.x);
		rotateNormal.Normalize();
	}
}

void EntityModificationSystem::EndModification()
{
	modifEntitiesCenter.Set(0, 0, 0);
	modifEntities.clear();
	modifEntitiesBbox.Empty();
}

DAVA::Entity* EntityModificationSystem::GetFirstIntersectedEntity(const EntityGroup *gr1, const EntityGroup *gr2)
{
	DAVA::Entity* ret = NULL;

	for(size_t i = 0; i < gr1->Size(); ++i)
	{
		if(gr2->HasEntity(gr1->Get(i)))
		{
			ret = gr1->Get(i);
			break;
		}
	}

	return ret;
}

DAVA::Vector3 EntityModificationSystem::CamCursorPosToModifPos(const DAVA::Vector3 &camPosition, const DAVA::Vector3 &camPointDirection, const DAVA::Vector3 &planePoint)
{
	DAVA::Vector3 ret;
	DAVA::Vector3 planeNormal;

	switch(curAxis)
	{
	case AXIS_X:
	case AXIS_Y:
	case AXIS_XY:
		planeNormal = DAVA::Vector3(0, 0, 1);
		break;
	case AXIS_Z:
	case AXIS_YZ:
		planeNormal = DAVA::Vector3(1, 0, 0);
		break;
	case AXIS_XZ:
		planeNormal = DAVA::Vector3(0, 1, 0);
		break;
	default:
		break;
	}

	DAVA::GetIntersectionVectorWithPlane(camPosition, camPointDirection, planeNormal, planePoint, ret);
	return ret;
}

DAVA::Vector2 EntityModificationSystem::Cam2dProjection(const DAVA::Vector3 &from, const DAVA::Vector3 &to)
{
	DAVA::Vector2 axisBegin = cameraSystem->GetS�reenPos(from);
	DAVA::Vector2 axisEnd = cameraSystem->GetS�reenPos(to);
	DAVA::Vector2 ret = axisEnd - axisBegin;
	ret.Normalize();

	return ret;
}

void EntityModificationSystem::Move(const DAVA::Vector3 &newPos3d)
{
	for (size_t i = 0; i < modifEntities.size(); ++i)
	{
		DAVA::Vector3 modifPosWithLocedAxis = newPos3d;

		switch(curAxis)
		{
		case AXIS_X:
			modifPosWithLocedAxis.y = modifStartPos3d.y;
			modifPosWithLocedAxis.z = modifStartPos3d.z;
			break;
		case AXIS_Y:
			modifPosWithLocedAxis.x = modifStartPos3d.x;
			modifPosWithLocedAxis.z = modifStartPos3d.z;
			break;
		case AXIS_Z:
			modifPosWithLocedAxis.x = modifStartPos3d.x;
			modifPosWithLocedAxis.y = modifStartPos3d.y;
			break;
		case AXIS_XY:
			modifPosWithLocedAxis.z = modifStartPos3d.z;
			break;
		case AXIS_XZ:
			modifPosWithLocedAxis.y = modifStartPos3d.y;
			break;
		case AXIS_YZ:
			modifPosWithLocedAxis.x = modifStartPos3d.x;
			break;
		}

		DAVA::Matrix4 moveModification;
		moveModification.Identity();
		moveModification.CreateTranslation(modifPosWithLocedAxis - modifStartPos3d);

		modifEntities[i].entity->SetLocalTransform(modifEntities[i].originalTransform * moveModification);
	}
}

void EntityModificationSystem::Rotate(const DAVA::Vector2 &newPos2d)
{
	DAVA::Vector2 rotateLength = newPos2d - modifStartPos2d;
	DAVA::float32 rotateForce = -(rotateNormal.DotProduct(rotateLength));

	for (size_t i = 0; i < modifEntities.size(); ++i)
	{
		DAVA::Matrix4 rotateModification;
		rotateModification.Identity();
		rotateModification.CreateRotation(rotateAround, rotateForce / 70.0f);

		switch(curPivotPoint)
		{
		case ENTITY_CENTER:
			// move to zero, rotate, move back to original center point
			rotateModification = (modifEntities[i].moveToZeroPos * rotateModification) * modifEntities[i].moveFromZeroPos;
			break;
		case SELECTION_CENTER:
			// move to zero relative selection center, rotate, move back to original center point
			rotateModification = (moveToZeroPosRelativeCenter * rotateModification) * moveFromZeroPosRelativeCenter;
			break;
		default:
			rotateModification.Identity();
			break;
		}

		modifEntities[i].entity->SetLocalTransform(modifEntities[i].originalTransform * rotateModification);
	}
}

void EntityModificationSystem::Scale(const DAVA::Vector2 &newPos2d)
{

}
