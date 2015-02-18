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



#include <QApplication>

#include "Scene/SceneEditor2.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Qt/Settings/SettingsManager.h"

// framework
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Scene.h"
#include "Input/InputSystem.h"
#include "Input/KeyboardDevice.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

#include "Scene3D/Systems/Controller/WASDControllerSystem.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"

#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Components/Controller/RotationControllerComponent.h"

#include "Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h"


#include "Commands2/RemoveComponentCommand.h"
#include "Commands2/AddComponentCommand.h"




#include "../StringConstants.h"

#include "../../Main/QtUtils.h"
#include "Qt/Settings/SettingsManager.h"

#define SPEED_ARRAY_SIZE	4

SceneCameraSystem::SceneCameraSystem(DAVA::Scene * scene)
	: SceneSystem(scene)
	, debugCamerasCreated(false)
	, curSceneCamera(NULL)
	, animateToNewPos(false)
	, animateToNewPosTime(0)
	, distanceToCamera(0.f)
	, activeSpeedIndex(0)
{
	renderState = RenderManager::Instance()->Subclass3DRenderState(RenderStateData::STATE_COLORMASK_ALL | RenderStateData::STATE_DEPTH_WRITE);
}

SceneCameraSystem::~SceneCameraSystem()
{
	SafeRelease(curSceneCamera);
}

DAVA::Camera* SceneCameraSystem::GetCurCamera() const
{
    return curSceneCamera;
}

DAVA::Vector3 SceneCameraSystem::GetPointDirection(const DAVA::Vector2 &point) const
{
	DAVA::Vector3 dir;

	if(NULL != curSceneCamera)
	{
		DAVA::Vector3 pos = curSceneCamera->GetPosition();
		dir = curSceneCamera->UnProject(point.x, point.y, 0, viewportRect);
		dir -= pos;
	}

	return dir;
}

DAVA::Vector3 SceneCameraSystem::GetCameraPosition() const
{
	DAVA::Vector3 pos;

	if(NULL != curSceneCamera)
	{
		pos = curSceneCamera->GetPosition();
	}

	return pos;
}

DAVA::Vector3 SceneCameraSystem::GetCameraDirection() const
{
	DAVA::Vector3 dir;

	if(NULL != curSceneCamera)
	{
		dir = curSceneCamera->GetDirection();
	}

	return dir;
}

DAVA::float32 SceneCameraSystem::GetMoveSpeed()
{
    DAVA::float32 speed = 1.0;

    switch(activeSpeedIndex)
    {
        case 0: speed = SettingsManager::GetValue(Settings::Scene_CameraSpeed0).AsFloat(); break;
        case 1: speed = SettingsManager::GetValue(Settings::Scene_CameraSpeed1).AsFloat(); break;
        case 2: speed = SettingsManager::GetValue(Settings::Scene_CameraSpeed2).AsFloat(); break;
        case 3: speed = SettingsManager::GetValue(Settings::Scene_CameraSpeed3).AsFloat(); break;
    }

	return speed;
}

DAVA::uint32 SceneCameraSystem::GetActiveSpeedIndex()
{
	return activeSpeedIndex;
}

void SceneCameraSystem::SetMoveSpeedArrayIndex(DAVA::uint32 index)
{
	DVASSERT(index < 4);
	activeSpeedIndex = index;
}

void SceneCameraSystem::SetViewportRect(const DAVA::Rect &rect)
{
	viewportRect = rect;

	RecalcCameraAspect();
}

const DAVA::Rect SceneCameraSystem::GetViewportRect()
{
	return viewportRect;
}

DAVA::Vector2 SceneCameraSystem::GetScreenPos(const DAVA::Vector3 &pos3) const
{
	DAVA::Vector3 ret3d = GetScreenPosAndDepth(pos3);
	return DAVA::Vector2(ret3d.x, ret3d.y);
}

DAVA::Vector3 SceneCameraSystem::GetScreenPosAndDepth(const DAVA::Vector3 &pos3) const
{
	DAVA::Vector3 ret;

	if(NULL != curSceneCamera)
	{
		if(curSceneCamera)
		{
			ret = curSceneCamera->GetOnScreenPositionAndDepth(pos3, viewportRect);
		}
	}

	return ret;
}

