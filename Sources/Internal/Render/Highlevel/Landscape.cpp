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


#include "Render/Highlevel/Landscape.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "Render/RenderHelper.h"
#include "Render/Texture.h"
#include "Scene3D/Scene.h"
#include "Render/Shader.h"
#include "Platform/SystemTimer.h"
#include "Utils/StringFormat.h"
#include "Scene3D/SceneFileV2.h"
#include "Render/Highlevel/Heightmap.h"
#include "FileSystem/FileSystem.h"
#include "Render/TextureDescriptor.h"
#include "Debug/Stats.h"
#include "Render/Material/NMaterial.h"
#include "Scene3D/Systems/MaterialSystem.h"
#include "Scene3D/Systems/FoliageSystem.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Renderer.h"
#include "Render/Highlevel/RenderPassNames.h"

namespace DAVA
{
const FastName Landscape::PARAM_TEXTURE_TILING("textureTiling");
const FastName Landscape::PARAM_TILE_COLOR0("tileColor0");
const FastName Landscape::PARAM_TILE_COLOR1("tileColor1");
const FastName Landscape::PARAM_TILE_COLOR2("tileColor2");
const FastName Landscape::PARAM_TILE_COLOR3("tileColor3");

const FastName Landscape::TEXTURE_COLOR("colorTexture");
const FastName Landscape::TEXTURE_TILE("tileTexture0");
const FastName Landscape::TEXTURE_TILEMASK("tileMask");
const FastName Landscape::TEXTURE_SPECULAR("specularMap");

const uint32 LANDSCAPE_BATCHES_POOL_SIZE = 32;

Landscape::Landscape()
    : indices(0)
    , landscapeMaterial(NULL)
    , foliageSystem(NULL)
    , vertexLayoutUID(rhi::VertexLayout::InvalidUID)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    drawIndices = 0;
    
    type = TYPE_LANDSCAPE;
    
    frustum = 0;
    
    nearLodIndex = 0;
    farLodIndex = 0;
    
    heightmap = new Heightmap;
    prevLodLayer = -1;

    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);

    rhi::VertexLayout vLayout;
    vLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    vLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
#ifdef LANDSCAPE_SPECULAR_LIT
    vLayout.AddElement(rhi::VS_NORMAL, 0, rhi::VDT_FLOAT, 3);
    vLayout.AddElement(rhi::VS_TANGENT, 0, rhi::VDT_FLOAT, 3);
#endif
    vertexLayoutUID = rhi::VertexLayout::UniqueId(vLayout);
}

Landscape::~Landscape()
{
    ReleaseGeometryData();

    SafeRelease(heightmap);
		
	SafeRelease(landscapeMaterial);
}
    

int16 Landscape::AllocateQuadVertexBuffer(LandscapeQuad * quad)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(quad->size == RENDER_QUAD_WIDTH - 1);
    uint32 verticesCount = (quad->size + 1) * (quad->size + 1);
    LandscapeVertex * landscapeVertices = new LandscapeVertex[verticesCount];
    
    int32 index = 0;
    for (int32 y = quad->y; y < quad->y + quad->size + 1; ++y)
    {
        for (int32 x = quad->x; x < quad->x + quad->size + 1; ++x)
        {
            landscapeVertices[index].position = GetPoint(x, y, heightmap->Data()[y * heightmap->Size() + x]);
            Vector2 texCoord = Vector2((float32)(x) / (float32)(heightmap->Size() - 1), 1.0f - (float32)(y) / (float32)(heightmap->Size() - 1));

            landscapeVertices[index].texCoord = texCoord;

#ifdef LANDSCAPE_SPECULAR_LIT
            //VI: calculate normal for the point.
            uint32 xx = 0;
            uint32 yy = 0;

            xx = (x < heightmap->Size() - 1) ? x + 1 : x;
            Vector3 right = GetPoint(xx, y, heightmap->Data()[y * heightmap->Size() + xx]);

            xx = (x > 0) ? x - 1 : x;
            Vector3 left = GetPoint(xx, y, heightmap->Data()[y * heightmap->Size() + xx]);

            yy = (y < heightmap->Size() - 1) ? y + 1 : y;
            Vector3 bottom = GetPoint(x, yy, heightmap->Data()[yy * heightmap->Size() + x]);
            yy = (y > 0) ? y - 1 : y;
            Vector3 top = GetPoint(x, yy, heightmap->Data()[yy * heightmap->Size() + x]);

            Vector3 position = landscapeVertices[index].position;
            Vector3 normal0 = (top != position && right != position) ? CrossProduct(top - position, right - position) : Vector3(0, 0, 0);
            Vector3 normal1 = (right != position && bottom != position) ? CrossProduct(right - position, bottom - position) : Vector3(0, 0, 0);
            Vector3 normal2 = (bottom != position && left != position) ? CrossProduct(bottom - position, left - position) : Vector3(0, 0, 0);
            Vector3 normal3 = (left != position && top != position) ? CrossProduct(left - position, top - position) : Vector3(0, 0, 0);

            Vector3 normalAverage = normal0 + normal1 + normal2 + normal3;
            normalAverage.Normalize();
            landscapeVertices[index].normal = normalAverage;
            landscapeVertices[index].tangent = Normalize(right - position);

            /*
                VS: Algorithm
                // # P.xy store the position for which we want to calculate the normals
                // # height() here is a function that return the height at a point in the terrain

                // read neightbor heights using an arbitrary small offset
                vec3 off = vec3(1.0, 1.0, 0.0);
                float hL = height(P.xy - off.xz);
                float hR = height(P.xy + off.xz);
                float hD = height(P.xy - off.zy);
                float hU = height(P.xy + off.zy);

                // deduce terrain normal
                N.x = hL - hR;
                N.y = hD - hU;
                N.z = 2.0;
                N = normalize(N);
                */

#endif

            index++;
        }
    }

    uint32 vBufferSize = verticesCount * sizeof(LandscapeVertex);
    rhi::HVertexBuffer vertexBuffer = rhi::CreateVertexBuffer(vBufferSize);
    rhi::UpdateVertexBuffer(vertexBuffer, landscapeVertices, 0, vBufferSize);
    vertexBuffers.push_back(vertexBuffer);
    
#if defined(__DAVAENGINE_IPHONE__)
    SafeDeleteArray(landscapeVertices);
#endif

    return (int16)(vertexBuffers.size() - 1);
}

