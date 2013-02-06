/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#ifndef __DAVAENGINE_DATANODE_H__
#define __DAVAENGINE_DATANODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/SceneNode.h"

namespace DAVA
{

/**
    
 */
class SceneFileV2;
class DataNode : public BaseObject
{
public:	
	DataNode();
	virtual ~DataNode();
    virtual int32 Release();

    
    /**
     */
    void SetScene(Scene * _scene);
    inline Scene * GetScene() { return scene; };
    
    /**
        \brief Set name of this particular node.
        \param[in] new name for this node
     */
    void SetName(const String & name);

    /**
        \brief Get name of this particular node.
        \returns name of this node
     */
    const String & GetName();
    
    DataNode *	FindByName(const String & searchName);
    virtual void	AddNode(DataNode * node);
	virtual void	RemoveNode(DataNode * node);
	virtual DataNode * GetChild(int32 index);
	virtual int32   GetChildrenCount();
	virtual void	RemoveAllChildren();

    //DataNode * FindByAddress();
    int32  GetNodeIndex();
    uint64 GetPreviousPointer(); 
    
    /**
        \brief virtual function to save node to KeyedArchive
     */
    virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFile);
    
    /**
        \brief virtual function to load node to KeyedArchive
     */
	virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFile);
    
protected:
    uint64 pointer;
    Scene * scene;
    String name;
    Vector<DataNode*> children;
    int32 index;
    
public:
    
    INTROSPECTION_EXTEND(DataNode, BaseObject,
        MEMBER(name, "Name", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(index, "Index", INTROSPECTION_SERIALIZABLE)
        MEMBER(pointer, "Pointer", INTROSPECTION_SERIALIZABLE)
        COLLECTION(children, "Children", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};
    
/*class DataNodeArray : public BaseObject
{
public:
    DataNodeArray(Scene * _scene);
    
    virtual void	AddNode(DataNode * node);
	virtual void	RemoveNode(DataNode * node);
	virtual DataNode * GetChild(int32 index);
	virtual int32   GetChildrenCount();
	virtual void	RemoveAllChildren();

protected:
    
    Scene * scene;
};*/
    

};

#endif // __DAVAENGINE_SCENENODE_H__