DAVA::Vector3 SceneCameraSystem::GetScenePos(const DAVA::float32 x, const DAVA::float32 y, const DAVA::float32 z) const
{
	DAVA::Vector3 ret;

	if(NULL != curSceneCamera)
	{
		ret = curSceneCamera->UnProject(x, y, z, viewportRect);
	}

	return ret;
}

void SceneCameraSystem::LookAt(const DAVA::AABBox3 &box)
{
	if(NULL != curSceneCamera && !box.IsEmpty())
	{
		DAVA::Vector3 pos = curSceneCamera->GetPosition();
		DAVA::Vector3 targ = curSceneCamera->GetTarget();
		DAVA::Vector3 dir = targ - pos;
		dir.Normalize();

		float32 boxSize = ((box.max - box.min).Length());
		const Vector3 c = box.GetCenter();

		pos = c - (dir * (boxSize + curSceneCamera->GetZNear() * 1.5f));
		targ = c;

		MoveTo(pos, targ);
	}
}

void SceneCameraSystem::MoveTo(const DAVA::Vector3 &pos)
{
	if(NULL != curSceneCamera)
	{
		MoveTo(pos, curSceneCamera->GetTarget());
	}
}

void SceneCameraSystem::MoveTo(const DAVA::Vector3 &pos, const DAVA::Vector3 &target)
{
    if(NULL != curSceneCamera && !curSceneCamera->GetIsOrtho())
    {
        animateToNewPos = true;
        animateToNewPosTime = 0;

        newPos = pos;
        newTar = target;
    }
}

void SceneCameraSystem::Process(float timeElapsed)
{
    //TODO: set move speed
    SceneEditor2 *scene = static_cast<SceneEditor2 *>(GetScene());

    WASDControllerSystem *wasdSystem = scene->wasdSystem;
    if(wasdSystem)
    {
        wasdSystem->SetMoveSpeed(GetMoveSpeed());
    }
    RotationControllerSystem *rotationSystem = scene->rotationSystem;
    if(rotationSystem)
    {
        rotationSystem->SetRotationSpeeed(0.15f);
        
        HoodSystem *hoodSystem = scene->hoodSystem;
        if(NULL != hoodSystem)
        {
            rotationSystem->SetRotationPoint(hoodSystem->GetPosition());
        }
    }
    //TODO: set move speed

    
	if(!debugCamerasCreated)
	{
		CreateDebugCameras();
	}

	if(NULL != scene)
	{
		DAVA::Camera* camera = scene->GetDrawCamera();

		// is current camera in scene changed?
		if(curSceneCamera != camera)
		{
			// update collision object for last camera
			if(NULL != curSceneCamera)
			{
				SceneCollisionSystem *collSystem = ((SceneEditor2 *) GetScene())->collisionSystem;
				collSystem->UpdateCollisionObject(GetEntityFromCamera(curSceneCamera));
			}
			
			// remember current scene camera
			SafeRelease(curSceneCamera);
			curSceneCamera = camera;
			SafeRetain(curSceneCamera);

			// Recalc camera aspect
			RecalcCameraAspect();
		}
	}

	// camera move animation
	MoveAnimate(timeElapsed);
}

void SceneCameraSystem::Input(DAVA::UIEvent *event)
{
    if(event->phase == UIEvent::PHASE_KEYCHAR)
    {
        if(('+' == event->keyChar)|| ('-' == event->keyChar))
        {
            Entity *entity = GetEntityWithEditorCamera();
            SnapToLandscapeControllerComponent *snapComponent = GetSnapToLandscapeControllerComponent(entity);
            if(snapComponent)
            {
                float32 height = SettingsManager::Instance()->GetValue(Settings::Scene_CameraHeightOnLandscapeStep).AsFloat();
                if('+' == event->keyChar)
                {
                    snapComponent->SetHeightOnLandscape(snapComponent->GetHeightOnLandscape() + height);
                }
                else
                {
                    snapComponent->SetHeightOnLandscape(snapComponent->GetHeightOnLandscape() - height);
                }
            }
        }
    }
}

