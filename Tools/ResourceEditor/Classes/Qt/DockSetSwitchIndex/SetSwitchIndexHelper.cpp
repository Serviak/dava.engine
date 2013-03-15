#include "SetSwitchIndexHelper.h"
#include "../Qt/Scene/SceneDataManager.h"

namespace DAVA
{

void SetSwitchIndexHelper::ProcessSwitchIndexUpdate(uint32 value, eSET_SWITCH_INDEX state)
{
	for(int i = 0; i < SceneDataManager::Instance()->SceneCount(); ++i)
	{
		SceneData *sceneData = SceneDataManager::Instance()->SceneGet(i);
		
		List<Entity*> switchComponents;
		if( SetSwitchIndexHelper::FOR_SELECTED == state)
		{
			Entity *selectedNode = SceneDataManager::Instance()->SceneGetSelectedNode(sceneData);
			if(NULL != selectedNode)
			{
				selectedNode->FindAllSwitchComponentsRecursive(switchComponents);
			}
		}
		if( SetSwitchIndexHelper::FOR_SCENE == state)
		{
			sceneData->GetScene()->FindAllSwitchComponentsRecursive( switchComponents);
		}
		
		for(List<Entity*>::const_iterator it = switchComponents.begin(); it != switchComponents.end(); ++it)
		{
			
			SwitchComponent * switchComponent = cast_if_equal<SwitchComponent*>((*it)->GetComponent(Component::SWITCH_COMPONENT));
			if(NULL != switchComponent)
			{
				uint32 rangeSize = switchComponent->GetEntity()->GetChildrenCount();
				switchComponent->SetSwitchIndex(value);
			}
		}
	}
}

};
