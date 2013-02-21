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
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Material.h"
#include "Render/RenderDataObject.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderFastNames.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA
{

REGISTER_CLASS(RenderBatch)
    
RenderBatch::RenderBatch()
    :   ownerLayer(0)
    ,   removeIndex(-1)
    ,   sortingKey(8)
{
    dataSource = 0;
    renderDataObject = 0;
    material = 0;
    startIndex = 0;
    indexCount = 0;
    type = PRIMITIVETYPE_TRIANGLELIST;
	renderObject = 0;
    materialInstance = new InstanceMaterialState();
    ownerLayerName = INHERIT_FROM_MATERIAL;
}
    
RenderBatch::~RenderBatch()
{
	SafeRelease(dataSource);
	SafeRelease(renderDataObject);
	SafeRelease(material);
    SafeRelease(materialInstance);
}
    
void RenderBatch::SetPolygonGroup(PolygonGroup * _polygonGroup)
{
	SafeRelease(dataSource);
    dataSource = SafeRetain(_polygonGroup);
	if(NULL != dataSource)
	{
		aabbox = dataSource->GetBoundingBox();
	}
}

void RenderBatch::SetRenderDataObject(RenderDataObject * _renderDataObject)
{
	SafeRelease(renderDataObject);
    renderDataObject = SafeRetain(_renderDataObject);
}

void RenderBatch::SetMaterial(Material * _material)
{
	SafeRelease(material);
    material = SafeRetain(_material);
}

    
void RenderBatch::Draw(Camera * camera)
{
	if(!renderObject)return;
    Matrix4 * worldTransformPtr = renderObject->GetWorldTransformPtr();
    if (!worldTransformPtr)
    {
        return;
    }
    
    uint32 flags = renderObject->GetFlags();
    if ((flags & RenderObject::VISIBILITY_CRITERIA) != RenderObject::VISIBILITY_CRITERIA)
        return;
	
    Matrix4 finalMatrix = (*worldTransformPtr) * camera->GetMatrix();
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);
    material->Draw(dataSource,  materialInstance);
}
    
    
const FastName & RenderBatch::GetOwnerLayerName()
{
    if (ownerLayerName == INHERIT_FROM_MATERIAL)
    {
        DVASSERT(material != 0);
        return material->GetOwnerLayerName();
    }
    else
    {
        return ownerLayerName;
    }
}
    
void RenderBatch::SetOwnerLayerName(const FastName & fastname)
{
    ownerLayerName = fastname;
}
    
//const FastName & RenderBatch::GetOwnerLayerName()
//{
//    static FastName opaqueLayer("OpaqueRenderLayer");
//    static FastName translucentLayer("TransclucentRenderLayer");
//    
//    if (material)
//    {
//        if(material->GetOpaque() || material->GetAlphablend())
//		{
//			return translucentLayer;
//		}
//    }
//    
//    return opaqueLayer;
//}

void RenderBatch::SetRenderObject(RenderObject * _renderObject)
{
	renderObject = _renderObject;
}

const AABBox3 & RenderBatch::GetBoundingBox() const
{
    return aabbox;
}
    
    
inline void RenderBatch::SetSortingKey(uint32 _key)
{
    sortingKey = _key;
    if (ownerLayer)ownerLayer->ForceLayerSort();
}


void RenderBatch::GetDataNodes(Set<DataNode*> & dataNodes)
{
	if(material)
	{
		InsertDataNode(material, dataNodes);
	}

	if(dataSource)
	{
		InsertDataNode(dataSource, dataNodes);
	}
}

void RenderBatch::InsertDataNode(DataNode *node, Set<DataNode*> & dataNodes)
{
	dataNodes.insert(node);

	for(int32 i = 0; i < node->GetChildrenCount(); ++i)
	{
		InsertDataNode(node->GetChild(i), dataNodes);
	}
}

RenderBatch * RenderBatch::Clone(RenderBatch * destination)
{
    RenderBatch * rb = destination;
    if (!rb)
        rb = new RenderBatch();

	rb->dataSource = SafeRetain(dataSource);
	rb->renderDataObject = SafeRetain(renderDataObject);
	rb->material = SafeRetain(material);
    rb->materialInstance = SafeRetain(materialInstance);

	rb->startIndex = startIndex;
	rb->indexCount = indexCount;
	rb->type = type;

	rb->aabbox = aabbox;

	rb->ownerLayerName = ownerLayerName;
	rb->sortingKey = sortingKey;
// TODO: Understand what this code means.
// 
//	rb->ownerLayer = ownerLayer;
//	if(ownerLayer)
//	{
//		ownerLayer->AddRenderBatch(rb);
//	}

	return rb;
}

void RenderBatch::Save(KeyedArchive * archive, SceneFileV2* sceneFile)
{
	BaseObject::Save(archive);

	if(NULL != archive)
	{
		archive->SetUInt32("rb.type", type);
		archive->SetUInt32("rb.indexCount", indexCount);
		archive->SetUInt32("rb.startIndex", startIndex);
		archive->SetVariant("rb.aabbox", VariantType(aabbox));
		archive->SetVariant("rb.datasource", VariantType(dataSource));
		archive->SetVariant("rb.maretial", VariantType(GetMaterial()));
		
		KeyedArchive *mia = new KeyedArchive();
		materialInstance->Save(mia, sceneFile);
		archive->SetArchive("rb.matinst", mia);
		mia->Release();
	}
}

void RenderBatch::Load(KeyedArchive * archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive)
	{
		type = archive->GetUInt32("rb.type", type);
		indexCount = archive->GetUInt32("rb.indexCount", indexCount);
		startIndex = archive->GetUInt32("rb.startIndex", startIndex);
		aabbox = archive->GetVariant("rb.aabbox")->AsAABBox3();

		PolygonGroup *pg = dynamic_cast<PolygonGroup*>(sceneFile->GetNodeByPointer((uint64) archive->GetVariant("rb.datasource")->AsPointer()));
		Material *mat = dynamic_cast<Material*>(sceneFile->GetNodeByPointer((uint64) archive->GetVariant("rb.maretial")->AsPointer()));

		SetPolygonGroup(pg);
		SetMaterial(mat);

		KeyedArchive *mia = archive->GetArchive("rb.matinst");
		if(NULL != mia)
		{
			materialInstance->Load(mia, sceneFile);
		}
	}

	BaseObject::Load(archive);
}

};
