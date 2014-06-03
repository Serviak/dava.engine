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


#include "Render/Highlevel/SpeedTreeObject.h"
#include "Render/Material/NMaterialNames.h"
#include "Utils/Utils.h"

namespace DAVA 
{
    
const FastName SpeedTreeObject::FLAG_WIND_ANIMATION("WIND_ANIMATION");

SpeedTreeObject::SpeedTreeObject()
{
    type = TYPE_SPEED_TREE;
}
    
SpeedTreeObject::~SpeedTreeObject()
{
}
    
void SpeedTreeObject::RecalcBoundingBox()
{
    bbox = AABBox3();

    uint32 size = (uint32)renderBatchArray.size();
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch * rb = renderBatchArray[k].renderBatch;
        bbox.AddAABBox(CalcBBoxForSpeedTreeGeometry(rb));
    }
}

void SpeedTreeObject::SetTreeAnimationParams(const Vector2 & trunkOscillationParams, const Vector2 & leafOscillationParams)
{
    trunkOscillation = trunkOscillationParams;
    leafOscillation = leafOscillationParams;
}

void SpeedTreeObject::BindDynamicParams()
{
    RenderManager::SetDynamicParam(PARAM_SPEED_TREE_TRUNK_OSCILLATION, &trunkOscillation, UPDATE_SEMANTIC_ALWAYS);
    RenderManager::SetDynamicParam(PARAM_SPEED_TREE_LEAFS_OSCILLATION, &leafOscillation, UPDATE_SEMANTIC_ALWAYS);
}

void SpeedTreeObject::UpdateAnimationFlag(int32 maxAnimatedLod)
{
    uint32 size = (uint32)renderBatchArray.size();
    for (uint32 k = 0; k < size; ++k)
    {
        NMaterial::eFlagValue flagValue = (renderBatchArray[k].lodIndex > maxAnimatedLod) ? NMaterial::FlagOff : NMaterial::FlagOn;
        renderBatchArray[k].renderBatch->GetMaterial()->SetFlag(FLAG_WIND_ANIMATION, flagValue);
    }
}

RenderObject * SpeedTreeObject::Clone(RenderObject *newObject)
{
    if(!newObject)
    {
        DVASSERT_MSG(IsPointerToExactClass<SpeedTreeObject>(this), "Can clone only SpeedTreeObject");
        newObject = new SpeedTreeObject();
    }

    RenderObject::Clone(newObject);
    
    return newObject;
}

AABBox3 SpeedTreeObject::CalcBBoxForSpeedTreeGeometry(RenderBatch * rb)
{
    if(IsTreeLeafBatch(rb))
    {
        AABBox3 pgBbox;
        PolygonGroup * pg = rb->GetPolygonGroup();

        if((pg->GetFormat() & EVF_PIVOT) == 0)
            return rb->GetBoundingBox();

        int32 vertexCount = pg->GetVertexCount();
        for(int32 vi = 0; vi < vertexCount; vi++)
        {
            Vector3 pivot;
            pg->GetPivot(vi, pivot);

            Vector3 pointX, pointY, pointZ;
            Vector3 offsetX, offsetY;

            pg->GetCoord(vi, pointZ);
            offsetX = offsetY = pointZ - pivot;

            Swap(offsetX.x, offsetX.z);
            Swap(offsetX.y, offsetX.z);
            
            pointX = pivot + offsetX;
            pointY = pivot + offsetY;

            pgBbox.AddPoint(pointX);
            pgBbox.AddPoint(pointY);
            pgBbox.AddPoint(pointZ);
        }

        return pgBbox;
    }

    return rb->GetBoundingBox();
}

bool SpeedTreeObject::IsTreeLeafBatch(RenderBatch * batch)
{
    if(batch && batch->GetMaterial())
    {
        const NMaterialTemplate * material = batch->GetMaterial()->GetMaterialTemplate();
        return (material->name == NMaterialName::SPEEDTREE_LEAF) || (material->name == NMaterialName::SPHERICLIT_SPEEDTREE_LEAF);
    }
    return false;
}

};
