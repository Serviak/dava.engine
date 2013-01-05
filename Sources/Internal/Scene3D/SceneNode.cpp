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
#include "Scene3D/SceneNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneNodeAnimation.h"
#include "Scene3D/SceneNodeAnimationList.h"
#include "FileSystem/KeyedArchive.h"
#include "Base/ObjectFactory.h"
#include "Utils/StringFormat.h"
#include "Render/RenderHelper.h"
#include "Scene3D/SceneFileV2.h"
#include "FileSystem/FileSystem.h"
#include "Debug/Stats.h"
#include "Scene3D/TransformSystem.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/DeleteSystem.h"

namespace DAVA
{
    
    
REGISTER_CLASS(SceneNode);
	
SceneNode::SceneNode()
	: scene(0)
	, parent(0)
    , inUpdate(false)
    , tag(0)
	, entity(0)
	, transformComponent(0)
    , renderComponent(0)
{
//    Logger::Debug("SceneNode: %p", this);
    componentFlags = 0;
    for (uint32 k = 0; k < Component::COMPONENT_COUNT; ++k)
        components[k] = 0;
    
	worldTransform.Identity();
    defaultLocalTransform.Identity();
	//animation = 0;
    debugFlags = DEBUG_DRAW_NONE;
    flags = NODE_VISIBLE | NODE_UPDATABLE | NODE_LOCAL_MATRIX_IDENTITY;
	userData = 0;
    
    customProperties = new KeyedArchive();

	AddComponent(new TransformComponent());
    
//    Stats::Instance()->RegisterEvent("Scene.Update.SceneNode.Update", "SceneNode update time");
//    Stats::Instance()->RegisterEvent("Scene.Draw.SceneNode.Draw", "SceneNode draw time");
}

SceneNode::~SceneNode()
{
    /*
        TODO: Double check that everything is working fine.
     */
//    if (scene)
//    {
//        scene->UnregisterNode(this);
//        scene = 0;
//    }
    DVASSERT(scene == 0);
    
    RemoveAllChildren();
	SafeRelease(userData);
    SafeRelease(customProperties);

	for(int32 i = 0; i < Component::COMPONENT_COUNT; ++i)
	{
		if(components[i])
		{
			RemoveComponent(components[i]);
		}
	}

//  Logger::Debug("~SceneNode: %p", this);

	//TODO: delete entity?
}
    
    
void SceneNode::UpdateComponentsFastPtrs()
{
    transformComponent = dynamic_cast<TransformComponent*>(components[Component::TRANSFORM_COMPONENT]);
    renderComponent = dynamic_cast<RenderComponent*>(components[Component::RENDER_COMPONENT]);
}

    
void SceneNode::AddComponent(Component * component)
{
    SafeDelete(components[component->GetType()]);
    components[component->GetType()] = component;
    if (scene)
        scene->AddComponent(this, component);
    componentFlags |= 1 << component->GetType();
    UpdateComponentsFastPtrs();
}

void SceneNode::RemoveComponent(Component * component)
{
    components[component->GetType()] = 0;
    if (scene)
        scene->RemoveComponent(this, component);
    componentFlags &= ~(1 << component->GetType());
	delete(component);
    UpdateComponentsFastPtrs();
}

void SceneNode::SetScene(Scene * _scene)
{
    if (scene == _scene)
    {
        return;
    }
    // Сheck 
    if (scene)scene->UnregisterNode(this);
    scene = _scene;
    if (scene)scene->RegisterNode(this);
    
    const std::vector<SceneNode*>::iterator & childrenEnd = children.end();
	for (std::vector<SceneNode*>::iterator t = children.begin(); t != childrenEnd; ++t)
	{
        (*t)->SetScene(_scene);
    }
}
    
Scene * SceneNode::GetScene()
{
    return scene;
}


void SceneNode::SetParent(SceneNode * node)
{
	parent = node;
	transformComponent->SetParent(parent);
    if (scene)
        scene->ImmediateUpdate(this, transformComponent);
}

void SceneNode::AddNode(SceneNode * node)
{
	if (node)
    {
        node->Retain();
        children.push_back(node);
        if (node->parent)
        {
            node->parent->RemoveNode(node);
        }
        node->SetScene(GetScene());
        node->SetParent(this);
    }
}
    
void SceneNode::InsertBeforeNode(SceneNode *newNode, SceneNode *beforeNode)
{
    if (newNode)
    {
        const Vector<SceneNode*>::iterator &itEnd = children.end();
        for (Vector<SceneNode*>::iterator it = children.begin(); it != itEnd; ++it)
        {
            if(beforeNode == (*it))
            {
                newNode->Retain();
                children.insert(it, newNode);
                if (newNode->parent)
                {
                    newNode->parent->RemoveNode(newNode);
                }
                newNode->SetParent(this);
                newNode->SetScene(GetScene());
                break;
            }
        }
    }
}

void SceneNode::RemoveNode(SceneNode * node)
{
    if (!node) 
    {
        return;
    }
    if (inUpdate) 
    {
        removedCache.push_back(node);
        return;
    }
    const std::vector<SceneNode*>::iterator & childrenEnd = children.end();
	for (std::vector<SceneNode*>::iterator t = children.begin(); t != childrenEnd; ++t)
	{
		if (*t == node)
		{
			children.erase(t);
            if (node)
            {
                node->SetScene(0);
                node->SetParent(0);
                node->Release();
            }
			break;
		}
	}
	
}
	
SceneNode * SceneNode::GetChild(int32 index)
{
	return children[index];
}

int32 SceneNode::GetChildrenCount()
{
    return (int32)children.size();
}
int32 SceneNode::GetChildrenCountRecursive()
{
    int32 result = 0;
    result += (int32)children.size();
    for (std::vector<SceneNode*>::iterator t = children.begin(); t != children.end(); ++t)
	{
        SceneNode *node = *t;
        result += node->GetChildrenCountRecursive();
    }
    return result;
}

    
void SceneNode::RemoveAllChildren()
{
	for (std::vector<SceneNode*>::iterator t = children.begin(); t != children.end(); ++t)
	{
        SceneNode *node = *t;
        node->SetScene(0);
        node->SetParent(0);
        node->Release();
	}
	children.clear();
}


SceneNode *	SceneNode::FindByName(const String & searchName)
{
	if (name == searchName)
		return this;
	
	uint32 size = (uint32)children.size();
	for (uint32 c = 0; c < size; ++c)
	{
		SceneNode * res = children[c]->FindByName(searchName);
		if (res != 0)return res;
	}
	return 0;
}

	
void SceneNode::ExecuteAnimation(SceneNodeAnimation * _animation)
{
	nodeAnimations.push_back(_animation);
//	printf("-- add animation: %d node: %s anim: %s\n", nodeAnimations.size(), name.c_str(), _animation->GetParent()->name.c_str()); 
//	if (_animation->GetParent()->name == "a1")
//	{
//		int k = 0;
//		k++;
//	}
}
	
void SceneNode::DetachAnimation(SceneNodeAnimation * animation)
{
//	int32 size = nodeAnimations.size();
	for (std::deque<SceneNodeAnimation*>::iterator t = nodeAnimations.begin(); t != nodeAnimations.end(); ++t)
	{
		if (*t == animation)
		{
			nodeAnimations.erase(t);
			break;
		}
	}
//	int32 sizeAfter = nodeAnimations.size();
//	if (sizeAfter != size - 1)
//	{
//		printf("******** Error with animation detach");
//	}	
}

void SceneNode::StopAllAnimations(bool recursive)
{
	nodeAnimations.clear();
	if (recursive)
	{
		uint32 size = (uint32)children.size();
		for (uint32 c = 0; c < size; ++c)
			children[c]->StopAllAnimations(recursive);
	}	
}

void SceneNode::RestoreOriginalTransforms()
{
    SetLocalTransform(GetDefaultLocalTransform());
	
	uint32 size = (uint32)children.size();
	for (uint32 c = 0; c < size; ++c)
		children[c]->RestoreOriginalTransforms();
}
    
void SceneNode::BakeTransforms()
{
    uint32 size = (uint32)children.size();
    if(size == 1) // propagate matrices
    {
        for (uint32 c = 0; c < size; ++c)
        {
            children[c]->SetLocalTransform(children[c]->GetLocalTransform() * GetLocalTransform());
            children[c]->SetDefaultLocalTransform(children[c]->GetDefaultLocalTransform() * defaultLocalTransform);
        }
        SetLocalTransform(Matrix4::IDENTITY);
        AddFlag(NODE_LOCAL_MATRIX_IDENTITY);
    }

	for(uint32 c = 0; c < size; ++c)
	{
		children[c]->BakeTransforms();
	}
}

void SceneNode::PropagateBoolProperty(String name, bool value)
{
	KeyedArchive *currentProperties = GetCustomProperties();
	currentProperties->SetBool(name, value);

	uint32 size = (uint32)children.size();
	if (size > 0) // propagate value to children
	{
		for (uint32 c = 0; c < size; ++c)
		{
			children[c]->PropagateBoolProperty(name, value);
		}
	}
}


	
void SceneNode::ExtractCurrentNodeKeyForAnimation(SceneNodeAnimationKey & key)
{
	const Matrix4 & localTransform = GetLocalTransform();
	key.time = 0.0f;
	key.translation.x = localTransform._30;
	key.translation.y = localTransform._31;
	key.translation.z = localTransform._32;
	key.rotation.Construct(localTransform);
	//key.matrix = localTransform;
}

    
//void SceneNode::Update(float32 timeElapsed)
//{
//    //Stats::Instance()->BeginTimeMeasure("Scene.Update.SceneNode.Update", this);
//
////    if (!(flags & NODE_UPDATABLE))return;
//
//    inUpdate = true;
//	// TODO - move node update to render because any of objects can change params of other objects
//	if (nodeAnimations.size() != 0)
//	{
//		Quaternion blendedRotation;
//		Vector3 blendedTranslation;
//		float32 accumWeight = 0.0f;
//		std::deque<SceneNodeAnimation*>::const_iterator end = nodeAnimations.end();
//		for (std::deque<SceneNodeAnimation*>::iterator it = nodeAnimations.begin(); it != end; ++it)
//		{
//			SceneNodeAnimation * animation = *it;
//			SceneNodeAnimationKey & key = animation->Intepolate(animation->GetCurrentTime());
//			if (accumWeight == 0.0f)
//			{
//				blendedTranslation = key.translation;
//				blendedRotation = key.rotation;
//				accumWeight = animation->weight;
//			}else
//			{
//				float32 factor = animation->weight / (accumWeight + animation->weight);
//				accumWeight += accumWeight;
//				blendedTranslation.Lerp(blendedTranslation, key.translation, factor);
//				blendedRotation.Slerp(blendedRotation, key.rotation, factor);
//			}
//			//key.GetMatrix(localTransform);
//		}
//		Matrix4 localTransformTrans;
//		Matrix4 localTransformRot;
//		Matrix4 localTransformFinal;
//		localTransformTrans.CreateTranslation(blendedTranslation);
//		localTransformRot = blendedRotation.GetMatrix();
//		
//		localTransform = localTransformRot * localTransformTrans;
//		
////		if (nodeAnimations.size() != 1)
////		{
////			printf("-- blended node: %s\n", name.c_str());
////			std::deque<SceneNodeAnimation*>::const_iterator end = nodeAnimations.end();
////			for (std::deque<SceneNodeAnimation*>::iterator it = nodeAnimations.begin(); it != end; ++it)
////			{
////				SceneNodeAnimation * animation = *it;
////				printf(">>> blend: %s wei: %f inDelay: %f\n", animation->GetParent()->name.c_str(), animation->weight, animation->delayTime);
////			}
////		}
//	}
//	
//	UpdateTransform();
//	uint32 size = (uint32)children.size();
//	for (uint32 c = 0; c < size; ++c)
//	{
//		children[c]->Update(timeElapsed);
//	}
//
//	//printf("- node: %s tr: %f %f %f\n", name.c_str(), localTransform.data[12], localTransform.data[13], localTransform.data[14]); 
//	
//	
//	inUpdate = false;
//
//    if (!removedCache.empty()) 
//    {
//        for (std::deque<SceneNode*>::iterator t = removedCache.begin(); t != removedCache.end(); ++t)
//        {
//            RemoveNode(*t);
//        }
//        removedCache.clear();
//    }
//    //Stats::Instance()->EndTimeMeasure("Scene.Update.SceneNode.Update", this);
//}

void SceneNode::Draw()
{
    //Stats::Instance()->BeginTimeMeasure("Scene.Draw.SceneNode.Draw", this);
    
	if (!(flags & NODE_VISIBLE) || !(flags & NODE_UPDATABLE) || (flags & NODE_INVALID))return;

	//uint32 size = (uint32)children.size();
    const Vector<SceneNode*>::iterator & itEnd = children.end();
	for (Vector<SceneNode*>::iterator it = children.begin(); it != itEnd; ++it)
		(*it)->Draw();
    if (scene)
        scene->nodeCounter++;

	
	if (debugFlags & DEBUG_DRAW_AABOX_CORNERS)
	{
//		Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
//		Matrix4 finalMatrix = worldTransform * prevMatrix;
//		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);
		
		AABBox3 box = GetWTMaximumBoundingBoxSlow();
		if(box == AABBox3())
		{
			box.min = Vector3(0, 0, 0) * GetWorldTransform();
			box.max = box.min;
		}

        RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
        RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_DEPTH_TEST); 
		RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		RenderHelper::Instance()->DrawCornerBox(box);
        RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
        RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
//		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
	}

