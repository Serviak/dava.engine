/*
 *  GameCore.h
 *  TemplateProjectMacOS
 *
 *  Created by Vitaliy  Borodovsky on 3/19/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "DAVAEngine.h"

class ResourcePackerScreen;
class SceneEditorScreen;
class SceneEditorScreenMain;
class ExporterScreen;

class GameCore : public DAVA::ApplicationCore
{
public:	
	GameCore();
	virtual ~GameCore();
	
	virtual void OnAppStarted();
	virtual void OnAppFinished();
	
	virtual void OnSuspend();
	virtual void OnResume();
	virtual void OnBackground();
	
	virtual void BeginFrame();
	virtual void Update(DAVA::float32 update);
	virtual void Draw();
	
private:
    
#if defined (DAVA_QT)
    void ResizeScreens();
    DAVA::Vector2 virtualSize;
#endif //#if defined (DAVA_QT)
    
	ResourcePackerScreen * resourcePackerScreen;
    SceneEditorScreenMain * sceneEditorScreenMain;
    
    ExporterScreen *exporterScreen;
    
};



#endif // __GAMECORE_H__