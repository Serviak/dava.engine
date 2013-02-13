#ifndef __SCENE_GRAPH_COMMANDS_H__
#define __SCENE_GRAPH_COMMANDS_H__

#include "Command.h"

class SceneData;
class DAVA::SceneNode;
class CommandRemoveRootNodes: public Command
{
public:	
	CommandRemoveRootNodes();

protected:	
    
    virtual void Execute();
    virtual void Cancel();
    
private:
    
    SceneData * activeScene;
};


class CommandRefreshSceneGraph: public Command
{
public:	
	CommandRefreshSceneGraph();
    
protected:	
    
    virtual void Execute();
};

class CommandLockAtObject: public Command
{
public:
	CommandLockAtObject();
    
protected:
    
    virtual void Execute();
};


class CommandRemoveSceneNode: public Command
{
public:
	CommandRemoveSceneNode();
	virtual ~CommandRemoveSceneNode();
    
protected:
	DAVA::SceneNode* undoNode;

    virtual void Execute();
    virtual void Cancel();
    
private:
    
    SceneData * activeScene;
};


class CommandDebugFlags: public Command
{
    
public:	
	CommandDebugFlags();
    
protected:	
    
    virtual void Execute();
};





#endif // #ifndef __SCENE_GRAPH_COMMANDS_H__