	if (debugFlags & DEBUG_DRAW_RED_AABBOX)
	{
		AABBox3 box = GetWTMaximumBoundingBoxSlow();
		RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
		RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_DEPTH_TEST); 
		RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		RenderHelper::Instance()->DrawBox(box);
		RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
		RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	}

	
	//Stats::Instance()->EndTimeMeasure("Scene.Draw.SceneNode.Draw", this);
}

    
void SceneNode::SceneDidLoaded()
{
    const Vector<SceneNode*>::const_iterator & itEnd = children.end();
	for (Vector<SceneNode*>::iterator it = children.begin(); it != itEnd; ++it)
		(*it)->SceneDidLoaded();
}
    
SceneNode* SceneNode::Clone(SceneNode *dstNode)
{
    if (!dstNode) 
	{
		DVASSERT_MSG(IsPointerToExactClass<SceneNode>(this), "Can clone only SceneNode");
		dstNode = new SceneNode();
    }
    dstNode->defaultLocalTransform = defaultLocalTransform;
    
    //uint32 size = components.size();
    for (uint32 k = 0; k < Component::COMPONENT_COUNT;++k)
	{
		if(components[k])
		{
			SafeDelete(dstNode->components[k]);
			dstNode->components[k] = components[k]->Clone();
		}
	}
    dstNode->UpdateComponentsFastPtrs();
    
    dstNode->worldTransform = worldTransform;
    dstNode->name = name;
    dstNode->tag = tag;
    dstNode->debugFlags = debugFlags;
    dstNode->flags = flags;

	dstNode->RemoveFlag(SceneNode::TRANSFORM_NEED_UPDATE);
	dstNode->RemoveFlag(SceneNode::TRANSFORM_DIRTY);

    SafeRelease(dstNode->customProperties);
    dstNode->customProperties = new KeyedArchive(*customProperties);

//    Logger::Debug("Node %s clonned", name.c_str());
    
    dstNode->nodeAnimations = nodeAnimations;
    
    
//    Logger::Debug("Children +++++++++++++++++++++++++++++++");
    std::vector<SceneNode*>::iterator it = children.begin();
    
    const std::vector<SceneNode*>::iterator & childsEnd = children.end();
    for(; it != childsEnd; it++)
    {
        SceneNode *n = (*it)->Clone();
        dstNode->AddNode(n);
        n->Release();
    }
//    Logger::Debug("Children -------------------------------");
    
    return dstNode;
}

