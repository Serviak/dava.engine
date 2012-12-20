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
#include "Scene3D/Render/RenderSystem.h"
#include "Scene3D/SceneNode.h"
#include "Scene3D/Render/RenderLayer.h"
#include "Scene3D/Render/RenderList.h"
#include "Scene3D/Components/RenderComponent.h"

namespace DAVA
{

RenderSystem::RenderSystem()
    : entityObjectMap(2048 /* size of hash table */, 0 /* defaut value */)
{
}

RenderSystem::~RenderSystem()
{
}
    
void RenderSystem::AddEntity(SceneNode * entity)
{
    RenderObject * renderObject = entity->GetRenderComponent()->GetRenderObject();
    entityObjectMap.Insert(entity, renderObject);
    renderObjectArray.push_back(renderObject);
    renderObject->SetRemoveIndex((uint32)(renderObjectArray.size() - 1));
}

void RenderSystem::RemoveEntity(SceneNode * entity)
{
    RenderObject * renderObject = entityObjectMap.Value(entity);
    renderObjectArray[renderObject->GetRemoveIndex()] = renderObjectArray[renderObjectArray.size() - 1];
    entityObjectMap.Remove(entity);
}

void RenderSystem::AddRenderObject(RenderObject * renderObject)
{
    
}

void RenderSystem::RemoveRenderObject(RenderObject * renderObject)
{
    
}
    
void RenderSystem::Process()
{
    //
    uint32 size = (uint32)renderObjectArray.size();
    for (uint32 k = 0; k < renderObjectArray.size(); ++k)
    {
        renderObjectArray[k]->Draw();
    }
}

    
};