void Landscape::ReleaseGeometryData()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    SafeDeleteArray(indices);

    for (IndexedRenderBatch & batch : renderBatchArray)
        batch.renderBatch->Release();
    renderBatchArray.clear();

    for (rhi::HIndexBuffer handle : indexBuffers.elements)
        rhi::DeleteIndexBuffer(handle);

    for (rhi::HVertexBuffer handle : vertexBuffers)
        rhi::DeleteVertexBuffer(handle);
    vertexBuffers.clear();
}

void Landscape::SetLods(const Vector4 & lods)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    lodLevelsCount = 4;
    
    lodDistance[0] = lods.x;
    lodDistance[1] = lods.y;
    lodDistance[2] = lods.z;
    lodDistance[3] = lods.w;
    
    for (int32 ll = 0; ll < lodLevelsCount; ++ll)
        lodSqDistance[ll] = lodDistance[ll] * lodDistance[ll];
}
    
void Landscape::BuildLandscapeFromHeightmapImage(const FilePath & heightmapPathname, const AABBox3 & _box)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    heightmapPath = heightmapPathname;
    BuildHeightmap();

    bbox = _box;

    BuildLandscape();
    
    if(foliageSystem)
    {
        foliageSystem->SyncFoliageWithLandscape();
    }
}

void Landscape::RecalcBoundingBox()
{
    //do nothing, bbox setup in BuildLandscapeFromHeightmapImage()
}

bool Landscape::BuildHeightmap()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    bool retValue = false;

    if(DAVA::TextureDescriptor::IsSourceTextureExtension(heightmapPath.GetExtension()))
    {
        Vector<Image *> imageSet;
        ImageSystem::Instance()->Load(heightmapPath, imageSet);
        if(0 != imageSet.size())
        {
            if ((imageSet[0]->GetPixelFormat() != FORMAT_A8) && (imageSet[0]->GetPixelFormat() != FORMAT_A16))
            {
                Logger::Error("Image for landscape should be gray scale");
            }
            else
            {
                DVASSERT(imageSet[0]->GetWidth() == imageSet[0]->GetHeight());
                heightmap->BuildFromImage(imageSet[0]);
                retValue = true;
            }
            
            for_each(imageSet.begin(), imageSet.end(), SafeRelease<Image>);
        }
    }
    else if(heightmapPath.IsEqualToExtension(Heightmap::FileExtension()))
    {
        retValue = heightmap->Load(heightmapPath);
    }
	else
	{
		// SZ: don't assert here, and it will be possible to load landscape in editor and
		// fix wrong path to heightmap
		// 
		//DVASSERT(false && "wrong extension");
        heightmap->ReleaseData();
	}

    return retValue;
}

void Landscape::AllocateGeometryData()
{
    DVASSERT(vertexLayoutUID != rhi::VertexLayout::InvalidUID);

    if (!landscapeMaterial)
    {
        landscapeMaterial = new NMaterial();
        landscapeMaterial->SetFXName(NMaterialName::TILE_MASK);
    }

    for (int32 i = 0; i < LANDSCAPE_BATCHES_POOL_SIZE; i++)
    {
        ScopedPtr<RenderBatch> batch(new RenderBatch());
        AddRenderBatch(batch);

        batch->SetMaterial(landscapeMaterial);
        batch->SetSortingKey(10);

        batch->vertexLayoutId = vertexLayoutUID;
        batch->vertexCount = RENDER_QUAD_WIDTH * RENDER_QUAD_WIDTH;
    }

    for (auto & handle : indexBuffers.elements)
        handle = rhi::CreateIndexBuffer(INDEX_ARRAY_COUNT * 2);

    currentIndexBuffer = indexBuffers.Next();

    indices = new uint16[INDEX_ARRAY_COUNT];
}

void Landscape::BuildLandscape()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    ReleaseGeometryData();
    AllocateGeometryData();


    quadTreeHead.data.x = quadTreeHead.data.y = quadTreeHead.data.lod = 0;
    //quadTreeHead.data.xbuf = quadTreeHead.data.ybuf = 0;
    quadTreeHead.data.rdoQuad = -1;
    
    SetLods(Vector4(60.0f, 120.0f, 240.0f, 480.0f));
 
    allocatedMemoryForQuads = 0;

    if(heightmap->Size())
    {
        quadTreeHead.data.size = heightmap->Size() - 1;
        
        RecursiveBuild(&quadTreeHead, 0, lodLevelsCount);
        FindNeighbours(&quadTreeHead);
    }
    else
    {
        quadTreeHead.ReleaseChildren();
        quadTreeHead.data.size = 0;
    }
}
    

Vector3 Landscape::GetPoint(int16 x, int16 y, uint16 height) const
{
    Vector3 res;
    res.x = (bbox.min.x + (float32)x / (float32)(heightmap->Size() - 1) * (bbox.max.x - bbox.min.x));
    res.y = (bbox.min.y + (float32)y / (float32)(heightmap->Size() - 1) * (bbox.max.y - bbox.min.y));
    res.z = (bbox.min.z + ((float32)height / (float32)Heightmap::MAX_VALUE) * (bbox.max.z - bbox.min.z));
    return res;
};