void SceneNode::SetDebugFlags(uint32 _debugFlags, bool isRecursive)
{
    debugFlags = _debugFlags;
    if (isRecursive)
    {
        std::vector<SceneNode*>::iterator it = children.begin();
        const std::vector<SceneNode*>::iterator & childrenEnd = children.end();
        for(; it != childrenEnd; it++)
        {
            SceneNode *n = (*it);
            n->SetDebugFlags(_debugFlags, isRecursive);
        }
    }
}

void SceneNode::SetName(const String & _name)
{
    name = _name;
}

String SceneNode::GetFullName()
{
    return RecursiveBuildFullName(this, scene);
}

String SceneNode::RecursiveBuildFullName(SceneNode * node, SceneNode * endNode)
{
    if (!node)
        return "";
        
    if (node->GetParent() != endNode)
    {
        return RecursiveBuildFullName(node->GetParent(), endNode) + String("->") + node->name; 
    }else
    {
        return node->name;
    }
}
    
bool SceneNode::FindNodesByNamePart(const String &namePart, List<SceneNode *> &outNodeList)
{
    bool isFind = false;
    size_t fp = name.find(namePart);
    if (fp != String::npos) 
    {
        outNodeList.push_back(this);
        isFind = true;
    }
    
    int32 sz = (int32)children.size();
    for (int32 i = 0; i < sz; i++) 
    {
        if (children[i]->FindNodesByNamePart(namePart, outNodeList)) 
        {
            isFind = true;
        }
    }
    
    return isFind;
}

