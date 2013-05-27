#ifndef __SCENE_GRAPH_COMMANDS_H__
#define __SCENE_GRAPH_COMMANDS_H__

#include "Command.h"

class SceneData;
class DAVA::Entity;
class CommandRemoveRootNodes: public MultiCommand
{
public:	
	CommandRemoveRootNodes();
	virtual ~CommandRemoveRootNodes();

protected:
	virtual void Execute();
	virtual void Cancel();
	virtual DAVA::Set<DAVA::Entity*> GetAffectedEntities();

	CommandInternalRemoveSceneNode* removeCmd;
};


class CommandLockAtObject: public Command
{
public:
	DAVA_DEPRECATED(CommandLockAtObject());// DEPRECATED: using SceneDataManager(QOBJECT)
    
protected:
    
    virtual void Execute();
};


class CommandRemoveSceneNode: public MultiCommand
{
public:
	CommandRemoveSceneNode();
	virtual ~CommandRemoveSceneNode();

protected:
    virtual void Execute();
	virtual void Cancel();
	virtual DAVA::Set<DAVA::Entity*> GetAffectedEntities();

	CommandInternalRemoveSceneNode* removeCmd;
};

class CommandInternalRemoveSceneNode: public Command
{
public:
	CommandInternalRemoveSceneNode(DAVA::Entity* node, bool removeSimilar = false);
	virtual ~CommandInternalRemoveSceneNode();
	
protected:
	struct RemoveNodeRec
	{
		DAVA::Entity* node;
		DAVA::Entity* insertBeforeNode;
		DAVA::Entity* nodeParent;

		RemoveNodeRec()
		:	node(NULL)
		,	insertBeforeNode(NULL)
		,	nodeParent(NULL)
		{}
	};

	DAVA::Vector<RemoveNodeRec> nodesForDeletion;
	DAVA::Entity* selectedNode;

    virtual void Execute();
    virtual void Cancel();

	DAVA::int32 GetNodeIndex(const RemoveNodeRec& nodeRec);

	virtual DAVA::Set<DAVA::Entity*> GetAffectedEntities();
};


class CommandDebugFlags: public Command
{
    
public:	
	DAVA_DEPRECATED(CommandDebugFlags());// DEPRECATED: using SceneDataManager(QOBJECT)
    
protected:	
    
    virtual void Execute();
};




#endif // #ifndef __SCENE_GRAPH_COMMANDS_H__