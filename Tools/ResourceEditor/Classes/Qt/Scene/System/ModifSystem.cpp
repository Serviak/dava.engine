#include "Scene/System/ModifSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/SelectionSystem.h"

#include "Scene/EntityGroup.h"
#include "Scene/SceneEditorProxy.h"

#include "Commands/CommandsManager.h"
#include "Commands/EditorBodyControlCommands.h"

EntityModificationSystem::EntityModificationSystem(DAVA::Scene * scene, SceneCollisionSystem *colSys, SceneCameraSystem *camSys, HoodSystem *hoodSys)
	: DAVA::SceneSystem(scene)
	, collisionSystem(colSys)
	, cameraSystem(camSys)
	, hoodSystem(hoodSys)
	, inModifState(false)
	, modified(false)
{
	SetModifMode(EM_MODE_MOVE);
	SetModifAxis(EM_AXIS_Z);
}

EntityModificationSystem::~EntityModificationSystem()
{ }

void EntityModificationSystem::SetModifAxis(int axis)
{
	if(axis != EM_AXIS_NONE)
	{
		curAxis = axis;
		hoodSystem->SetSelectedAxis(axis);
	}
}

int EntityModificationSystem::GetModifAxis() const
{
	return curAxis;
}

void EntityModificationSystem::SetModifMode(int mode)
{
	curMode = mode;
	hoodSystem->SetType(mode);
}

int EntityModificationSystem::GetModifMode() const
{
	return curMode;
}

void EntityModificationSystem::Update(DAVA::float32 timeElapsed)
{ }

void EntityModificationSystem::ProcessUIEvent(DAVA::UIEvent *event)
{
	if(NULL != collisionSystem)
	{
		// current selected entities
		SceneSelectionSystem *selectionSystem = ((SceneEditorProxy *) GetScene())->selectionSystem;
		const EntityGroup *selectedEntities = selectionSystem->GetSelection();

		// remember current cursor point, when looking from current camera
		DAVA::Vector3 camPosition = cameraSystem->GetCameraPosition();
		DAVA::Vector3 camToPointDirection = cameraSystem->GetPointDirection(event->point);

		// if we are not in modification state, try to find some selected item
		// that have mouse cursor at the top of it
		if(!inModifState)
		{
			bool modifCanStart = false;

			// we can start modification only if mouse is over hood
			// on mouse is over one of currently selected items
			int mouseOverAxis = hoodSystem->GetPassingAxis();
			if(mouseOverAxis != EM_AXIS_NONE)
			{
				// allow starting modification
				modifCanStart = true;
			}
			else if(selectedEntities->Size() > 0)
			{
				// send this ray to collision system and get collision objects
				const EntityGroup *collisionEntities = collisionSystem->ObjectsRayTestFromCamera();

				// check if one of got collision objects is intersected with selected items
				// if so - we can start modification
				if(NULL != selectedEntities->IntersectedEntity(collisionEntities))
				{
					modifCanStart = true;
				}
			}

			// can we start modification???
			if(modifCanStart)
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

						// select current hood axis as active
						if(curMode == EM_MODE_MOVE || curMode == EM_MODE_ROTATE)
						{
							SetModifAxis(mouseOverAxis);
						}

						// set entities to be modified
						BeginModification(selectedEntities);

						// init some values, needed for modifications
						modifStartPos3d = CamCursorPosToModifPos(camPosition, camToPointDirection, modifEntitiesCenter);
						modifStartPos2d = event->point;
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
				DAVA::Vector3 moveOffset;
				DAVA::float32 rotateAngle;
				DAVA::float32 scaleForce;

				switch (curMode)
				{
				case EM_MODE_MOVE:
					{
						DAVA::Vector3 newPos3d = CamCursorPosToModifPos(camPosition, camToPointDirection, modifEntitiesCenter);
						moveOffset = Move(newPos3d);
						modified = true;
					}
					break;
				case EM_MODE_ROTATE:
					{
						rotateAngle = Rotate(event->point);
						modified = true;
					}
					break;
				case EM_MODE_SCALE:
					{
						scaleForce = Scale(event->point);
						modified = true;
					}
					break;
				default:
					break;
				}

				if(modified)
				{
					// say to selection system, that selected items were modified
					selectionSystem->SelectedItemsWereModified();

					// lock hood, so it wont process ui events, wont calc. scale depending on it current position
					hoodSystem->Lock();
					hoodSystem->MovePosition(moveOffset);

					// TODO:
					// emit move/rotate/scale offset/angle/force
					// ...
				}
			}
			// phase ended
			else if(event->phase == DAVA::UIEvent::PHASE_ENDED)
			{
				if(event->tid == DAVA::UIEvent::BUTTON_1)
				{
					if(modified)
					{
						for (size_t i = 0; i < modifEntities.size(); ++i)
						{
							// apply modification command
							CommandsManager::Instance()->ExecuteAndRelease(new CommandTransformObject(modifEntities[i].entity,	modifEntities[i].originalTransform, modifEntities[i].entity->GetLocalTransform()));
						}
					}

					EndModification();
					inModifState = false;
					modified = false;

					hoodSystem->Unlock();
				}
			}
		}
	}
}