AABBox3 SceneNode::GetWTMaximumBoundingBoxSlow()
{
    AABBox3 retBBox;
    const Vector<SceneNode*>::iterator & itEnd = children.end();
	for (Vector<SceneNode*>::iterator it = children.begin(); it != itEnd; ++it)
    {
        AABBox3 box = (*it)->GetWTMaximumBoundingBoxSlow();
        if(  (AABBOX_INFINITY != box.min.x && AABBOX_INFINITY != box.min.y && AABBOX_INFINITY != box.min.z)
           &&(-AABBOX_INFINITY != box.max.x && -AABBOX_INFINITY != box.max.y && -AABBOX_INFINITY != box.max.z))
        {
            retBBox.AddAABBox(box);
        }
    }
    
    return retBBox;
}
    
String SceneNode::GetDebugDescription()
{
    return Format("children: %d ", GetChildrenCount());
}

    
void SceneNode::Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
{
    // Perform refactoring and add Matrix4, Vector4 types to VariantType and KeyedArchive
    BaseObject::Save(archive);
    
    String savedPath = "";
    if(customProperties && customProperties->IsKeyExists("editor.referenceToOwner"))
    {
        savedPath = customProperties->GetString("editor.referenceToOwner");
        String newPath = sceneFileV2->AbsoluteToRelative(savedPath);
        customProperties->SetString("editor.referenceToOwner", newPath);
    }
    
    archive->SetString("name", name);
    archive->SetInt32("tag", tag);
    archive->SetByteArrayAsType("localTransform", GetLocalTransform());
    archive->SetByteArrayAsType("defaultLocalTransform", defaultLocalTransform);
    
    archive->SetUInt32("flags", flags);
//    archive->SetUInt32("debugFlags", debugFlags);
    
    archive->SetByteArrayFromArchive("customprops", customProperties);
    
    if(customProperties && savedPath.length())
    {
        customProperties->SetString("editor.referenceToOwner", savedPath);
    }
}

