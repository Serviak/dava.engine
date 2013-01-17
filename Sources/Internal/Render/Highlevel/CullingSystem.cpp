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
#include "Render/Highlevel/CullingSystem.h"
#include "Scene3D/SceneNode.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Frustum.h"
#include "Render/Highlevel/Camera.h"

namespace DAVA
{

CullingSystem::CullingSystem()
{
}

CullingSystem::~CullingSystem()
{
}
    
void CullingSystem::ImmediateUpdate(SceneNode * entity)
{
    RenderObject * renderObject = ((RenderComponent*)entity->GetComponent(Component::RENDER_COMPONENT))->GetRenderObject();
    if (!renderObject)return;
    
    if (renderObject->GetRemoveIndex() == -1) // FAIL, SHOULD NOT HAPPEN
    {
        Logger::Error("Object in entity was replaced suddenly. ");
    }
    
    // Do we need updates??? 
}
    
void CullingSystem::AddEntity(SceneNode * entity)
{
    
}

void CullingSystem::RemoveEntity(SceneNode * entity)
{
    
}
    
void CullingSystem::SetCamera(Camera * _camera)
{
    camera = _camera;
}
    
void CullingSystem::Process()
{
    int32 objectsCulled = 0;
    
    Frustum * frustum = camera->GetFrustum();

    uint32 size = renderObjectArray.size();
    for (uint32 pos = 0; pos < size; ++pos)
    {
        RenderObject * node = renderObjectArray[pos];
        node->AddFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
        //Logger::Debug("Cull Node: %s rc: %d", node->GetFullName().c_str(), node->GetRetainCount());
        //if (!frustum->IsInside(node->GetWorldTransformedBox()))
        {
            //node->RemoveFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
            objectsCulled++;
        }
    }
}
    
    
};