void EntityModificationSystem::Draw()
{ }

void EntityModificationSystem::BeginModification(const EntityGroup *entities)
{
	// clear any priv. selection
	EndModification();

	if(NULL != entities)
	{
		for(size_t i = 0; i < entities->Size(); ++i)
		{
			DAVA::Entity *en = entities->GetSolidEntity(i);

			if(NULL == en)
			{
				en = entities->GetEntity(i);
			}

			if(NULL != en)
			{
				EntityToModify etm;
				etm.entity = en;
				etm.originalCenter = en->GetWorldTransform().GetTranslationVector();
				etm.originalTransform = en->GetLocalTransform();
				etm.moveToZeroPos.CreateTranslation(-etm.originalCenter);
				etm.moveFromZeroPos.CreateTranslation(etm.originalCenter);
				modifEntities.push_back(etm);
			}
		}

		// center of this bbox will modification center, common for all entities
		modifEntitiesCenter = entities->GetCommonBbox().GetCenter();

		// prepare translation matrix's, used before and after rotation
		moveToZeroPosRelativeCenter.CreateTranslation(-modifEntitiesCenter);
		moveFromZeroPosRelativeCenter.CreateTranslation(modifEntitiesCenter);

		// remember axis vector we are rotating around
		switch(curAxis)
		{
		case EM_AXIS_X:
		case EM_AXIS_YZ:
			rotateAround = DAVA::Vector3(1, 0, 0);
			break;
		case EM_AXIS_Y:
		case EM_AXIS_XZ:
			rotateAround = DAVA::Vector3(0, 1, 0);
			break;
		case EM_AXIS_XY:
		case EM_AXIS_Z:
			rotateAround = DAVA::Vector3(0, 0, 1);
			break;
		}

		// 2d axis projection we are rotating around
		DAVA::Vector2 rotateAxis = Cam2dProjection(modifEntitiesCenter, modifEntitiesCenter + rotateAround);

		// axis dot products
		DAVA::Vector2 zeroPos = cameraSystem->GetScreenPos(modifEntitiesCenter);
		DAVA::Vector2 xPos = cameraSystem->GetScreenPos(modifEntitiesCenter + DAVA::Vector3(1, 0, 0));
		DAVA::Vector2 yPos = cameraSystem->GetScreenPos(modifEntitiesCenter + DAVA::Vector3(0, 1, 0));
		DAVA::Vector2 zPos = cameraSystem->GetScreenPos(modifEntitiesCenter + DAVA::Vector3(0, 0, 1));

		DAVA::Vector2 vx = xPos - zeroPos;
		DAVA::Vector2 vy = yPos - zeroPos;
		DAVA::Vector2 vz = zPos - zeroPos;

		crossXY = Abs(vx.CrossProduct(vy));
		crossXZ = Abs(vx.CrossProduct(vz));
		crossYZ = Abs(vy.CrossProduct(vz));

		// real rotate should be done in direction of 2dAxis normal,
		// so calculate this normal
		rotateNormal = DAVA::Vector2(-rotateAxis.y, rotateAxis.x);
		rotateNormal.Normalize();

		// remember current selection pivot point
		SceneSelectionSystem *selectionSystem = ((SceneEditorProxy *) GetScene())->selectionSystem;
		modifPivotPoint = selectionSystem->GetPivotPoint();
	}
}