bool Landscape::PlacePoint(const Vector3 & point, Vector3 & result, Vector3 * normal) const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	if (point.x > bbox.max.x ||
		point.x < bbox.min.x ||
		point.y > bbox.max.y ||
		point.y < bbox.min.y)
	{
		return false;
	}
	
	if(heightmap->Data() == NULL)
	{
		Logger::Error("[Landscape::PlacePoint] Trying to place point on empty heightmap data!");
		return false;
	}
	
	float32 kW = (float32)(heightmap->Size() - 1) / (bbox.max.x - bbox.min.x);
	
	float32 x = (point.x - bbox.min.x) * kW;
	float32 y = (point.y - bbox.min.y) * kW;

	float32 x1 = floor(x);
	float32 y1 = floor(y);

	float32 x2 = ceil(x);
	float32 y2 = ceil(y);

	if (x1 == x2)
		x2 += 1.0f;

	if (y1 == y2)
		y2 += 1.0f;

	uint16 * data = heightmap->Data();
	int32 imW = heightmap->Size();

	Vector3 p1(x1, y1, 0);
	p1.z = data[(int32)p1.y * imW + (int32)p1.x];

	Vector3 p2(x2, y2, 0);
	p2.z = data[(int32)p2.y * imW + (int32)p2.x];

	Vector3 p3;
	if (x - x1 >= y - y1)
		p3 = Vector3(x2, y1, 0);
	else
		p3 = Vector3(x1, y2, 0);
	p3.z = data[(int32)p3.y * imW + (int32)p3.x];

	//http://algolist.manual.ru/maths/geom/equation/plane.php
	float32 A = p1.y * (p2.z - p3.z) + p2.y * (p3.z - p1.z) + p3.y * (p1.z - p2.z); 
	float32 B = p1.z * (p2.x - p3.x) + p2.z * (p3.x - p1.x) + p3.z * (p1.x - p2.x);
	float32 C = p1.x * (p2.y - p3.y) + p2.x * (p3.y - p1.y) + p3.x * (p1.y - p2.y);
	float32 D = p1.x * (p2.y * p3.z - p3.y * p2.z) + p2.x * (p3.y * p1.z - p1.y * p3.z) + p3.x * (p1.y * p2.z - p2.y * p1.z);

	result.x = point.x;
	result.y = point.y;

	result.z = (D - B * y - A * x) / C;
	result.z = bbox.min.z + result.z / ((float32)Heightmap::MAX_VALUE) * (bbox.max.z - bbox.min.z);

	if (normal != 0)
	{
		normal->x = A;
		normal->y = B;
		normal->z = C;
		normal->Normalize();
	}
	return true;
};

void Landscape::RecursiveBuild(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 level, int32 maxLevels)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    allocatedMemoryForQuads += sizeof(LandQuadTreeNode<LandscapeQuad>);
    currentNode->data.lod = level;
    
    // if we have parrent get rdo quad 
    if (currentNode->parent)
    {
        currentNode->data.rdoQuad = currentNode->parent->data.rdoQuad;
    }
    
    if ((currentNode->data.rdoQuad == -1) && (currentNode->data.size == RENDER_QUAD_WIDTH - 1))
    {
        currentNode->data.rdoQuad = AllocateQuadVertexBuffer(&currentNode->data);
        //currentNode->data.xbuf = 0;
        //currentNode->data.ybuf = 0;
    }
    
    // 
    // Check if we can build tree with less number of nodes
    // I think we should stop much earlier to perform everything faster
    //
    
    if (currentNode->data.size == 2)
    {
        // compute node bounding box
        uint16 * data = heightmap->Data();
        for (int16 x = currentNode->data.x; x <= currentNode->data.x + currentNode->data.size; ++x)
            for (int16 y = currentNode->data.y; y <= currentNode->data.y + currentNode->data.size; ++y)
            {
                uint16 value = data[heightmap->Size() * y + x];
                Vector3 pos = GetPoint(x, y, value);
                
                currentNode->data.bbox.AddPoint(pos);
            }
        return;
    }
    
    // alloc and process childs
    currentNode->AllocChildren();
    
    int16 minIndexX = currentNode->data.x;
    int16 minIndexY = currentNode->data.y;
    
    //int16 bufMinIndexX = currentNode->data.xbuf;
    //int16 bufMinIndexY = currentNode->data.ybuf;
    
    int16 size = currentNode->data.size;
    
    // We should be able to divide landscape by 2 here
    DVASSERT((size & 1) == 0);

    LandQuadTreeNode<LandscapeQuad> * child0 = &currentNode->children[0];
    child0->data.x = minIndexX;
    child0->data.y = minIndexY;
    //child0->data.xbuf = bufMinIndexX;
    //child0->data.ybuf = bufMinIndexY;
    child0->data.size = size / 2;
    
    LandQuadTreeNode<LandscapeQuad> * child1 = &currentNode->children[1];
    child1->data.x = minIndexX + size / 2;
    child1->data.y = minIndexY;
    //child1->data.xbuf = bufMinIndexX + size / 2;
    //child1->data.ybuf = bufMinIndexY;
    child1->data.size = size / 2;

    LandQuadTreeNode<LandscapeQuad> * child2 = &currentNode->children[2];
    child2->data.x = minIndexX;
    child2->data.y = minIndexY + size / 2;
    //child2->data.xbuf = bufMinIndexX;
    //child2->data.ybuf = bufMinIndexY + size / 2;
    child2->data.size = size / 2;

    LandQuadTreeNode<LandscapeQuad> * child3 = &currentNode->children[3];
    child3->data.x = minIndexX + size / 2;
    child3->data.y = minIndexY + size / 2;
    //child3->data.xbuf = bufMinIndexX + size / 2;
    //child3->data.ybuf = bufMinIndexY + size / 2;
    child3->data.size = size / 2;
    
    for (int32 index = 0; index < 4; ++index)
    {
        LandQuadTreeNode<LandscapeQuad> * child = &currentNode->children[index];
        child->parent = currentNode;
        RecursiveBuild(child, level + 1, maxLevels);
        
        currentNode->data.bbox.AddPoint(child->data.bbox.min);
        currentNode->data.bbox.AddPoint(child->data.bbox.max);
    }


}
/*
    Neighbours looks up
    *********
    *0*1*0*1*
    **0***1**
    *2*3*2*3*
    ****0****
    *0*1*0*1*
    **2***3**
    *2*3*2*3*
    *********
    *0*1*0*1*
    **0***1**
    *2*3*2*3*
    ****2****
    *0*1*0*1*
    **2***3**
    *2*3*2*3*
    *********
 */

LandQuadTreeNode<Landscape::LandscapeQuad> * Landscape::FindNodeWithXY(LandQuadTreeNode<LandscapeQuad> * currentNode, int16 quadX, int16 quadY, int16 quadSize)
{
    if ((currentNode->data.x <= quadX) && (quadX < currentNode->data.x + currentNode->data.size))
        if ((currentNode->data.y <= quadY) && (quadY < currentNode->data.y + currentNode->data.size))
    {
        if (currentNode->data.size == quadSize)return currentNode;
        if (currentNode->children)
        {
            for (int32 index = 0; index < 4; ++index)
            {
                LandQuadTreeNode<LandscapeQuad> * child = &currentNode->children[index];
                LandQuadTreeNode<LandscapeQuad> * result = FindNodeWithXY(child, quadX, quadY, quadSize);
                if (result)
                    return result;
            } 
        }
    }
    
    return 0;
}
    
