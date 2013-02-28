#include "Scene3D/SwitchNode.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{

REGISTER_CLASS(SwitchNode);

SwitchNode::SwitchNode()
:	oldSwitchIndex(0),
	newSwitchIndex(0)
{

}

SceneNode* SwitchNode::Clone(SceneNode *dstNode /*= NULL*/)
{
	if (!dstNode) 
	{
		dstNode = new SwitchNode();
	}

	return SceneNode::Clone(dstNode);
}

void SwitchNode::SetSwitchIndex(int32 _switchIndex)
{
	newSwitchIndex = _switchIndex;
}

int32 SwitchNode::GetSwitchIndex()
{
	return newSwitchIndex;
}

void SwitchNode::Update(float32 timeElapsed)
{
	if(oldSwitchIndex != newSwitchIndex)
	{
		int32 childrenCound = GetChildrenCount();
		for(int32 i = 0; i < childrenCound; ++i)
		{
			GetChild(i)->SetUpdatable(newSwitchIndex == i);
		}

		oldSwitchIndex = newSwitchIndex;
	}
}

void SwitchNode::AddNode(SceneNode * node)
{
	SceneNode::AddNode(node);

	ReapplySwitch();
}

void SwitchNode::ReapplySwitch()
{
	oldSwitchIndex = -1;
}

void SwitchNode::Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
{
	SceneNode::Save(archive, sceneFileV2);

	archive->SetInt32("switchIndex", newSwitchIndex);
}

void SwitchNode::Load(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
{
	SceneNode::Load(archive, sceneFileV2);

	int32 loadedSwitchIndex = archive->GetInt32("switchIndex");
	SetSwitchIndex(loadedSwitchIndex);
}

}
