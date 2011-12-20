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

namespace DAVA
{
	
SceneNode::SceneNode(Scene * _scene)
	: scene(_scene)
	, parent(0)
    , visible(true)
    , inUpdate(false)
    , tag(0)
{
	localTransform.Identity();
	worldTransform.Identity();
    defaultLocalTransform.Identity();
	//animation = 0;
    debugFlags = DEBUG_DRAW_NONE;
    flags = 0;
	isSolidNode = false;
	userData = 0;
}

SceneNode::~SceneNode()
{
    RemoveAllChildren();
	SafeDelete(userData);
}

void SceneNode::SetParent(SceneNode * node)
{
	parent = node;
}

void SceneNode::AddNode(SceneNode * node)
{
	if (node)
    {
        node->Retain();
        childs.push_back(node);
        node->SetParent(this);
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
    std::vector<SceneNode*>::iterator childsEnd = childs.end();
	for (std::vector<SceneNode*>::iterator t = childs.begin(); t != childsEnd; ++t)
	{
		if (*t == node)
		{
			childs.erase(t);
            if (node)
            {
                node->SetParent(0);
                node->Release();
            }
			break;
		}
	}
	
}
	
SceneNode * SceneNode::GetChild(int32 index)
{
	return childs[index];
}

int32 SceneNode::GetChildrenCount()
{
    return (int32)childs.size();
}

void SceneNode::RemoveAllChildren()
{
	for (std::vector<SceneNode*>::iterator t = childs.begin(); t != childs.end(); ++t)
	{
        SceneNode *node = *t;
        node->SetParent(0);
        node->Release();
	}
	childs.clear();
}


SceneNode *	SceneNode::FindByName(const String & searchName)
{
	if (name == searchName)
		return this;
	
	uint32 size = (uint32)childs.size();
	for (uint32 c = 0; c < size; ++c)
	{
		SceneNode * res = childs[c]->FindByName(searchName);
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
		uint32 size = (uint32)childs.size();
		for (uint32 c = 0; c < size; ++c)
			childs[c]->StopAllAnimations(recursive);
	}	
}

void SceneNode::RestoreOriginalTransforms()
{
    SetLocalTransform(GetDefaultLocalTransform());
	
	uint32 size = (uint32)childs.size();
	for (uint32 c = 0; c < size; ++c)
		childs[c]->RestoreOriginalTransforms();
}
	
void SceneNode::ExtractCurrentNodeKeyForAnimation(SceneNodeAnimationKey & key)
{
	key.time = 0.0f;
	key.translation.x = localTransform._30;
	key.translation.y = localTransform._31;
	key.translation.z = localTransform._32;
	key.rotation.Construct(localTransform);
	//key.matrix = localTransform;
}

    
void SceneNode::Update(float32 timeElapsed)
{
    inUpdate = true;
	// TODO - move node update to render because any of objects can change params of other objects
	if (nodeAnimations.size() != 0)
	{
		Quaternion blendedRotation;
		Vector3 blendedTranslation;
		float32 accumWeight = 0.0f;
		std::deque<SceneNodeAnimation*>::const_iterator end = nodeAnimations.end();
		for (std::deque<SceneNodeAnimation*>::iterator it = nodeAnimations.begin(); it != end; ++it)
		{
			SceneNodeAnimation * animation = *it;
			SceneNodeAnimationKey & key = animation->Intepolate(animation->GetCurrentTime());
			if (accumWeight == 0.0f)
			{
				blendedTranslation = key.translation;
				blendedRotation = key.rotation;
				accumWeight = animation->weight;
			}else
			{
				float32 factor = animation->weight / (accumWeight + animation->weight);
				accumWeight += accumWeight;
				blendedTranslation.Lerp(blendedTranslation, key.translation, factor);
				blendedRotation.Slerp(blendedRotation, key.rotation, factor);
			}
			//key.GetMatrix(localTransform);
		}
		Matrix4 localTransformTrans;
		Matrix4 localTransformRot;
		Matrix4 localTransformFinal;
		localTransformTrans.CreateTranslation(blendedTranslation);
		localTransformRot = blendedRotation.GetMatrix();
		
		localTransform = localTransformRot * localTransformTrans;
		
//		if (nodeAnimations.size() != 1)
//		{
//			printf("-- blended node: %s\n", name.c_str());
//			std::deque<SceneNodeAnimation*>::const_iterator end = nodeAnimations.end();
//			for (std::deque<SceneNodeAnimation*>::iterator it = nodeAnimations.begin(); it != end; ++it)
//			{
//				SceneNodeAnimation * animation = *it;
//				printf(">>> blend: %s wei: %f inDelay: %f\n", animation->GetParent()->name.c_str(), animation->weight, animation->delayTime);
//			}
//		}
	}
	
    // update world transform only in case if 
    if (!(flags & NODE_WORLD_MATRIX_ACTUAL))  
	{
		if (parent)
        {
            worldTransform = localTransform * parent->worldTransform;
        }else 
		{
            worldTransform = localTransform;
        }
        
        // need propagate changes to child nodes
        flags |= NODE_WORLD_MATRIX_ACTUAL;
        uint32 size = (uint32)childs.size();
        for (uint32 c = 0; c < size; ++c)
        {
            childs[c]->InvalidateLocalTransform();
            childs[c]->Update(timeElapsed);
        }
        
	}
    else 
    {
        uint32 size = (uint32)childs.size();
        for (uint32 c = 0; c < size; ++c)
        {
            childs[c]->Update(timeElapsed);
        }
    }

	//printf("- node: %s tr: %f %f %f\n", name.c_str(), localTransform.data[12], localTransform.data[13], localTransform.data[14]); 
	
	
	inUpdate = false;

    if (!removedCache.empty()) 
    {
        for (std::deque<SceneNode*>::iterator t = removedCache.begin(); t != removedCache.end(); ++t)
        {
            RemoveNode(*t);
        }
        removedCache.clear();
    }
}

void SceneNode::Draw()
{
	//uint32 size = (uint32)childs.size();
    Vector<SceneNode*>::iterator itEnd = childs.end();
	for (Vector<SceneNode*>::iterator it = childs.begin(); it != itEnd; ++it)
		(*it)->Draw();
    if (scene)
        scene->nodeCounter++;
}

    
Matrix4 SceneNode::AccamulateLocalTransform(SceneNode *fromParent)
{
    if (fromParent == this) 
    {
        return localTransform;
    }
    return localTransform * parent->AccamulateLocalTransform(fromParent);
}

    
SceneNode* SceneNode::Clone(SceneNode *dstNode)
{
    if (!dstNode) 
    {
        dstNode = new SceneNode(scene);
    }
    dstNode->visible = visible;
    dstNode->defaultLocalTransform = defaultLocalTransform;
    
    dstNode->localTransform = localTransform;
    dstNode->worldTransform = worldTransform;
    dstNode->name = name;
    dstNode->tag = tag;
    dstNode->debugFlags = debugFlags;
	dstNode->isSolidNode = isSolidNode;

//    Logger::Debug("Node %s clonned", name.c_str());
    
    dstNode->nodeAnimations = nodeAnimations;
    
    
//    Logger::Debug("Children +++++++++++++++++++++++++++++++");
    std::vector<SceneNode*>::iterator it = childs.begin();
    
    std::vector<SceneNode*>::iterator childsEnd = childs.end();
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
        std::vector<SceneNode*>::iterator it = childs.begin();
        std::vector<SceneNode*>::iterator childsEnd = childs.end();
        for(; it != childsEnd; it++)
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
    
    int32 sz = (int32)childs.size();
    for (int32 i = 0; i < sz; i++) 
    {
        if (childs[i]->FindNodesByNamePart(namePart, outNodeList)) 
        {
            isFind = true;
        }
    }
    
    return isFind;
}

AABBox3 SceneNode::GetWTMaximumBoundingBox()
{
    AABBox3 retBBox;
    Vector<SceneNode*>::iterator itEnd = childs.end();
	for (Vector<SceneNode*>::iterator it = childs.begin(); it != itEnd; ++it)
    {
        AABBox3 box = (*it)->GetWTMaximumBoundingBox();
        if(  (AABBOX_INFINITY != box.min.x && AABBOX_INFINITY != box.min.y && AABBOX_INFINITY != box.min.z)
           &&(-AABBOX_INFINITY != box.max.x && -AABBOX_INFINITY != box.max.y && -AABBOX_INFINITY != box.max.z))
        {
            retBBox.AddAABBox(box);
        }
    }
    
    return retBBox;
}

};




