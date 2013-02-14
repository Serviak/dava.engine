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
#ifndef __DAVAENGINE_SCENE3D_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Base/Introspection.h"

namespace DAVA 
{
    
class DataNode;
class SceneNode;
class Component
{
public:
    enum eType
    {
        TRANSFORM_COMPONENT = 0,
        RENDER_COMPONENT,
        DEBUG_RENDER_COMPONENT, 
		LOD_COMPONENT,
		SWITCH_COMPONENT,
        CAMERA_COMPONENT,
        LIGHT_COMPONENT,
		PARTICLE_EFFECT_COMPONENT,
		BULLET_COMPONENT,
		UPDATABLE_COMPONENT,
        ANIMATION_COMPONENT,
        COLLISION_COMPONENT,    // multiple instances
        PHYSICS_COMPONENT,
        ACTION_COMPONENT,       // actions, something simplier than scripts that can influence logic, can be multiple
        SCRIPT_COMPONENT,       // multiple instances, not now, it will happen much later.
        COMPONENT_COUNT,
    };

	static Component * CreateByType(uint32 componentType);

	Component();
    
    virtual ~Component() {};
    virtual uint32 GetType() = 0;
    virtual Component * Clone(SceneNode * toEntity) = 0;

	virtual void SetEntity(SceneNode * entity);
    
    /**
         \brief This function should be implemented in each node that have data nodes inside it.
     */
    virtual void GetDataNodes(Set<DataNode*> & dataNodes);
    /**
         \brief Function to get data nodes of requested type to specific container you provide.
     */
    template<template <typename> class Container, class T>
	void GetDataNodes(Container<T> & container);

    SceneNode * GetEntity() { return entity; };
protected:
    SceneNode * entity;

public:
	INTROSPECTION(Component, 
		MEMBER(entity, "entity", INTROSPECTION_SERIALIZABLE)
		);
};

    
#define IMPLEMENT_COMPONENT_TYPE(TYPE) virtual uint32 GetType() { return TYPE; }; 

    
template<template <typename> class Container, class T>
void Component::GetDataNodes(Container<T> & container)
{
    container.clear();
    
    Set<DataNode*> objects;
    GetDataNodes(objects);
    
    Set<DataNode*>::const_iterator end = objects.end();
    for (Set<DataNode*>::iterator t = objects.begin(); t != end; ++t)
    {
        DataNode* obj = *t;
        
        T res = dynamic_cast<T> (obj);
        if (res)
            container.push_back(res);
    }	
}

    
    
};
#endif //__DAVAENGINE_SCENE3D_COMPONENT_H__