void SceneCameraSystem::Draw()
{
	//int oldState = DAVA::RenderManager::Instance()->GetState();
	//DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_TEST);
	
	SceneEditor2 *sceneEditor = (SceneEditor2 *) GetScene();
	if(NULL != sceneEditor)
	{
		SceneCollisionSystem *collSystem = sceneEditor->collisionSystem;

		if(NULL != collSystem)
		{
			DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0, 1.0f, 0, 1.0f));		

			DAVA::Set<DAVA::Entity *>::iterator it = sceneCameras.begin();
			for(; it != sceneCameras.end(); ++it)
			{
				DAVA::Entity *entity = *it;
				DAVA::Camera *camera = GetCamera(entity);

				if(NULL != entity && NULL != camera && camera != curSceneCamera)
				{
					AABBox3 worldBox;
					AABBox3 collBox = collSystem->GetBoundingBox(*it);
					Matrix4 transform;

					transform.Identity();
					transform.SetTranslationVector(camera->GetPosition());
					collBox.GetTransformedBox(transform, worldBox);	
					DAVA::RenderHelper::Instance()->FillBox(worldBox, renderState);
				}
			}

			DAVA::RenderManager::Instance()->ResetColor();
		}
	}
}

void SceneCameraSystem::ProcessCommand(const Command2 *command, bool redo)
{
}

void SceneCameraSystem::AddEntity(DAVA::Entity * entity)
{
	DAVA::Camera *camera = GetCamera(entity);
	if(NULL != camera)
	{
		sceneCameras.insert(entity);
	}
}

void SceneCameraSystem::RemoveEntity(DAVA::Entity * entity)
{
	DAVA::Set<DAVA::Entity *>::iterator it = sceneCameras.find(entity);
	if(it != sceneCameras.end())
	{
		sceneCameras.erase(it);
	}
}


void SceneCameraSystem::CreateDebugCameras()
{
	DAVA::Scene *scene = GetScene();

	// add debug cameras
	// there already can be other cameras in scene
	if(NULL != scene)
	{
		DAVA::Camera *topCamera = new DAVA::Camera();
		topCamera->SetUp(DAVA::Vector3(0.0f, 0.0f, 1.0f));
		topCamera->SetPosition(DAVA::Vector3(-50.0f, 0.0f, 50.0f));
		topCamera->SetTarget(DAVA::Vector3(0.0f, 0.1f, 0.0f));
		DAVA::float32 cameraFov = SettingsManager::GetValue(Settings::Scene_CameraFOV).AsFloat();
		DAVA::float32 cameraNear = SettingsManager::GetValue(Settings::Scene_CameraNear).AsFloat();
		DAVA::float32 cameraFar = SettingsManager::GetValue(Settings::Scene_CameraFar).AsFloat();
		topCamera->SetupPerspective(cameraFov, 320.0f / 480.0f, cameraNear, cameraFar);
		topCamera->SetAspect(1.0f);

		DAVA::Entity *topCameraEntity = new DAVA::Entity();
		topCameraEntity->SetName(ResourceEditor::EDITOR_DEBUG_CAMERA);
		topCameraEntity->AddComponent(new DAVA::CameraComponent(topCamera));
        topCameraEntity->AddComponent(new DAVA::WASDControllerComponent());
        topCameraEntity->AddComponent(new DAVA::RotationControllerComponent());
		scene->InsertBeforeNode(topCameraEntity, scene->GetChild(0));

		// set current default camera
		if(NULL == scene->GetCurrentCamera())
		{
			scene->SetCurrentCamera(topCamera);
		}
        
        scene->AddCamera(topCamera);

		SafeRelease(topCamera);

		debugCamerasCreated = true;
	}
}

void SceneCameraSystem::RecalcCameraAspect()
{
	if(NULL != curSceneCamera)
	{
		DAVA::float32 aspect = 1.0;

		if(0 != viewportRect.dx && 0 != viewportRect.dy)
		{
			aspect = viewportRect.dx / viewportRect.dy;
		}

		curSceneCamera->SetAspect(aspect);
	}
}



void SceneCameraSystem::MoveAnimate(DAVA::float32 timeElapsed)
{
	static const DAVA::float32 animationTime = 3.0f;

	if(NULL != curSceneCamera && animateToNewPos)
	{
		DAVA::Vector3 pos = curSceneCamera->GetPosition();
		DAVA::Vector3 tar = curSceneCamera->GetTarget();

		if((pos != newPos || tar != newTar) && animateToNewPosTime < animationTime)
		{
			animateToNewPosTime += timeElapsed;

			DAVA::float32 fnX = animateToNewPosTime / animationTime;
			DAVA::float32 fnY = sin(1.57 * fnX);
			
			DAVA::Vector3 dPos = newPos - pos;
			DAVA::Vector3 dTar = newTar - tar;

			//dPos = dPos * fnY;
			//dTarg = dTarg * fnY;

			if(dPos.Length() > 0.01f) dPos = dPos * fnY;
			if(dTar.Length() > 0.01f) dTar = dTar * fnY;

			curSceneCamera->SetPosition(pos + dPos);
			curSceneCamera->SetTarget(tar + dTar);
		}
		else
		{
			animateToNewPos = false;
			animateToNewPosTime = 0;

			curSceneCamera->SetTarget(newTar);
			curSceneCamera->SetPosition(newPos);

            SceneEditor2 *sc = static_cast<SceneEditor2 *>(GetScene());
            sc->rotationSystem->RecalcCameraViewAngles(curSceneCamera);
		}
        
        UpdateDistanceToCamera();
	}
}

