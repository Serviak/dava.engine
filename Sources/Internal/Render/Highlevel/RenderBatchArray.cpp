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
#include "Render/Highlevel/RenderBatchArray.h"
#include "Debug/Stats.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderLayerManager.h"

namespace DAVA
{
    
RenderPassBatchArray::RenderPassBatchArray(RenderSystem * rs)
{
    const RenderLayerManager * manager = rs->GetRenderLayerManager();
    for (RenderLayerID id = 0; id < RENDER_LAYER_ID_COUNT; ++id)
    {
        RenderLayerBatchArray* batchArray = new RenderLayerBatchArray( manager->GetRenderLayer(id)->GetFlags() );
        layerBatchArrays[id] = batchArray;
    }
}
    
RenderPassBatchArray::~RenderPassBatchArray()
{
    for (RenderLayerID id = 0; id < RENDER_LAYER_ID_COUNT; ++id)
    {
        SafeDelete(layerBatchArrays[id]);
    }
}
    
void RenderPassBatchArray::Clear()
{
    for (RenderLayerID id = 0; id < RENDER_LAYER_ID_COUNT; ++id)
    {
        layerBatchArrays[id]->Clear();
    }
}

void RenderPassBatchArray::PrepareVisibilityArray(VisibilityArray * visibilityArray)
{
    cameraWorldMatrices.clear();
    uint32 size = (uint32)visibilityArray->visibilityArray.size();
    for (uint32 ro = 0; ro < size; ++ro)
    {
        RenderObject * renderObject = visibilityArray->visibilityArray[ro];
        //cameraWorldMatrices[ro] = camera->GetTransform() * (*renderObject->GetWorldTransformPtr());
        
        uint32 batchCount = renderObject->GetRenderBatchCount();
		for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
		{
			RenderBatch * batch = renderObject->GetRenderBatch(batchIndex);
            //batch->SetCameraWorldTransformPtr(&cameraWorldMatrices[ro]);

			NMaterial * material = batch->GetMaterial();
			if (material)
			{
//                const Vector<RenderLayerID> & layers = material->GetRenderLayerIDs();
//                uint32 size = (uint32)layers.size();
//                for (uint32 k = 0; k < size; ++k)
//                {
//                    AddRenderBatch(layers[k], batch);

//                uint32 checkCount = 0;
                uint32 renderLayerBitmask = material->GetRenderLayerIDsBitmask();
                for (uint32 layer = (renderLayerBitmask >> RENDER_LAYER_ID_BITMASK_MIN_POS) & RENDER_LAYER_ID_BITMASK_MIN_MASK,
                     max = (renderLayerBitmask >> RENDER_LAYER_ID_BITMASK_MAX_POS) & RENDER_LAYER_ID_BITMASK_MAX_MASK; layer <= max; ++layer)
                {
                    if (renderLayerBitmask & (1 << layer))
                    {
                        AddRenderBatch(layer, batch);
//                        checkCount++;
                    }
                }
//                DVASSERT(checkCount == size);
			}
		}
    }
}
    
    
RenderLayerBatchArray::RenderLayerBatchArray(uint32 sortingFlags)
    : flags(sortingFlags)
{
    //flags = SORT_ENABLED | SORT_BY_MATERIAL | SORT_BY_DISTANCE;
	//renderBatchArray.reserve(4096);
}
    
RenderLayerBatchArray::~RenderLayerBatchArray()
{
    
}

void RenderLayerBatchArray::Clear()
{
    renderBatchArray.clear();
}
    
bool RenderLayerBatchArray::MaterialCompareFunction(const RenderBatch * a, const RenderBatch *  b)
{
    return a->layerSortingKey > b->layerSortingKey;
}
	
void RenderLayerBatchArray::Sort(Camera * camera)
{
    TIME_PROFILE("RenderLayerBatchArray::Sort");
    // Need sort
	flags |= SORT_REQUIRED;
	
    if ((flags & SORT_THIS_FRAME) == SORT_THIS_FRAME)
    {
        uint32 renderBatchCount = (uint32)renderBatchArray.size();
        if (flags & SORT_BY_MATERIAL)
        {
            //Vector3 cameraPosition = camera->GetPosition();

            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatch * batch = renderBatchArray[k];
				//pointer_size renderObjectId = (pointer_size)batch->GetRenderObject();
                //RenderObject * renderObject = batch->GetRenderObject();
                //Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
                //float32 distance = (position - cameraPosition).Length();
                //uint32 distanceBits = (0xFFFF - ((uint32)distance) & 0xFFFF);
                uint32 materialIndex = batch->GetMaterial()->GetSortingKey();
				//VI: sorting key has the following layout: (m:8)(s:4)(d:20)
                //batch->layerSortingKey = (pointer_size)((materialIndex << 20) | (batch->GetSortingKey() << 28) | (distanceBits));
				batch->layerSortingKey = (pointer_size)(materialIndex | (batch->GetSortingKey() << 28));
				//batch->layerSortingKey = (pointer_size)((batch->GetMaterial()->GetSortingKey() << 20) | (batch->GetSortingKey() << 28) | (renderObjectId & 0x000FFFFF));
            }
            
			std::sort(renderBatchArray.begin(), renderBatchArray.end(), MaterialCompareFunction);
            
            flags &= ~SORT_REQUIRED;
        }
        else if (flags & SORT_BY_DISTANCE_BACK_TO_FRONT)
        {
            Vector3 cameraPosition = camera->GetPosition();
            
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatch * batch = renderBatchArray[k];
                RenderObject * renderObject = batch->GetRenderObject();
                Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
                float32 distance = (position - cameraPosition).Length();
                batch->layerSortingKey = (((uint32)distance) & 0x0fffffff) | (batch->GetSortingKey() << 28);
            }
            
            std::sort(renderBatchArray.begin(), renderBatchArray.end(), MaterialCompareFunction);
            
            flags |= SORT_REQUIRED;
        }else if (flags & SORT_BY_DISTANCE_FRONT_TO_BACK)
        {
            Vector3 cameraPosition = camera->GetPosition();
            
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatch * batch = renderBatchArray[k];
                RenderObject * renderObject = batch->GetRenderObject();
                Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
                float32 distance = (position - cameraPosition).Length();
                uint32 distanceBits = 0x0fffffff - ((uint32)distance) & 0x0fffffff;

                batch->layerSortingKey = distanceBits | (batch->GetSortingKey() << 28);
            }
            
            std::sort(renderBatchArray.begin(), renderBatchArray.end(), MaterialCompareFunction);
            
            flags |= SORT_REQUIRED;
        }
    }
}
    
uint32 RenderLayerBatchArray::GetRenderBatchCount()
{
    return (uint32)renderBatchArray.size();
}

RenderBatch * RenderLayerBatchArray::Get(uint32 index)
{
    return renderBatchArray[index];
}

};