void EntityModificationSystem::EndModification()
{
	modifEntitiesCenter.Set(0, 0, 0);
	modifEntities.clear();
}

DAVA::Vector3 EntityModificationSystem::CamCursorPosToModifPos(const DAVA::Vector3 &camPosition, const DAVA::Vector3 &camPointDirection, const DAVA::Vector3 &planePoint)
{
	DAVA::Vector3 ret;
	DAVA::Vector3 planeNormal;

	printf("xy %g; xz %g; yz %g\n", crossXY, crossXZ, crossYZ);

	switch(curAxis)
	{
	case EM_AXIS_X:
		{
			if(crossXY > crossXZ)
			{
				printf("XY\n");
				planeNormal = DAVA::Vector3(0, 0, 1);
			}
			else
			{
				printf("XZ\n");
				planeNormal = DAVA::Vector3(0, 1, 0);
			}
		}
		break;
	case EM_AXIS_Y:
		{
			if(crossXY > crossYZ)
			{
				printf("XY\n");
				planeNormal = DAVA::Vector3(0, 0, 1);
			}
			else
			{
				printf("YZ\n");
				planeNormal = DAVA::Vector3(1, 0, 0);
			}
		}
		break;
	case EM_AXIS_Z:
		{
			if(crossXZ > crossYZ)
			{
				printf("XZ\n");
				planeNormal = DAVA::Vector3(0, 1, 0);
			}
			else
			{
				printf("YZ\n");
				planeNormal = DAVA::Vector3(1, 0, 0);
			}
		}
		break;
	case EM_AXIS_XZ:
		planeNormal = DAVA::Vector3(0, 1, 0);
		break;
	case EM_AXIS_YZ:
		planeNormal = DAVA::Vector3(1, 0, 0);
		break;
	case EM_AXIS_XY:
	default:
		planeNormal = DAVA::Vector3(0, 0, 1);
		break;
	}

	DAVA::Plane plane(planeNormal, planePoint);
	DAVA::float32 distance = FLT_MAX;

	//DAVA::Vector3 retOld;
	//DAVA::GetIntersectionVectorWithPlane(camPosition, camPointDirection, planeNormal, planePoint, retOld);

	plane.IntersectByRay(camPosition, camPointDirection, distance);
	ret = camPosition + (camPointDirection * distance);
	
	return ret;
}

DAVA::Vector2 EntityModificationSystem::Cam2dProjection(const DAVA::Vector3 &from, const DAVA::Vector3 &to)
{
	DAVA::Vector2 axisBegin = cameraSystem->GetScreenPos(from);
	DAVA::Vector2 axisEnd = cameraSystem->GetScreenPos(to);
	DAVA::Vector2 ret = axisEnd - axisBegin;
	ret.Normalize();

	return ret;
}