void Landscape::FindNeighbours(LandQuadTreeNode<LandscapeQuad> * currentNode)
{
    currentNode->neighbours[LEFT] = FindNodeWithXY(&quadTreeHead, currentNode->data.x - 1, currentNode->data.y, currentNode->data.size);
    currentNode->neighbours[RIGHT] = FindNodeWithXY(&quadTreeHead, currentNode->data.x + currentNode->data.size, currentNode->data.y, currentNode->data.size);
    currentNode->neighbours[TOP] = FindNodeWithXY(&quadTreeHead, currentNode->data.x, currentNode->data.y - 1, currentNode->data.size);
    currentNode->neighbours[BOTTOM] = FindNodeWithXY(&quadTreeHead, currentNode->data.x, currentNode->data.y + currentNode->data.size, currentNode->data.size);
    
    if (currentNode->children)
    {
        for (int32 index = 0; index < 4; ++index)
        {
            LandQuadTreeNode<LandscapeQuad> * child = &currentNode->children[index];
            FindNeighbours(child);
        }
    }
}

void Landscape::MarkFrames(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 & depth)
{
    if (--depth <= 0)
    {
        currentNode->data.frame = Core::Instance()->GetGlobalFrameIndex();
        depth++;
        return;
    }
    if (currentNode->children)
    {
        for (int32 index = 0; index < 4; ++index)
        {
            LandQuadTreeNode<LandscapeQuad> * child = &currentNode->children[index];
            MarkFrames(child, depth);
        }
    }
    depth++;
}
    
void Landscape::FlushQueue()
{
    if ((queueIndexCount - queueIndexOffset) == 0) return;


    DVASSERT(queueIndexCount < INDEX_ARRAY_COUNT);
    DVASSERT(flushQueueCounter < (int32)renderBatchArray.size());
    DVASSERT(queueRdoQuad != -1);


    RenderBatch * batch = renderBatchArray[flushQueueCounter].renderBatch;
    activeRenderBatchArray.push_back(batch);

    batch->indexBuffer = currentIndexBuffer;
    batch->indexCount = queueIndexCount - queueIndexOffset;
    batch->startIndex = queueIndexOffset;
    batch->vertexBuffer = vertexBuffers[queueRdoQuad];

    queueIndexOffset = queueIndexCount;
    queueRdoQuad = -1;

    drawIndices += batch->indexCount;
    ++flushQueueCounter;
}
    
void Landscape::ClearQueue()
{
    queueIndexCount = 0;
    queueIndexOffset = 0;
    queueRdoQuad = -1;
    queueDrawIndices = indices;
}

void Landscape::GenQuad(LandQuadTreeNode<LandscapeQuad> * currentNode, int8 lod)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    int32 depth = currentNode->data.size / (1 << lod);
    if (depth == 1)
    {
        currentNode->parent->data.frame = Core::Instance()->GetGlobalFrameIndex();
    }else
    {
        //int32 newdepth = (int)(logf((float)depth) / logf(2.0f) + 0.5f);
        int32 newdepth2 = FastLog2(depth);
        //Logger::FrameworkDebug("dp: %d %d %d", depth, newdepth, newdepth2);
        //DVASSERT(newdepth == newdepth2); // Check of math, we should use optimized version with depth2
        
        MarkFrames(currentNode, newdepth2);
    }
    
    int32 step = Min((int16)(1 << lod), currentNode->data.size);
    
    if ((currentNode->data.rdoQuad != queueRdoQuad) && (queueRdoQuad != -1))
    {
        FlushQueue();
    }
    
    queueRdoQuad = currentNode->data.rdoQuad;
    
    for (uint16 y = (currentNode->data.y & RENDER_QUAD_AND); y < (currentNode->data.y & RENDER_QUAD_AND) + currentNode->data.size; y += step)
        for (uint16 x = (currentNode->data.x & RENDER_QUAD_AND); x < (currentNode->data.x & RENDER_QUAD_AND) + currentNode->data.size; x += step)
        {
            *queueDrawIndices++ = x + y * RENDER_QUAD_WIDTH;
            *queueDrawIndices++ = (x + step) + y * RENDER_QUAD_WIDTH;
            *queueDrawIndices++ = x + (y + step) * RENDER_QUAD_WIDTH;
            
            *queueDrawIndices++ = (x + step) + y * RENDER_QUAD_WIDTH;
            *queueDrawIndices++ = (x + step) + (y + step) * RENDER_QUAD_WIDTH;
            *queueDrawIndices++ = x + (y + step) * RENDER_QUAD_WIDTH;     
 
            queueIndexCount += 6;
        }
}
    
void Landscape::GenFans()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    uint32 currentFrame = Core::Instance()->GetGlobalFrameIndex();;
    int16 width = RENDER_QUAD_WIDTH;//heightmap->GetWidth();

    queueRdoQuad = -1;
    
    Vector<LandQuadTreeNode<LandscapeQuad>*>::const_iterator end = fans.end();
    for (Vector<LandQuadTreeNode<LandscapeQuad>*>::iterator t = fans.begin(); t != end; ++t)
    {
        LandQuadTreeNode<LandscapeQuad>* node = *t;
        
        if ((node->data.rdoQuad != queueRdoQuad) && (queueRdoQuad != -1))
        {
            FlushQueue();
        }
        queueRdoQuad = node->data.rdoQuad;
        
        int16 halfSize = (node->data.size >> 1);
        int16 xbuf = node->data.x & RENDER_QUAD_AND;
        int16 ybuf = node->data.y & RENDER_QUAD_AND;


#define ADD_VERTEX(index) {*queueDrawIndices++ = (index); queueIndexCount++;}
        
        ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
        ADD_VERTEX((xbuf) + (ybuf) * width);
        
        if ((node->neighbours[TOP]) && (node->neighbours[TOP]->data.frame == currentFrame))
        {
            ADD_VERTEX((xbuf + halfSize) + (ybuf) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf) * width);
        }
        
        ADD_VERTEX((xbuf + node->data.size) + (ybuf) * width);
        ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
        ADD_VERTEX((xbuf + node->data.size) + (ybuf) * width);

        
        if ((node->neighbours[RIGHT]) && (node->neighbours[RIGHT]->data.frame == currentFrame))
        {
            ADD_VERTEX((xbuf + node->data.size) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + node->data.size) + (ybuf + halfSize) * width);
        }

        ADD_VERTEX((xbuf + node->data.size) + (ybuf + node->data.size) * width);
        ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
        ADD_VERTEX((xbuf + node->data.size) + (ybuf + node->data.size) * width);
        
        if ((node->neighbours[BOTTOM]) && (node->neighbours[BOTTOM]->data.frame == currentFrame))
        {
            ADD_VERTEX((xbuf + halfSize) + (ybuf + node->data.size) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + node->data.size) * width);
        }
        
        ADD_VERTEX((xbuf) + (ybuf + node->data.size) * width);
        ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
        ADD_VERTEX((xbuf) + (ybuf + node->data.size) * width);
        
        if ((node->neighbours[LEFT]) && (node->neighbours[LEFT]->data.frame == currentFrame))
        {
            ADD_VERTEX((xbuf) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf + halfSize) + (ybuf + halfSize) * width);
            ADD_VERTEX((xbuf) + (ybuf + halfSize) * width);
        }

        ADD_VERTEX((xbuf) + (ybuf) * width);
        
