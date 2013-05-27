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

#include "Scene/SceneEditorProxy.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/GridSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/SelectionSystem.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/SceneSignals.h"

// framework
#include "Scene3D/SceneFileV2.h"

SceneEditorProxy::SceneEditorProxy()
	: Scene()
{
	cameraSystem = new SceneCameraSystem(this);
	AddSystem(cameraSystem, 0);

	gridSystem = new SceneGridSystem(this);
	AddSystem(gridSystem, 0);

	collisionSystem = new SceneCollisionSystem(this);
	AddSystem(collisionSystem, 0);

	hoodSystem = new HoodSystem(this, cameraSystem);
	AddSystem(hoodSystem, 0);

	selectionSystem = new SceneSelectionSystem(this, collisionSystem, hoodSystem);
	AddSystem(selectionSystem, 0);

	modifSystem = new EntityModificationSystem(this, collisionSystem, cameraSystem, hoodSystem);
	AddSystem(modifSystem, 0);

	SceneSignals::Instance()->EmitOpened(this);
}

SceneEditorProxy::~SceneEditorProxy()
{
	SceneSignals::Instance()->EmitClosed(this);
}

bool SceneEditorProxy::Load(const DAVA::FilePath &path)
{
	bool ret = false;

	Entity * rootNode = GetRootNode(path);
	if(rootNode)
	{
		ret = true;

		DAVA::Vector<DAVA::Entity*> tmpEntities;
		int entitiesCount = rootNode->GetChildrenCount();

		tmpEntities.reserve(entitiesCount);

		// remember all child pointers, but don't add them to scene in this cycle
		// because when entity is adding it is automatically removing from its old hierarchy
		for (DAVA::int32 i = 0; i < entitiesCount; ++i)
		{
			tmpEntities.push_back(rootNode->GetChild(i));
		}

		// now we can safely add entities into our hierarchy
		for (DAVA::int32 i = 0; i < (DAVA::int32) tmpEntities.size(); ++i)
		{
			AddNode(tmpEntities[i]);
		}

		curScenePath = path;
	}

	SceneSignals::Instance()->EmitLoaded(this);
	return ret;
}

bool SceneEditorProxy::Save(const DAVA::FilePath &path)
{
	bool ret = false;

	DAVA::SceneFileV2 *file = new DAVA::SceneFileV2();
	file->EnableDebugLog(false);

	DAVA::SceneFileV2::eError err = file->SaveScene(path, this);
	ret = (DAVA::SceneFileV2::ERROR_NO_ERROR == err);

	if(ret)
	{
		curScenePath = path;
	}

	SafeRelease(file);

	SceneSignals::Instance()->EmitSaved(this);
	return ret;
}

bool SceneEditorProxy::Save()
{
	return Save(curScenePath);
}

DAVA::FilePath SceneEditorProxy::GetScenePath()
{
	return curScenePath;
}

void SceneEditorProxy::SetScenePath(const DAVA::FilePath &newScenePath)
{
	curScenePath = newScenePath;
}

void SceneEditorProxy::Update(float timeElapsed)
{
	Scene::Update(timeElapsed);

	gridSystem->Update(timeElapsed);
	cameraSystem->Update(timeElapsed);
	collisionSystem->Update(timeElapsed);
	hoodSystem->Update(timeElapsed);
	selectionSystem->Update(timeElapsed);
	modifSystem->Update(timeElapsed);
}

void SceneEditorProxy::PostUIEvent(DAVA::UIEvent *event)
{
	gridSystem->ProcessUIEvent(event);
	cameraSystem->ProcessUIEvent(event);
	collisionSystem->ProcessUIEvent(event);
	hoodSystem->ProcessUIEvent(event);
	selectionSystem->ProcessUIEvent(event);
	modifSystem->ProcessUIEvent(event);
}

void SceneEditorProxy::SetViewportRect(const DAVA::Rect &newViewportRect)
{
	cameraSystem->SetViewportRect(newViewportRect);
}

void SceneEditorProxy::Draw()
{
	Scene::Draw();

	gridSystem->Draw();
	cameraSystem->Draw();
	collisionSystem->Draw();
	selectionSystem->Draw();
	hoodSystem->Draw();
	modifSystem->Draw();
}
