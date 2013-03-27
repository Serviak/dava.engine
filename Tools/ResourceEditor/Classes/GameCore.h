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
class SceneUtilsScreen;
class ImageSplitterScreen;

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
    
    void ResizeScreens();
    DAVA::Vector2 virtualSize;
    
	ResourcePackerScreen * resourcePackerScreen;
    SceneEditorScreenMain * sceneEditorScreenMain;
    
    SceneUtilsScreen *sceneUtilsScreen;
    
    ImageSplitterScreen *imageSplitterScreen;
};



#endif // __GAMECORE_H__