DAVA::Vector3 EntityModificationSystem::Move(const DAVA::Vector3 &newPos3d)
{
	DAVA::Vector3 moveOffset;
	DAVA::Vector3 modifPosWithLocedAxis = modifStartPos3d;
	DAVA::Vector3 deltaPos3d = newPos3d - modifStartPos3d;

	switch(curAxis)
	{
	case EM_AXIS_X:
		modifPosWithLocedAxis.x += DAVA::Vector3(1, 0, 0).DotProduct(deltaPos3d);
		break;
	case EM_AXIS_Y:
		modifPosWithLocedAxis.y += DAVA::Vector3(0, 1, 0).DotProduct(deltaPos3d);
		break;
	case EM_AXIS_Z:
		modifPosWithLocedAxis.z += DAVA::Vector3(0, 0, 1).DotProduct(deltaPos3d);
		break;
	case EM_AXIS_XY:
		modifPosWithLocedAxis.x = newPos3d.x;
		modifPosWithLocedAxis.y = newPos3d.y;
		break;
	case EM_AXIS_XZ:
		modifPosWithLocedAxis.x = newPos3d.x;
		modifPosWithLocedAxis.z = newPos3d.z;
		break;
	case EM_AXIS_YZ:
		modifPosWithLocedAxis.z = newPos3d.z;
		modifPosWithLocedAxis.y = newPos3d.y;
		break;
	}

	moveOffset = modifPosWithLocedAxis - modifStartPos3d;

	for (size_t i = 0; i < modifEntities.size(); ++i)
	{
		DAVA::Matrix4 moveModification;
		moveModification.Identity();
		moveModification.CreateTranslation(moveOffset);

		DAVA::Matrix4 newLocalTransform = modifEntities[i].originalTransform * moveModification;
		modifEntities[i].entity->SetLocalTransform(newLocalTransform);
	}

	return moveOffset;
}

DAVA::float32 EntityModificationSystem::Rotate(const DAVA::Vector2 &newPos2d)
{
	DAVA::Vector2 rotateLength = newPos2d - modifStartPos2d;
	DAVA::float32 rotateForce = -(rotateNormal.DotProduct(rotateLength)) / 70.0f;

	for (size_t i = 0; i < modifEntities.size(); ++i)
	{
		DAVA::Matrix4 rotateModification;
		rotateModification.Identity();
		rotateModification.CreateRotation(rotateAround, rotateForce);

		switch(modifPivotPoint)
		{
		case SceneSelectionSystem::SELECTION_ENTITY_CENTER:
			// move to zero, rotate, move back to original center point
			rotateModification = (modifEntities[i].moveToZeroPos * rotateModification) * modifEntities[i].moveFromZeroPos;
			break;
		case SceneSelectionSystem::SELECTION_COMMON_CENTER:
			// move to zero relative selection center, rotate, move back to original center point
			rotateModification = (moveToZeroPosRelativeCenter * rotateModification) * moveFromZeroPosRelativeCenter;
			break;
		default:
			rotateModification.Identity();
			break;
		}

		modifEntities[i].entity->SetLocalTransform(modifEntities[i].originalTransform * rotateModification);
	}

	return rotateForce;
}

DAVA::float32 EntityModificationSystem::Scale(const DAVA::Vector2 &newPos2d)
{
	DAVA::Vector2 scaleDir = (newPos2d - modifStartPos2d);
	DAVA::float32 scaleForce;

	scaleForce = 1.0f - (scaleDir.y / 70.0f);

	if(scaleForce >= 0)
	{
		for (size_t i = 0; i < modifEntities.size(); ++i)
		{
			DAVA::Matrix4 scaleModification;
			scaleModification.Identity();
			scaleModification.CreateScale(DAVA::Vector3(scaleForce, scaleForce, scaleForce));

			switch(modifPivotPoint)
			{
			case SceneSelectionSystem::SELECTION_ENTITY_CENTER:
				// move to zero, rotate, move back to original center point
				scaleModification = (modifEntities[i].moveToZeroPos * scaleModification) * modifEntities[i].moveFromZeroPos;
				break;
			case SceneSelectionSystem::SELECTION_COMMON_CENTER:
				// move to zero relative selection center, rotate, move back to original center point
				scaleModification = (moveToZeroPosRelativeCenter * scaleModification) * moveFromZeroPosRelativeCenter;
				break;
			default:
				scaleModification.Identity();
				break;
			}
			
			modifEntities[i].entity->SetLocalTransform(modifEntities[i].originalTransform * scaleModification);
		}
	}

	return scaleForce;
}