#undef ADD_VERTEX
    }
    
    FlushQueue();
}
	
void Landscape::GenLods(LandQuadTreeNode<LandscapeQuad> * currentNode, uint8 clippingFlags, Camera * camera)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    //Frustum * frustum = scene->GetClipCamera()->GetFrustum();
    // if (!frustum->IsInside(currentNode->data.bbox))return;
    Frustum::eFrustumResult frustumRes = Frustum::EFR_INSIDE; 
    
    if (currentNode->data.size >= 2)
		if (clippingFlags)
			frustumRes = frustum->Classify(currentNode->data.bbox, clippingFlags, currentNode->data.startClipPlane);		
    
    if (frustumRes == Frustum::EFR_OUTSIDE)return;
    
    /*
        If current quad do not have geometry just traverse it childs. 
        Magic starts when we have a geometry
     */
    if (currentNode->data.rdoQuad == -1)
    {
        if (currentNode->children)
        {
            for (int32 index = 0; index < 4; ++index)
            {
                LandQuadTreeNode<LandscapeQuad> * child = &currentNode->children[index];
                GenLods(child, clippingFlags, camera);
            }
        }
        return;
    }
    /*
        // UNCOMMENT THIS TO CHECK GEOMETRY WITH 0 LEVEL OF DETAIL
        else
        {
            DrawQuad(currentNode, 0);
            return;
        }
     */
    
    /*
        We can be here only if we have a geometry in the node. 
        Here we use Geomipmaps rendering algorithm. 
        These quads are 129x129.
     */
//    Camera * cam = clipCamera;
    
    Vector3 corners[8];
    currentNode->data.bbox.GetCorners(corners);
    
    float32 minDist =  100000000.0f;
    float32 maxDist = -100000000.0f;
    for (int32 k = 0; k < 8; ++k)
    {
        Vector3 v = camera->GetPosition() - corners[k];
        float32 dist = v.SquareLength();
        if (dist < minDist)
            minDist = dist;
        if (dist > maxDist)
            maxDist = dist;
    };
    
    int32 minLod = 0;
    int32 maxLod = 0;
    
    for (int32 k = 0; k < lodLevelsCount; ++k)
    {
        if (minDist > lodSqDistance[k])
            minLod = k + 1;
        if (maxDist > lodSqDistance[k])
            maxLod = k + 1;
    }
    
    // debug block
#if 1
    if (currentNode == &quadTreeHead)
    {
        //Logger::FrameworkDebug("== draw start ==");
    }
    //Logger::FrameworkDebug("%f %f %d %d", minDist, maxDist, minLod, maxLod);
#endif

//    if (frustum->IsFullyInside(currentNode->data.bbox))
//    {
//        RenderSystem2D::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
//        RenderHelper::Instance()->DrawBox(currentNode->data.bbox);
//    }
    
    
    if ((minLod == maxLod) && (/*frustum->IsFullyInside(currentNode->data.bbox)*/(frustumRes == Frustum::EFR_INSIDE) || currentNode->data.size <= (1 << maxLod) + 1) )
    {
        //Logger::FrameworkDebug("lod: %d depth: %d pos(%d, %d)", minLod, currentNode->data.lod, currentNode->data.x, currentNode->data.y);
        
//        if (currentNode->data.size <= (1 << maxLod))
//            RenderSystem2D::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
//        if (frustum->IsFullyInside(currentNode->data.bbox))
//            RenderSystem2D::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        //if (frustum->IsFullyInside(currentNode->data.bbox) && (currentNode->data.size <= (1 << maxLod)))
        //    RenderSystem2D::Instance()->SetColor(1.0f, 1.0f, 0.0f, 1.0f);

            
        //RenderSystem2D::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
#if defined (DRAW_OLD_STYLE)        
        GenQuad(currentNode, maxLod);
#else //#if defined (DRAW_OLD_STYLE)        
        currentNode->data.lod = maxLod;
        if(maxLod)
        {
            lodNot0quads.push_back(currentNode);
        }
        else 
        {
            lod0quads.push_back(currentNode);
        }
#endif //#if defined (DRAW_OLD_STYLE)
        
        //RenderSystem2D::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        //RenderHelper::Instance()->DrawBox(currentNode->data.bbox);

        return;
    }

    
    if ((minLod != maxLod) && (currentNode->data.size <= (1 << maxLod) + 1))
    {
//        RenderSystem2D::Instance()->SetColor(0.0f, 0.0f, 1.0f, 1.0f);
        //DrawQuad(currentNode, minLod);
        //RenderSystem2D::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
        //RenderHelper::Instance()->DrawBox(currentNode->data.bbox);
        fans.push_back(currentNode);
        return;
    }
    
    //
    // Check performance and identify do we need a sorting here. 
    // Probably sorting algorithm can be helpfull to render on Mac / Windows, but will be useless for iOS and Android
    //
    
    {
        if (currentNode->children)
        {
            for (int32 index = 0; index < 4; ++index)
            {
                LandQuadTreeNode<LandscapeQuad> * child = &currentNode->children[index];
                GenLods(child, clippingFlags, camera);
            }
        }
        /* EXPERIMENTAL => reduce level of quadtree, results was not successfull 
         else
        {
            DrawQuad(currentNode, maxLod);  
        }*/
    }
}

void Landscape::PrepareToRender(Camera * camera)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    RenderObject::PrepareToRender(camera);

    TIME_PROFILE("LandscapeNode.PrepareToRender");
	
	drawIndices = 0;

	if(!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::LANDSCAPE_DRAW))
	{
		return;
	}

    ClearQueue();

