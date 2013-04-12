#ifndef __SCENE_GRID_SYSTEM_H__
#define __SCENE_GRID_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "UI/UIEvent.h"

class SceneGridSystem : public DAVA::SceneSystem
{
	friend class SceneEditorProxy;

public:
	SceneGridSystem(DAVA::Scene * scene);
	~SceneGridSystem();

protected:
	virtual void Update(float timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
	void Draw();
};

#endif