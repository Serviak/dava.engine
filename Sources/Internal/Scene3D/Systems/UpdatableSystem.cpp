#include "Scene3D/Systems/UpdatableSystem.h"
#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/SceneNode.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{

UpdatableSystem::UpdatableSystem(Scene * scene)
:	SceneSystem(scene)
{

}

void UpdatableSystem::AddEntity(SceneNode * entity)
{
	UpdatableComponent * component = (UpdatableComponent*)entity->GetComponent(Component::UPDATABLE_COMPONENT);
	IUpdatable * object = component->GetUpdatableObject();
	DVASSERT(object);

	IUpdatableBeforeTransform * updateBeforeTransform = dynamic_cast<IUpdatableBeforeTransform*>(object);
	if(updateBeforeTransform)
	{
		updatesBeforeTransform.push_back(updateBeforeTransform);
	}

	IUpdatableAfterTransform * updateAfterTransform = dynamic_cast<IUpdatableAfterTransform*>(object);
	if(updateAfterTransform)
	{
		updatesAfterTransform.push_back(updateAfterTransform);
	}
}

void UpdatableSystem::RemoveEntity(SceneNode * entity)
{
	UpdatableComponent * component = (UpdatableComponent*)entity->GetComponent(Component::UPDATABLE_COMPONENT);
	IUpdatable * object = component->GetUpdatableObject();

	if(object)
	{
		IUpdatableBeforeTransform * updateBeforeTransform = dynamic_cast<IUpdatableBeforeTransform*>(object);
		if(updateBeforeTransform)
		{
			uint32 size = updatesBeforeTransform.size();
			for(uint32 i = 0; i < size; ++i)
			{
				if(updatesBeforeTransform[i] == updateBeforeTransform)
				{
					updatesBeforeTransform[i] = updatesBeforeTransform[size-1];
					updatesBeforeTransform.pop_back();
					return;
				}
			}
		}

		IUpdatableAfterTransform * updateAfterTransform = dynamic_cast<IUpdatableAfterTransform*>(object);
		if(updateAfterTransform)
		{
			uint32 size = updatesAfterTransform.size();
			for(uint32 i = 0; i < size; ++i)
			{
				if(updatesAfterTransform[i] == updateAfterTransform)
				{
					updatesAfterTransform[i] = updatesAfterTransform[size-1];
					updatesAfterTransform.pop_back();
					return;
				}
			}
		}
	}
}

void UpdatableSystem::Process()
{

}

void UpdatableSystem::UpdatePreTransform()
{
	float32 timeElapsed = SystemTimer::Instance()->FrameDelta();
	uint32 size = updatesBeforeTransform.size();
	for(uint32 i = 0; i < size; ++i)
	{
		updatesBeforeTransform[i]->UpdateBeforeTransform(timeElapsed);
	}
}

void UpdatableSystem::UpdatePostTransform()
{
	float32 timeElapsed = SystemTimer::Instance()->FrameDelta();
	uint32 size = updatesAfterTransform.size();
	for(uint32 i = 0; i < size; ++i)
	{
		updatesAfterTransform[i]->UpdateAfterTransform(timeElapsed);
	}
}

}