void SceneNode::Load(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
{
    BaseObject::Load(archive);
        
    name = archive->GetString("name", "");
    tag = archive->GetInt32("tag", 0);

	flags = archive->GetUInt32("flags", NODE_VISIBLE);
	flags |= NODE_UPDATABLE;

    const Matrix4 & localTransform = archive->GetByteArrayAsType("localTransform", GetLocalTransform());
	SetLocalTransform(localTransform);
    defaultLocalTransform = archive->GetByteArrayAsType("defaultLocalTransform", defaultLocalTransform);

    /// InvalidateLocalTransform();
//    debugFlags = archive->GetUInt32("debugFlags", 0);
    
    SafeRelease(customProperties);
    customProperties = archive->GetArchiveFromByteArray("customprops");
    if (!customProperties)
    {
        customProperties = new KeyedArchive();
    }
    else
    {
        if(customProperties->IsKeyExists("editor.referenceToOwner"))
        {
            String newPath = sceneFileV2->RelativeToAbsolute(customProperties->GetString("editor.referenceToOwner"));
            customProperties->SetString("editor.referenceToOwner", newPath);
        }
    }
}

KeyedArchive * SceneNode::GetCustomProperties()
{
    return customProperties;
}
    
void SceneNode::SetSolid(bool isSolid)
{
//    isSolidNode = isSolid;
    customProperties->SetBool("editor.isSolid", isSolid);
}
    
bool SceneNode::GetSolid()
{
//    return isSolidNode;
    return customProperties->GetBool("editor.isSolid", false);
}

void SceneNode::GetDataNodes(Set<DataNode*> & dataNodes)
{
    uint32 size = (uint32)children.size();
    for (uint32 c = 0; c < size; ++c)
    {
        children[c]->GetDataNodes(dataNodes);
    }
}

    
void SceneNode::AddFlagRecursive(int32 flagToAdd)
{
    AddFlag(flagToAdd);
    const Vector<SceneNode*>::iterator &itEnd = children.end();
	for (Vector<SceneNode*>::iterator it = children.begin(); it != itEnd; ++it)
    {
        (*it)->AddFlagRecursive(flagToAdd);
    }
}

void SceneNode::RemoveFlagRecursive(int32 flagToRemove)
{
    RemoveFlag(flagToRemove);
    const Vector<SceneNode*>::iterator &itEnd = children.end();
	for (Vector<SceneNode*>::iterator it = children.begin(); it != itEnd; ++it)
    {
        (*it)->RemoveFlagRecursive(flagToRemove);
    }
}

bool SceneNode::IsLodMain(SceneNode *childToCheck)
{
    if (!parent || !IsLodPart()) 
    {
        return true;
    }
    
    return parent->IsLodMain(this);
}

String SceneNode::GetPathID(SceneNode * root)
{
	String result;
	SceneNode * curr = this;
	SceneNode * parent = NULL;
	int32 sz, i;

	while (curr != root)
	{
		parent = curr->GetParent();
		sz = parent->GetChildrenCount();
		for (i = 0; i < sz; i++)
		{
			if (curr == parent->GetChild(i))
			{
				result = Format("%d:", i) + result;
				break;
			}
		}
		curr = parent;
	}
	return result;
}

SceneNode * SceneNode::GetNodeByPathID(SceneNode * root, String pathID)
{
	SceneNode * result = root;
	int32 offs = 0;
	int32 index = 0;
	int32 sz = pathID.size();
	char val;
	while (offs < sz)
	{
		val = pathID[offs];
		if (val < '0' || val > '9')
		{
			offs++;
			if (index >=0 && result->GetChildrenCount() > index)
				result = result->GetChild(index);
			else
				return NULL;
			continue;
		}
		index = index * 10 + val - '0';
		offs++;
	}
	return result;
}
    
void SceneNode::SetFog_Kostil(float32 density, const Color &color)
{
    Vector<Material *> materials;
    GetDataNodes(materials);
    
    for(int32 i = 0; i < (int32)materials.size(); ++i)
    {
        materials[i]->SetFogDensity(density);
        materials[i]->SetFogColor(color);
    }
}

inline const Matrix4 & SceneNode::ModifyLocalTransform()
{
    flags &= ~(NODE_WORLD_MATRIX_ACTUAL | NODE_LOCAL_MATRIX_IDENTITY);
    //scene->transformSystem->NeedUpdate(this);
    scene->ImmediateUpdate(this, transformComponent);
    return GetLocalTransform();
}

void SceneNode::SetLocalTransform(const Matrix4 & newMatrix)
{
    transformComponent->SetLocalTransform(&newMatrix);
//    scene->transformSystem->NeedUpdate(this);
    if (scene)
        scene->ImmediateUpdate(this, transformComponent);
    
    //localTransform = newMatrix;
    //flags &= ~NODE_WORLD_MATRIX_ACTUAL;
    //if (newMatrix == Matrix4::IDENTITY)flags |= NODE_LOCAL_MATRIX_IDENTITY;
    //else flags &= ~NODE_LOCAL_MATRIX_IDENTITY;
}

const Matrix4 & SceneNode::GetLocalTransform()
{
	return *(transformComponent->GetLocalTransform());
}

const Matrix4 & SceneNode::GetWorldTransform()
{
	return *(transformComponent->GetWorldTransform());
}

TransformComponent * SceneNode::GetTransformComponent()
{
	return transformComponent;
}

int32 SceneNode::Release()
{
	if(1 == referenceCount)
	{
		AddFlag(SceneNode::NODE_DELETED);
		SetScene(0);
		DeleteSystem::Instance()->MarkNodeAsDeleted(this);
		return referenceCount;
	}
	else
	{
		return BaseObject::Release();
	}
}

void SceneNode::SetVisible(bool isVisible)
{
	if(isVisible) 
	{
		AddFlag(NODE_VISIBLE);
		if(renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() | RenderObject::VISIBLE);
		}
	}
	else 
	{
		RemoveFlag(NODE_VISIBLE);
		if(renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() & ~RenderObject::VISIBLE);
		}
	}


}

void SceneNode::SetUpdatable(bool isUpdatable)
{
	if(isUpdatable) 
	{
		AddFlag(NODE_UPDATABLE);
		if(renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() | RenderObject::VISIBLE);
		}
	}
	else 
	{
		RemoveFlag(NODE_UPDATABLE);
		if(renderComponent)
		{
			renderComponent->GetRenderObject()->SetFlags(renderComponent->GetRenderObject()->GetFlags() & ~RenderObject::VISIBLE);
		}
	}
}

};