/*
    Boroda: I do not understand why, but this code breaks frustrum culling on landscape.
    I've spent an hour, trying to understand what's going on, without luck. 
 */
    
//	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
//	Matrix4 meshFinalMatrix = worldTransform * prevMatrix;
//    //frustum->Set(meshFinalMatrix * RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_PROJECTION));
//    cameraPos = scene->GetClipCamera()->GetPosition() * worldTransform;
//
//    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);
//    frustum->Set();

    frustum = camera->GetFrustum();    
    
    flushQueueCounter = 0;
    
#if defined (DRAW_OLD_STYLE)    
    BindMaterial(0);
	GenLods(&quadTreeHead);
#else //#if defined (DRAW_OLD_STYLE)   
    
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
	if(Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_LANDSCAPE_LODS))
	{
		fans.clear();
		lod0quads.clear();
		lodNot0quads.clear();
        activeRenderBatchArray.clear();

		GenLods(&quadTreeHead, 0x3f, camera);
	}

    int32 count0 = static_cast<int32>(lod0quads.size());
    for(int32 i = 0; i < count0; ++i)
    {
        GenQuad(lod0quads[i], 0);
    }
	FlushQueue();

    int32 countNot0 = static_cast<int32>(lodNot0quads.size());
    for(int32 i = 0; i < countNot0; ++i)
    {
        GenQuad(lodNot0quads[i], lodNot0quads[i]->data.lod);
    }
#endif //#if defined (DRAW_OLD_STYLE)    
    
	FlushQueue();

	GenFans();

    rhi::UpdateIndexBuffer(currentIndexBuffer, indices, 0, queueIndexCount * sizeof(uint16));
    currentIndexBuffer = indexBuffers.Next();
}


bool Landscape::GetGeometry(Vector<LandscapeVertex> & landscapeVertices, Vector<int32> & indices) const
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    if (heightmap->Data() == nullptr)
    {
        return false;
    }

	const LandQuadTreeNode<LandscapeQuad> * currentNode = &quadTreeHead;
	const LandscapeQuad * quad = &currentNode->data;
	
	landscapeVertices.resize((quad->size + 1) * (quad->size + 1));

	int32 index = 0;
	for (int32 y = quad->y; y < quad->y + quad->size + 1; ++y)
	{
		for (int32 x = quad->x; x < quad->x + quad->size + 1; ++x)
		{
			landscapeVertices[index].position = GetPoint(x, y, heightmap->Data()[y * heightmap->Size() + x]);
			landscapeVertices[index].texCoord = Vector2((float32)x / (float32)(heightmap->Size() - 1), 1.0f - (float32)y / (float32)(heightmap->Size() - 1));
			index++;
		}
	}

	indices.resize(heightmap->Size()*heightmap->Size()*6);
	int32 step = 1;
	int32 indexIndex = 0;
	int32 quadWidth = heightmap->Size();
	for(int32 y = 0; y < currentNode->data.size-1; y += step)
	{
		for(int32 x = 0; x < currentNode->data.size-1; x += step)
		{
			indices[indexIndex++] = x + y * quadWidth;
			indices[indexIndex++] = (x + step) + y * quadWidth;
			indices[indexIndex++] = x + (y + step) * quadWidth;

			indices[indexIndex++] = (x + step) + y * quadWidth;
			indices[indexIndex++] = (x + step) + (y + step) * quadWidth;
			indices[indexIndex++] = x + (y + step) * quadWidth;     
		}
	}

    return true;
}

const FilePath & Landscape::GetHeightmapPathname()
{
    return heightmapPath;
}
	
void Landscape::SetHeightmapPathname(const FilePath & newHeightMapPath)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	if(newHeightMapPath == heightmapPath)
	{
		return;
	}
	BuildLandscapeFromHeightmapImage(newHeightMapPath, bbox);
}
	
float32 Landscape::GetLandscapeSize() const
{
	return bbox.GetSize().x;
}
	
void Landscape::SetLandscapeSize(float32 newSize)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    Vector3 newLandscapeSize(newSize, newSize, bbox.GetSize().z);
    SetLandscapeSize(newLandscapeSize);
}

float32 Landscape::GetLandscapeHeight() const
{
    return bbox.GetSize().z;
}
	
void Landscape::SetLandscapeHeight(float32 newHeight)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	Vector3 newLandscapeSize(bbox.GetSize().x, bbox.GetSize().y, newHeight);
	SetLandscapeSize(newLandscapeSize);
}

void Landscape::SetLandscapeSize(const Vector3 & newLandscapeSize)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    if(newLandscapeSize.z < 0.0f || newLandscapeSize.x <0 || newLandscapeSize.y < 0)
	{
		return;
	}
    if(newLandscapeSize == bbox.GetSize())
	{
		return;
	}
    bbox.Empty();
	bbox.AddPoint(Vector3(-newLandscapeSize.x/2.f, -newLandscapeSize.y/2.f, 0.f));
	bbox.AddPoint(Vector3(newLandscapeSize.x/2.f, newLandscapeSize.y/2.f, newLandscapeSize.z));
    BuildLandscape();
    
    if(foliageSystem)
    {
        foliageSystem->SyncFoliageWithLandscape();
    }
}

void Landscape::GetDataNodes(Set<DataNode*> & dataNodes)
{
    NMaterial* curMaterialNode = landscapeMaterial;
    while (curMaterialNode != NULL)
    {
        dataNodes.insert(curMaterialNode);
        curMaterialNode = curMaterialNode->GetParent();
    }
}
    
void Landscape::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    
    AnimatedObject::SaveObject(archive);
    
    archive->SetUInt32("ro.debugflags", debugFlags);
    archive->SetUInt32("ro.sOclIndex", staticOcclusionIndex);

    //VI: save only VISIBLE flag for now. May be extended in the future
    archive->SetUInt32("ro.flags", flags & RenderObject::SERIALIZATION_CRITERIA);

    uint64 matKey = landscapeMaterial->GetNodeID();
    archive->SetUInt64("matname", matKey);

    //TODO: remove code in future. Need for transition from *.png to *.heightmap
    if(!heightmapPath.IsEqualToExtension(Heightmap::FileExtension()))
    {
        heightmapPath.ReplaceExtension(Heightmap::FileExtension());
    }

    heightmap->Save(heightmapPath);
    archive->SetString("hmap", heightmapPath.GetRelativePathname(serializationContext->GetScenePath()));
    archive->SetByteArrayAsType("bbox", bbox);
}
    
