#include "Scene/System/SceneCollisionSystem.h"


SceneCollisionSystem::SceneCollisionSystem(DAVA::Scene * scene)
	: DAVA::SceneSystem(scene)
{

}

SceneCollisionSystem::~SceneCollisionSystem()
{

}

void SceneCollisionSystem::Update(DAVA::float32 timeElapsed)
{

}

void SceneCollisionSystem::ProcessUIEvent(DAVA::UIEvent *event)
{

}

void SceneCollisionSystem::Draw()
{

}

void SceneCollisionSystem::AddEntity(DAVA::Entity * entity)
{
	if(NULL != entity)
	{

	}
	// check if this entity has render component
}

void SceneCollisionSystem::RemoveEntity(DAVA::Entity * entity)
{

}