void SceneCameraSystem::UpdateDistanceToCamera()
{
    SceneEditor2 *sc = (SceneEditor2 *)GetScene();
    
    Vector3 center = sc->selectionSystem->GetSelection().GetCommonBbox().GetCenter();
    
    const Camera *cam = GetScene()->GetCurrentCamera();
    if(cam)
    {
        distanceToCamera = (cam->GetPosition() - center).Length();
    }
    else
    {
        distanceToCamera = 0.f;
    }
}

DAVA::float32 SceneCameraSystem::GetDistanceToCamera() const
{
    return distanceToCamera;
}

DAVA::Entity* SceneCameraSystem::GetEntityFromCamera(DAVA::Camera *c) const
{
	DAVA::Entity *ret = NULL;

	DAVA::Set<DAVA::Entity *>::iterator it = sceneCameras.begin();
	for(; it != sceneCameras.end(); ++it)
	{
		DAVA::Entity *entity = *it;
		DAVA::Camera *camera = GetCamera(entity);

		if(camera == c)
		{
			ret = entity;
			break;
		}
	}

	return ret;
}

void SceneCameraSystem::GetRayTo2dPoint(const DAVA::Vector2 &point, DAVA::float32 maxRayLen, DAVA::Vector3 &outPointFrom, DAVA::Vector3 &outPointTo) const
{
    if(NULL != curSceneCamera)
    {
        DAVA::Vector3 camPos = GetCameraPosition();
        DAVA::Vector3 camDir = GetPointDirection(point);

        if(curSceneCamera->GetIsOrtho())
        {
            outPointFrom = DAVA::Vector3(camDir.x, camDir.y, camPos.z);
            outPointTo = DAVA::Vector3(camDir.x, camDir.y, camPos.z + maxRayLen);
        }
        else
        {
            outPointFrom = camPos;
            outPointTo = outPointFrom + camDir * maxRayLen;
        }
    }
}


DAVA::Entity* SceneCameraSystem::GetEntityWithEditorCamera() const
{
    int32 cameraCount = GetScene()->GetCameraCount();
    for(int32 i = 0; i < cameraCount; ++i)
    {
        Camera *c = GetScene()->GetCamera(i);
        Entity *e = GetEntityFromCamera(c);
        if(e && e->GetName() == ResourceEditor::EDITOR_DEBUG_CAMERA)
        {
            return e;
        }
    }
    
    return NULL;
}


bool SceneCameraSystem::SnapEditorCameraToLandscape(bool snap)
{
    Entity *entity = GetEntityWithEditorCamera();
    if(!entity) return false;

    SceneEditor2 *scene = static_cast<SceneEditor2 *>(GetScene());
    
    SnapToLandscapeControllerComponent *snapComponent = GetSnapToLandscapeControllerComponent(entity);
    if(snap)
    {
        if(!snapComponent)
        {
            float32 height = SettingsManager::Instance()->GetValue(Settings::Scene_CameraHeightOnLandscape).AsFloat();
            
            snapComponent = static_cast<SnapToLandscapeControllerComponent *>(Component::CreateByType(Component::SNAP_TO_LANDSCAPE_CONTROLLER_COMPONENT));
            snapComponent->SetHeightOnLandscape(height);

            scene->Exec(new AddComponentCommand(entity, snapComponent));
        }
    }
    else if(snapComponent)
    {
        scene->Exec(new RemoveComponentCommand(entity, snapComponent));
    }
    
    return true;
}

bool SceneCameraSystem::IsEditorCameraSnappedToLandscape() const
{
    Entity *entity = GetEntityWithEditorCamera();
    return (GetSnapToLandscapeControllerComponent(entity) != NULL);
}


