/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Scene3D/Systems/RenderUpdateSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/SceneNode.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Frustum.h"
#include "Scene3D/Camera.h"

#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Scene3D/Scene.h"

namespace DAVA
{
RenderUpdateSystem::RenderUpdateSystem()
{
    Scene::GetActiveScene()->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
}

RenderUpdateSystem::~RenderUpdateSystem()
{
}

void RenderUpdateSystem::ImmediateEvent(SceneNode * entity, uint32 event)
{
    if (event == EventSystem::WORLD_TRANSFORM_CHANGED)
    {
        // Update new transform pointer, and mark that transform is changed
        Matrix4 * worldTransformPointer = entity->GetTransformComponent()->GetWorldTransform();
        entity->GetRenderComponent()->GetRenderObject()->SetWorldTransformPtr(worldTransformPointer);
    }
    
    //if (event == EventSystem::ACTIVE_CAMERA_CHANGED)
    {
        // entity->GetCameraComponent();
        // RenderSystem::
    }
}
    
void RenderUpdateSystem::AddEntity(SceneNode * entity)
{
    RenderObject * renderObject = entity->GetRenderComponent()->GetRenderObject();
    if (!renderObject)return;

    entityObjectMap.Insert(entity, renderObject);
    RenderSystem::Instance()->RenderPermanent(renderObject);
}

void RenderUpdateSystem::RemoveEntity(SceneNode * entity)
{
    RenderObject * renderObject = entityObjectMap.Value(entity);
    if (!renderObject)return;

    RenderSystem::Instance()->RemoveFromRender(renderObject);
}
    
void RenderUpdateSystem::Process()
{
    RenderSystem::Instance()->Process();
}
    
};