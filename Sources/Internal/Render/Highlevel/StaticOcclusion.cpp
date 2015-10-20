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
#include "Render/RenderHelper.h"
#include "Render/Highlevel/StaticOcclusion.h"
#include "Render/Highlevel/StaticOcclusionRenderPass.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Image/Image.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Image/ImageSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"

namespace DAVA
{
const Vector3 viewDirections[6] =
{
  Vector3(1.0f, 0.0f, 0.0f), //  x 0
  Vector3(0.0f, 1.0f, 0.0f), //  y 1
  Vector3(-1.0f, 0.0f, 0.0f), // -x 2
  Vector3(0.0f, -1.0f, 0.0f), // -y 3
  Vector3(0.0f, 0.0f, 1.0f), // +z 4
  Vector3(0.0f, 0.0f, -1.0f), // -z 5
};

const uint32 effectiveSideCount[6] = { 3, 3, 3, 3, 1, 1 };

const uint32 effectiveSides[6][3] =
{
  { 0, 1, 3 },
  { 1, 0, 2 },
  { 2, 1, 3 },
  { 3, 0, 2 },
  { 4, 4, 4 },
  { 5, 5, 5 },
};

static uint64 blockProcessingTime = 0;

StaticOcclusion::StaticOcclusion()    
{
    staticOcclusionRenderPass = 0;        
    currentData = 0;
    landscape = 0;
    for (uint32 k = 0; k < 6; ++k)
    {
        cameras[k] = new Camera();
        cameras[k]->SetupPerspective(95.0f, 1.0f, 1.0f, 2500.0f); //aspect of one is anyway required to avoid side occlusion errors
    }
}
    
StaticOcclusion::~StaticOcclusion()
{
    for (uint32 k = 0; k < 6; ++k)
    {
        SafeRelease(cameras[k]);
    }
    SafeDelete(staticOcclusionRenderPass);        
}
    
    
void StaticOcclusion::StartBuildOcclusion(StaticOcclusionData * _currentData, RenderSystem * _renderSystem, Landscape * _landscape)
{
    staticOcclusionRenderPass = new StaticOcclusionRenderPass(PASS_FORWARD);
        
    currentData = _currentData;
    occlusionAreaRect = currentData->bbox;
    cellHeightOffset = currentData->cellHeightOffset;
    xBlockCount = currentData->sizeX;
    yBlockCount = currentData->sizeY;
    zBlockCount = currentData->sizeZ;
        
    currentFrameX = 0;
    currentFrameY = 0;
    currentFrameZ = 0;            
                
    renderSystem = _renderSystem;
    landscape = _landscape;        
}       
            
    
AABBox3 StaticOcclusion::GetCellBox(uint32 x, uint32 y, uint32 z)
{
    Vector3 size = occlusionAreaRect.GetSize();
        
    size.x /= xBlockCount;
    size.y /= yBlockCount;
    size.z /= zBlockCount;
        
    Vector3 min(occlusionAreaRect.min.x + x * size.x,
                occlusionAreaRect.min.y + y * size.y,
                occlusionAreaRect.min.z + z * size.z);
    if (cellHeightOffset)
    {
        min.z += cellHeightOffset[x+y*xBlockCount];
    }
    AABBox3 blockBBox(min, Vector3(min.x + size.x, min.y + size.y, min.z + size.z));
    return blockBBox;
}
    
bool StaticOcclusion::ProccessBlock()
{
    if (!ProcessRecorderQueries())
        return false;

    if (currentFrameZ >= zBlockCount)
        return true;

    if (renderPassConfigs.empty())
    {
        auto currentTime = SystemTimer::Instance()->GetAbsoluteNano();
        auto dt = currentTime - blockProcessingTime;
        blockProcessingTime = currentTime;

        Logger::Info("Block processing time: %.4llf", (double)dt / 1e+9);

        BuildRenderPassConfigsForBlock();
    }

    if (RenderCurrentBlock())
    {
        currentFrameX++;
        if (currentFrameX >= xBlockCount)
        {
            currentFrameX = 0;
            currentFrameY++;
            if (currentFrameY >= yBlockCount)
            {
                currentFrameY = 0;
                currentFrameZ++;
            }
        }
    }

    return false;
}

uint32 StaticOcclusion::GetCurrentStepsCount()
{
    return currentFrameX + (currentFrameY * xBlockCount) + (currentFrameZ * xBlockCount * yBlockCount);
    
}

uint32 StaticOcclusion::GetTotalStepsCount()
{
    return xBlockCount * yBlockCount * zBlockCount;
}

void StaticOcclusion::BuildRenderPassConfigsForBlock()
{
    const uint32 stepCount = 10;

    AABBox3 cellBox = GetCellBox(currentFrameX, currentFrameY, currentFrameZ);
    Vector3 stepSize = cellBox.GetSize();
    stepSize /= float32(stepCount);

    DVASSERT(occlusionFrameResults.size() == 0); // previous results are processed - at least for now

    for (uint32 side = 0; side < 6; ++side)
    {
        Vector3 startPosition, directionX, directionY;
        if (side == 0) // +x
        {
            startPosition = Vector3(cellBox.max.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(0.0f, 1.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }
        else if (side == 2) // -x
        {
            startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(0.0f, 1.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }
        else if (side == 1) // +y
        {
            startPosition = Vector3(cellBox.min.x, cellBox.max.y, cellBox.min.z);
            directionX = Vector3(1.0f, 0.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }
        else if (side == 3) // -y
        {
            startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(1.0f, 0.0f, 0.0f);
            directionY = Vector3(0.0f, 0.0f, 1.0f);
        }
        else if (side == 4) // +z
        {
            startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.max.z);
            directionX = Vector3(1.0f, 0.0f, 0.0f);
            directionY = Vector3(0.0f, 1.0f, 0.0f);
        }
        else if (side == 5) // -z
        {
            startPosition = Vector3(cellBox.min.x, cellBox.min.y, cellBox.min.z);
            directionX = Vector3(1.0f, 0.0f, 0.0f);
            directionY = Vector3(0.0f, 1.0f, 0.0f);
        }

        for (uint32 realSideIndex = 0; realSideIndex < effectiveSideCount[side]; ++realSideIndex)
        {
            for (uint32 stepX = 0; stepX <= stepCount; ++stepX)
            {
                for (uint32 stepY = 0; stepY <= stepCount; ++stepY)
                {
                    Vector3 renderPosition = startPosition + directionX * (float32)stepX * stepSize + directionY * (float32)stepY * stepSize;

                    if (landscape)
                    {
                        Vector3 pointOnLandscape;
                        if (landscape->PlacePoint(renderPosition, pointOnLandscape))
                        {
                            if (renderPosition.z < pointOnLandscape.z)
                                continue;
                        }
                    }

                    RenderPassCameraConfig config;
                    config.side = side;
                    config.position = renderPosition;
                    config.direction = viewDirections[effectiveSides[side][realSideIndex]];
                    if (effectiveSides[side][realSideIndex] == 4 || effectiveSides[side][realSideIndex] == 5)
                    {
                        config.up = Vector3(0.0f, 1.0f, 0.0f);
                        config.left = Vector3(1.0f, 0.0f, 0.0f);
                    }
                    else
                    {
                        config.up = Vector3(0.0f, 0.0f, 1.0f);
                        config.left = Vector3(1.0f, 0.0f, 0.0f);
                    }
                    renderPassConfigs.push_back(config);
                }
            }
        }
    }
}

bool StaticOcclusion::PerformRender(const RenderPassCameraConfig& rpc, uint32 blockIndex)
{
    Camera* camera = cameras[rpc.side];
    camera->SetLeft(rpc.left);
    camera->SetUp(rpc.up);
    camera->SetDirection(rpc.direction);
    camera->SetPosition(rpc.position);

    occlusionFrameResults.emplace_back();
    StaticOcclusionFrameResult& res = occlusionFrameResults.back();
    res.blockIndex = blockIndex;
    staticOcclusionRenderPass->DrawOcclusionFrame(renderSystem, camera, res, *currentData);
    if (res.queryBuffer == rhi::InvalidHandle)
    {
        occlusionFrameResults.pop_back();
        return false;
    }

    return true;
}

bool StaticOcclusion::RenderCurrentBlock()
{
    uint32 blockIndex = currentFrameX + currentFrameY * xBlockCount + currentFrameZ * xBlockCount * yBlockCount;

    // uint64 renderingStart = SystemTimer::Instance()->GetAbsoluteNano();

    uint32 renders = 0;
    uint32 actualRenders = 0;
    uint32 maxRenders = 1 + renderPassConfigs.size() / 4;
    while ((renders < maxRenders) && !renderPassConfigs.empty())
    {
        auto i = renderPassConfigs.begin();
        std::advance(i, rand() % renderPassConfigs.size());
        if (PerformRender(*i, blockIndex))
        {
            ++actualRenders;
        }
        renderPassConfigs.erase(i);
        ++renders;
    }
    /*
	uint64 timeTotalRendering = SystemTimer::Instance()->GetAbsoluteNano() - renderingStart;
    Logger::FrameworkDebug(Format("Block: %d, dt: %0.3llf, rendered: %u/%u, remaining: %u", 
		blockIndex, (double)timeTotalRendering / 1e+9, actualRenders, renders, (uint32_t)renderPassConfigs.size()).c_str());
	*/

    if (actualRenders == 0)
    {
        renderPassConfigs.clear();
    }

    return renderPassConfigs.empty();
}

bool StaticOcclusion::ProcessRecorderQueries()
{
    auto fr = occlusionFrameResults.begin();

    while (fr != occlusionFrameResults.end())
    {
        uint32 processedRequests = 0;
        uint32 index = 0;
        for (auto& req : fr->frameRequests)
        {
            if (req == nullptr)
            {
                ++processedRequests;
                ++index;
                continue;
            }

            if (rhi::QueryIsReady(fr->queryBuffer, index))
            {
                auto qv = rhi::QueryValue(fr->queryBuffer, index);
                if (qv != 0)
                {
                    if (!currentData->IsObjectVisibleFromBlock(fr->blockIndex, req->GetStaticOcclusionIndex()))
                    {
                        Logger::Info("Object: %u is visible from block %u (query value: %d)",
                                     uint32(req->GetStaticOcclusionIndex()), fr->blockIndex, qv);
                    }
                    currentData->EnableVisibilityForObject(fr->blockIndex, req->GetStaticOcclusionIndex());
                }
                ++processedRequests;
                req = nullptr;
            }

            ++index;
        }

        if (processedRequests == static_cast<uint32>(fr->frameRequests.size()))
        {
            rhi::DeleteQueryBuffer(fr->queryBuffer, true);
            fr = occlusionFrameResults.erase(fr);
        }
        else
        {
            ++fr;
        }
    }

    return occlusionFrameResults.empty();
}
   
StaticOcclusionData::StaticOcclusionData()
: sizeX(5)
, sizeY(5)
, sizeZ(2)
, blockCount(0)
, objectCount(0)
, data(0)
, cellHeightOffset(0)
{
        
}
    
StaticOcclusionData::~StaticOcclusionData()
{
    SafeDeleteArray(data);
    SafeDeleteArray(cellHeightOffset);
}
    
StaticOcclusionData & StaticOcclusionData::operator= (const StaticOcclusionData & other)
{
    sizeX = other.sizeX;
    sizeY = other.sizeY;
    sizeZ = other.sizeZ;
    objectCount = other.objectCount;
    blockCount = other.blockCount;
    bbox = other.bbox;
        
    SafeDeleteArray(data);
    data = new uint32[(blockCount * objectCount / 32)];
    memcpy(data, other.data, (blockCount * objectCount / 32) * 4);
        
    SafeDeleteArray(cellHeightOffset);
    if (other.cellHeightOffset)
    {
        cellHeightOffset = new float32[sizeX*sizeY];
        memcpy(cellHeightOffset, other.cellHeightOffset, sizeof(float32)*sizeX*sizeY);
    }

    return *this;
}

void StaticOcclusionData::Init(uint32 _sizeX, uint32 _sizeY, uint32 _sizeZ, uint32 _objectCount, const AABBox3 & _bbox, const float32 *_cellHeightOffset)
{
    //DVASSERT(data == 0);
    SafeDeleteArray(data);
    SafeDeleteArray(cellHeightOffset);    
        
    objectCount = _objectCount;
    sizeX = _sizeX;
    sizeY = _sizeY;
    sizeZ = _sizeZ;
    blockCount = sizeX * sizeY * sizeZ;
    bbox = _bbox;
        
    objectCount += (32 - objectCount & 31);
        
    data = new uint32[(blockCount * objectCount / 32)];
    memset(data, 0, (blockCount * objectCount / 32) * 4);
        
    if (_cellHeightOffset)
    {
        cellHeightOffset = new float32[sizeX*sizeY];
        memcpy(cellHeightOffset, _cellHeightOffset, sizeof(float32)*sizeX*sizeY);
    }
}

bool StaticOcclusionData::IsObjectVisibleFromBlock(uint32 blockIndex, uint32 objectIndex) const
{
    auto objIndex = 1 << (objectIndex & 31);
    return (data[(blockIndex * objectCount / 32) + (objectIndex / 32)] & objIndex) != 0;
}

void StaticOcclusionData::EnableVisibilityForObject(uint32 blockIndex, uint32 objectIndex)
{
    data[(blockIndex * objectCount / 32) + (objectIndex / 32)] |= 1 << (objectIndex & 31);
}
    
void StaticOcclusionData::DisableVisibilityForObject(uint32 blockIndex, uint32 objectIndex)
{
    data[(blockIndex * objectCount / 32) + (objectIndex / 32)] &= ~(1 << (objectIndex & 31));
}
    
    
uint32 * StaticOcclusionData::GetBlockVisibilityData(uint32 blockIndex)
{
    return &data[(blockIndex * objectCount / 32)];
}
    
};