void Landscape::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    debugFlags = archive->GetUInt32("ro.debugflags", 0);
    staticOcclusionIndex = (uint16)archive->GetUInt32("ro.sOclIndex", INVALID_STATIC_OCCLUSION_INDEX);

    //VI: load only VISIBLE flag for now. May be extended in the future.
    uint32 savedFlags = RenderObject::SERIALIZATION_CRITERIA & archive->GetUInt32("ro.flags", RenderObject::SERIALIZATION_CRITERIA);
    flags = (savedFlags | (flags & ~RenderObject::SERIALIZATION_CRITERIA));

    uint64 matKey = archive->GetUInt64("matname");

    if (!matKey) //Load from old landscape format: get material from batch0

    {
        uint32 roBatchCount = archive->GetUInt32("ro.batchCount");
        DVASSERT(roBatchCount);
        KeyedArchive *batchesArch = archive->GetArchive("ro.batches");
        DVASSERT(batchesArch);
        KeyedArchive *batchArch = batchesArch->GetArchive(KeyedArchive::GenKeyFromIndex(0));
        DVASSERT(batchArch);

        matKey = batchArch->GetUInt64("rb.nmatname");
    }

    DVASSERT(matKey);
    landscapeMaterial = SafeRetain(static_cast<NMaterial*>(serializationContext->GetDataBlock(matKey)));
    if (landscapeMaterial)
        landscapeMaterial->PreBuildMaterial(PASS_FORWARD);
	
	FilePath heightmapPath = serializationContext->GetScenePath() + archive->GetString("hmap");
    AABBox3 loadedBbox = archive->GetByteArrayAsType("bbox", AABBox3());

    BuildLandscapeFromHeightmapImage(heightmapPath, loadedBbox);
}

Heightmap * Landscape::GetHeightmap()
{
    return heightmap;
}

void Landscape::SetHeightmap(DAVA::Heightmap *height)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    SafeRelease(heightmap);
    heightmap = SafeRetain(height);
    
    BuildLandscape();
}

NMaterial * Landscape::GetMaterial()
{
    return landscapeMaterial;
}

void Landscape::SetMaterial(NMaterial * material)
{
    SafeRetain(material);
    SafeRelease(landscapeMaterial);
    landscapeMaterial = material;

    for (uint32 i = 0; i < GetRenderBatchCount(); ++i)
        GetRenderBatch(i)->SetMaterial(landscapeMaterial);
}

Texture * Landscape::CreateLandscapeTexture()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

#if RHI_COMPLETE
    //Set indexes
    Vector<float32> ftVertexes;
    Vector<float32> ftTextureCoords;
    
    float32 x0 = 0.f;
    float32 y0 = 0.f;
    float32 x1 = 1.f;
    float32 y1 = 1.f;
    
    //triangle 1
    //0, 0
    ftVertexes.push_back(x0);
    ftVertexes.push_back(y0);
    ftVertexes.push_back(0);
    ftTextureCoords.push_back(0);
    ftTextureCoords.push_back(0);
    
    
    //1, 0
    ftVertexes.push_back(x1);
    ftVertexes.push_back(y0);
    ftVertexes.push_back(0);
    ftTextureCoords.push_back(1);
    ftTextureCoords.push_back(0);
    
    
    //0, 1
    ftVertexes.push_back(x0);
    ftVertexes.push_back(y1);
    ftVertexes.push_back(0);
    ftTextureCoords.push_back(0);
    ftTextureCoords.push_back(1);
    
    //1, 1
    ftVertexes.push_back(x1);
    ftVertexes.push_back(y1);
    ftVertexes.push_back(0);
    ftTextureCoords.push_back(1);
    ftTextureCoords.push_back(1);
    
    RenderDataObject *ftRenderData = new RenderDataObject();
    ftRenderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &ftVertexes.front());
    ftRenderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &ftTextureCoords.front());

    //Draw landscape to texture
    Rect oldViewport = RenderManager::Instance()->GetViewport();
    
    Texture *fullTiled = Texture::CreateFBO(TEXTURE_TILE_FULL_SIZE, TEXTURE_TILE_FULL_SIZE, FORMAT_RGBA8888, Texture::DEPTH_NONE);
    RenderManager::Instance()->SetRenderTarget(fullTiled);
    RenderManager::Instance()->SetViewport(Rect(0.f, 0.f, (float32)fullTiled->GetWidth(), (float32)fullTiled->GetHeight()));
    RenderManager::Instance()->SetClip(Rect(0.f, 0.f, -1.f, -1.f));

	RenderManager::Instance()->ClearWithColor(1.f, 0.f, 1.f, 1.f);
 
    Renderer::GetDynamicBindings().SetDynamicParam(PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    Renderer::GetDynamicBindings().SetDynamicParam(PARAM_VIEW, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    Matrix4 projection;
    projection.glOrtho(0.f, 1.f, 0.f, 1.f, -1.f, 1.f);
    
    Matrix4 *oldProjection = (Matrix4*)RenderManager::GetDynamicParam(PARAM_PROJ);
    Renderer::GetDynamicBindings().SetDynamicParam(PARAM_PROJ, &projection, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    
    prevLodLayer = -1;

    NMaterial* tmpLandscapeParent = NMaterial::CreateMaterial(FastName("Landscape_Tilemask_Material_TMP"), FastName("~res:/Materials/TileMask.material"), NMaterialQualityName::DEFAULT_QUALITY_NAME);
    NMaterial* tmpTileMaskMaterial = landscapeMaterial->Clone();
    

    //MAGIC: This magic code is workaround for situation when fbo textures are present in tileMaskMaterial with pathname. Because NMaterial::Clone() use pathnames for cloning textures.
    const uint32 texturesCount = landscapeMaterial->GetTextureCount();
    for(uint32 t = 0; t < texturesCount; ++t)
    {
        FastName tName = landscapeMaterial->GetTextureName(t);
        Texture *tex = landscapeMaterial->GetTexture(t);
        if(tex && tex->isRenderTarget)
        {
            tmpTileMaskMaterial->SetTexture(tName, tex);
        }
    }
    //END of MAGIC
    
    tmpTileMaskMaterial->SetFlag(NMaterialFlagName::FLAG_VERTEXFOG, NMaterial::FlagOff);
    tmpTileMaskMaterial->SetParent(tmpLandscapeParent);
    tmpTileMaskMaterial->BindMaterialTechnique(TECHNIQUE_TILEMASK_NAME, NULL);

	RenderManager::Instance()->SetRenderData(ftRenderData);
	RenderManager::Instance()->AttachRenderData();
	RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);

    RenderManager::Instance()->SetRenderTarget(0);
    
    Renderer::GetDynamicBindings().SetDynamicParam(PARAM_PROJ, &oldProjection, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
	RenderManager::Instance()->SetViewport(oldViewport);
    SafeRelease(ftRenderData);

    SafeRelease(tmpTileMaskMaterial);
    SafeRelease(tmpLandscapeParent);
    
    return fullTiled;
#else
    return nullptr;
#endif
}


//FilePath Landscape::SaveFullTiledTexture()
//{
//    FilePath pathToSave;
//    
//    if(textures[TEXTURE_TILE_FULL])
//    {
//        if(textures[TEXTURE_TILE_FULL]->isRenderTarget)
//        {
//            pathToSave = GetTextureName(TEXTURE_COLOR);
//            pathToSave.ReplaceExtension(".thumbnail.png");
//            Image *image = textures[TEXTURE_TILE_FULL]->CreateImageFromMemory();
//            if(image)
//            {
//                ImageLoader::Save(image, pathToSave);
//                SafeRelease(image);
//            }
//        }
//        else
//        {
//            pathToSave = textureNames[TEXTURE_TILE_FULL];
//        }
//    }
//    
//    Logger::FrameworkDebug("[LN] SaveFullTiledTexture: %s", pathToSave.GetAbsolutePathname().c_str());
//    return pathToSave;
//}
//    
//void Landscape::UpdateFullTiledTexture()
//{
//	//TODO: WTF? this method is called during load phase when not all properties have been initialized potentially!
//    if(textureNames[TEXTURE_TILE_FULL].IsEmpty())
//    {
//        Texture *t = CreateFullTiledTexture();
//        t->GenerateMipmaps();
//        SetTexture(TEXTURE_TILE_FULL, t);
//        SafeRelease(t);
//    }
//}

RenderObject * Landscape::Clone( RenderObject *newObject )
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<Landscape>(this), "Can clone only Landscape");
		newObject = new Landscape();
    }
    
    Landscape *newLandscape = static_cast<Landscape *>(newObject);
    newLandscape->landscapeMaterial = landscapeMaterial->Clone();

    newLandscape->flags = flags;
    newLandscape->BuildLandscapeFromHeightmapImage(heightmapPath, bbox);

	return newObject;
}
	
