/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "BaseParticleEditorNode.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "FileSystem/KeyedArchive.h"
using namespace DAVA;

BaseParticleEditorNode::BaseParticleEditorNode(Entity* rootNode) :
    ExtraUserData()
{
	Component *effectComponent = rootNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT);
	DVASSERT(effectComponent);

    this->isMarkedForSelection = false;
    this->rootNode = rootNode;
    SetParentNode(NULL);

	extraData = new KeyedArchive();
}

BaseParticleEditorNode::~BaseParticleEditorNode()
{
    Cleanup();
}

void BaseParticleEditorNode::Cleanup()
{
    for (List<BaseParticleEditorNode*>::iterator iter = childNodes.begin(); iter != childNodes.end();
         iter ++)
    {
        SAFE_DELETE(*iter);
    }
    
    childNodes.clear();
	SetParentNode(NULL);

	ClearExtraData();
	SafeRelease(extraData);
}

void BaseParticleEditorNode::AddChildNode(BaseParticleEditorNode* childNode)
{
    if (!childNode)
    {
        return;
    }
    
	childNode->SetParentNode(this);
    this->childNodes.push_back(childNode);
}

ParticleEffectComponent* BaseParticleEditorNode::GetParticleEffectComponent() const
{
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(rootNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	DVASSERT(effectComponent);
	return effectComponent;
}

void BaseParticleEditorNode::AddChildNodeAbove(BaseParticleEditorNode* childNode, BaseParticleEditorNode* childNodeToMoveAbove)
{
	AddChildNode(childNode);
	if (childNodeToMoveAbove)
	{
		MoveChildNode(childNode, childNodeToMoveAbove);
	}
}

void BaseParticleEditorNode::RemoveChildNode(BaseParticleEditorNode* childNode, bool needDeleteNode)
{
    this->childNodes.remove(childNode);
	
	if (needDeleteNode)
	{
		childNode->SetParentNode(NULL);
		SAFE_DELETE(childNode);
	}
}

void BaseParticleEditorNode::MoveChildNode(BaseParticleEditorNode* childNode, BaseParticleEditorNode* childNodeToMoveAbove)
{
	PARTICLEEDITORNODESLIST::iterator curPositionIter = std::find(this->childNodes.begin(),
																  this->childNodes.end(),
																  childNode);
	PARTICLEEDITORNODESLIST::iterator newPositionIter = std::find(this->childNodes.begin(),
																  this->childNodes.end(),
																  childNodeToMoveAbove);

	if (curPositionIter == this->childNodes.end() ||
		newPositionIter == this->childNodes.end() ||
		curPositionIter == newPositionIter)
	{
		// No way to move.
		return;
	}

	childNodes.remove(childNode);
	
	// Re-calculate the new position iter - it might be changed during remove.
	newPositionIter = std::find(this->childNodes.begin(), this->childNodes.end(), childNodeToMoveAbove);
	childNodes.insert(newPositionIter, childNode);
}

KeyedArchive* BaseParticleEditorNode::GetExtraData()
{
	return extraData;
}

void BaseParticleEditorNode::ClearExtraData()
{
	extraData->DeleteAllKeys();
}
