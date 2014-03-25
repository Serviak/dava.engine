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
#include "Render/RenderManager.h"
#include "Render/OcclusionQuery.h"
#include "Render/Highlevel/StaticOcclusionRenderPass.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/StaticOcclusion.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
    
StaticOcclusionRenderLayer::StaticOcclusionRenderLayer(const FastName & name, uint32 sortingFlags, StaticOcclusion * _occlusion, RenderLayerID id)
    : RenderLayer(name, sortingFlags, id)
    , occlusion(_occlusion)
{
    
}
StaticOcclusionRenderLayer::~StaticOcclusionRenderLayer()
{
    
}

void StaticOcclusionRenderLayer::Draw(const FastName & ownerRenderPass, Camera * camera, RenderLayerBatchArray * renderLayerBatchArray)
{
    renderLayerBatchArray->Sort(camera);
    uint32 size = (uint32)renderLayerBatchArray->GetRenderBatchCount();
    
    OcclusionQueryManager & manager = occlusion->GetOcclusionQueryManager();
    
    for (uint32 k = 0; k < size; ++k)
    {
        OcclusionQueryManagerHandle handle = manager.CreateQueryObject();
        OcclusionQuery & query = manager.Get(handle);
        
        RenderBatch * batch = renderLayerBatchArray->Get(k);
        
        query.BeginQuery();
        batch->Draw(ownerRenderPass, camera);
        query.EndQuery();
        
        occlusion->RecordFrameQuery(batch, handle);
    }
    //Logger::FrameworkDebug(Format("Pass: %s Layer: %s - objects: %d", ownerRenderPass.c_str(), name.c_str(), size));
}

StaticOcclusionRenderPass::StaticOcclusionRenderPass(const FastName & name, StaticOcclusion * _occlusion, RenderPassID id)
    : RenderPass(name, id)
    , occlusion(_occlusion)
{
    SafeDelete(renderPassBatchArray);
    renderPassBatchArray = new RenderPassBatchArray();
    RenderLayerBatchArray * singleLayer = new RenderLayerBatchArray(RenderLayerBatchArray::SORT_ENABLED | RenderLayerBatchArray::SORT_BY_DISTANCE_FRONT_TO_BACK);
    renderPassBatchArray->InitPassLayersWithSingleLayer(this, singleLayer);
}
    
StaticOcclusionRenderPass::~StaticOcclusionRenderPass()
{
    
}




    
bool StaticOcclusionRenderPass::CompareFunction(const RenderBatch * a, const RenderBatch *  b)
{
    return a->layerSortingKey < b->layerSortingKey;
}
    
void StaticOcclusionRenderPass::Draw(Camera * camera, RenderSystem * renderSystem)
{
    PrepareVisibilityArrays(camera, renderSystem);
	
    Vector<RenderBatch*> terrainBatches;
    Vector<RenderBatch*> batches;
    
    terrainBatches.clear();
    batches.clear();
    
    uint32 size = (uint32)renderLayers.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderLayer * layer = renderLayers[k];
        RenderLayerBatchArray * renderLayerBatchArray = renderPassBatchArray->Get(layer->GetRenderLayerID());
    
        uint32 batchCount = (uint32)renderLayerBatchArray->GetRenderBatchCount();
        for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
        {
            RenderBatch * batch = renderLayerBatchArray->Get(batchIndex);
            if (batch->GetRenderObject()->GetType() == RenderObject::TYPE_LANDSCAPE)
            {
                terrainBatches.push_back(batch);
            }else
            {
                batches.push_back(batch);
            }
        }
    }

    OcclusionQueryManager & manager = occlusion->GetOcclusionQueryManager();
    size = (uint32)terrainBatches.size();
    for (uint32 k = 0; k < size; ++k)
    {
        OcclusionQueryManagerHandle handle = manager.CreateQueryObject();
        OcclusionQuery & query = manager.Get(handle);
        
        RenderBatch * batch = terrainBatches[k];
        
        query.BeginQuery();
        batch->Draw(name, camera);
        query.EndQuery();
        
        occlusion->RecordFrameQuery(batch, handle);
    }
    
    
    Vector3 cameraPosition = camera->GetPosition();

    size = (uint32)batches.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch * batch = batches[k];
        RenderObject * renderObject = batch->GetRenderObject();
        Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
        float realDistance = (position - cameraPosition).Length();
        uint32 distance = ((uint32)(realDistance * 100.0f));
        uint32 distanceBits = distance;
        
        batch->layerSortingKey = distanceBits;
    }

    std::sort(batches.begin(), batches.end(), CompareFunction);
    
    for (uint32 k = 0; k < size; ++k)
    {
        OcclusionQueryManagerHandle handle = manager.CreateQueryObject();
        OcclusionQuery & query = manager.Get(handle);
        
        RenderBatch * batch = batches[k];
        
        query.BeginQuery();
        batch->Draw(name, camera);
        query.EndQuery();
        
        occlusion->RecordFrameQuery(batch, handle);
    }
}



};