int32 Landscape::GetDrawIndices() const
{
    return drawIndices;
}

void Landscape::SetFoliageSystem(FoliageSystem* _foliageSystem)
{
    foliageSystem = _foliageSystem;
}
    
void Landscape::CollectNodesRecursive(LandQuadTreeNode<LandscapeQuad> * currentNode, int16 nodeSize,
                                                 Vector< LandQuadTreeNode<LandscapeQuad> * > & nodes)
{
    if(currentNode->data.size == nodeSize)
    {
        nodes.push_back(currentNode);
    }
    
    if(currentNode->children)
    {
        for(int32 i = 0; i < 4; ++i)
        {
            CollectNodesRecursive(&currentNode->children[i], nodeSize, nodes);
        }
    }
}
    
void Landscape::UpdatePart(Heightmap* fromHeightmap, const Rect2i & rect)
{
    DVASSERT(heightmap->Size() == fromHeightmap->Size());
    
    Vector<LandQuadTreeNode<LandscapeQuad> * > nodes;
    CollectNodesRecursive(&quadTreeHead, RENDER_QUAD_WIDTH - 1, nodes);
    
    for(LandQuadTreeNode<LandscapeQuad> * node : nodes)
    {
        Rect2i nodeRect(node->data.x, node->data.y, node->data.size, node->data.size);
        Rect2i intersect = nodeRect.Intersection(rect);
        if(intersect.dx || intersect.dy)
        {
            rhi::HVertexBuffer vertexBuffer = vertexBuffers[node->data.rdoQuad];

/*
            LandscapeVertex * rowVertices = new LandscapeVertex[intersect.dx + 1];
            for (int32 y = intersect.y; y < intersect.y + intersect.dy + 1; ++y)
            {
                int32 index = 0;
                for (int32 x = intersect.x; x < intersect.x + intersect.dx + 1; ++x)
                {
                    rowVertices[index].position = GetPoint(x, y, fromHeightmap->Data()[y * fromHeightmap->Size() + x]);
                    
                    Vector2 texCoord = Vector2((float32)(x) / (float32)(fromHeightmap->Size() - 1), 1.0f - (float32)(y) / (float32)(fromHeightmap->Size() - 1));
                    rowVertices[index].texCoord = texCoord;
                    
                    index++;
                }
                
                uint32 vBufferUpdateSize = (intersect.dx + 1) * sizeof(LandscapeVertex);
                uint32 vBufferOffset = 0; //TODO
                rhi::UpdateVertexBuffer(vertexBuffer, 0, vBufferOffset, vBufferUpdateSize);
            }
            SafeDeleteArray(rowVertices);
 */
            
            uint32 verticesCount = (node->data.size + 1) * (node->data.size + 1);
            LandscapeVertex * quadVertices = new LandscapeVertex[verticesCount];
            
            int32 index = 0;
            for (int32 y = node->data.y; y < node->data.y + node->data.size + 1; ++y)
            {
                for (int32 x = node->data.x; x < node->data.x + node->data.size + 1; ++x)
                {
                    quadVertices[index].position = GetPoint(x, y, fromHeightmap->Data()[y * fromHeightmap->Size() + x]);
                    
                    Vector2 texCoord = Vector2((float32)(x) / (float32)(fromHeightmap->Size() - 1), 1.0f - (float32)(y) / (float32)(fromHeightmap->Size() - 1));
                    quadVertices[index].texCoord = texCoord;
                    
                    index++;
                }
            }
            
            uint32 vBufferSize = verticesCount * sizeof(LandscapeVertex);
            rhi::UpdateVertexBuffer(vertexBuffer, quadVertices, 0, vBufferSize);
        }
    }
}
    
};

