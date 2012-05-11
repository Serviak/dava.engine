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
#ifndef __DAVAENGINE_SCENENODE_H__
#define __DAVAENGINE_SCENENODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/SceneNodeAnimationKey.h"
#include <deque>

namespace DAVA
{

class Scene;
class SceneNodeAnimation;
class SceneNodeAnimationKey;
class KeyedArchive;
class SceneFileV2;
class DataNode;
/**
    \brief Base class of 3D scene hierarchy. All nodes in our scene graph is inherited from this node.
 */
class SceneNode : public BaseObject
{
public:	
	SceneNode();
	virtual ~SceneNode();
	
    /**
        \brief Function to set scene for node and it's children. 
        Function goes recursively and set scene for this node, and each child. 
        \param[in] _scene pointer to scene we want to set as holder for this node. 
     */
    virtual void SetScene(Scene * _scene);
    /**
        \brief Function to return scene of this node. This is virtual function. For SceneNode's function returns it's scene value. 
        In Scene class function is overloaded and returns self. It required to avoid dynamic casts to find a scene. 
        \returns pointer to the scene that holds this node. 
     */
    virtual Scene * GetScene();

    
	// working with childs
	virtual void	AddNode(SceneNode * node);
    
    virtual void    InsertBeforeNode(SceneNode *newNode, SceneNode *beforeNode);
    
	virtual void	RemoveNode(SceneNode * node);
	virtual SceneNode * GetChild(int32 index);
	virtual int32   GetChildrenCount();
	virtual void	RemoveAllChildren();
        
	virtual bool FindNodesByNamePart(const String & namePart, List<SceneNode *> &outNodeList);
    
    /**
        \brief Find node by it's name inside this scene node.
        \param[in] name name of object you want to find. 
        \returns pointer to the object if object with such name available, 0 in case if it's not exists.
     */
	virtual SceneNode *	FindByName(const String & name);
    /**
        \brief Set name of this particular node.
        \param[in] new name for this node
     */
    virtual void SetName(const String & name);

    /**
        \brief Get name of this particular node.
        \returns name of this node
     */
    inline const String & GetName();

    /**
        \brief Get full name of this node from root. This function is slow because it go up by hierarchy and make full node name.
        \returns this node full name from root. Example [MaxScene->camera->instance0]
     */
    String GetFullName();
    
    /**
        \brief Set tag for this object.
        Tag can be used to identify object, or find it. You can mark objects with same properies by tag, and later find them using tag criteria. 
     */
    inline void SetTag(int32 _tag);
    
    /**
        \brief Return tag for this object
        \returns tag for this object
     */
    inline const int32 GetTag(); 

	
	// virtual updates
	virtual void	Update(float32 timeElapsed);
	virtual void	Draw();
	
	// properties
	inline void SetVisible(bool isVisible);
	inline bool GetVisible(void);
	inline SceneNode * GetParent();
	inline void SetUpdatable(bool isUpdatable);
	inline bool GetUpdatable(void);
	inline bool IsLodPart(void);
    virtual bool IsLodMain(SceneNode *childToCheck = NULL);//if childToCheck is NULL checks the caller node
	
	// extract data from current node to use it in animations
	void ExtractCurrentNodeKeyForAnimation(SceneNodeAnimationKey & resultKey);
	
    inline const Matrix4 & GetLocalTransform(); 

    /**
     \brief Accamulates local transform from the requested parent to this node.
     */
    Matrix4 AccamulateLocalTransform(SceneNode *fromParent);

    /**
     \brief This method means that you always modify geted matrix. 
        If you dont want to modify matrix call GetLocalTransform().
     */
    inline Matrix4 & ModifyLocalTransform(); 
    inline const Matrix4 & GetWorldTransform();
    inline const Matrix4 & GetDefaultLocalTransform(); 
    
    inline void SetLocalTransform(const Matrix4 & newMatrix);
    //inline void SetWorldTransform(const Matrix4 & newMatrix);
    inline void SetDefaultLocalTransform(const Matrix4 & newMatrix);
    inline void InvalidateLocalTransform();
    
    /*
        \brief Go down by hierarchy and bake all transforms.
        Function can be used to bake transforms to minimize amount of matrix multiplications.
     */
    virtual void BakeTransforms();
    
	enum
    {
        // NODE_STATIC = 0x1,  // this flag means that node is always static and we do not need to update it's worldTransform
        // NODE_DYNAMIC = 0x2, // node automatically become dynamic when we update it's local matrix
        NODE_WORLD_MATRIX_ACTUAL = 1, // if this flag set this means we do not need to rebuild worldMatrix
        NODE_VISIBLE = 1 << 1, // is node and subnodes should draw
        NODE_UPDATABLE = 1 << 2, // is node and subnodes should updates. This flag is updated by the engine and can be changed at any time. Flag is always rise up on node loading
        NODE_IS_LOD_PART = 1 << 3, // node is part of a LOD node.
        NODE_LOCAL_MATRIX_IDENTITY = 1 << 4, // local matrix of this node is identity. Used to avoid unnecessary computations.
        
        BOUNDING_VOLUME_AABB = 1 << 5,  // node has axial aligned bounding box.
        BOUNDING_VOLUME_OOB = 1 << 6,   // node has object oriented bounding box.
        BOUNDING_VOLUME_SPHERE = 1 << 7,    // node has bounding sphere.

        NODE_CLIPPED_PREV_FRAME = 1 << 8, // 
        NODE_CLIPPED_THIS_FRAME = 1 << 9, // 
        NODE_INVALID = 1 << 10,  // THIS NODE not passed some of verification stages and marked as invalid. Such nodes shouldn't be drawn.
        
        // I decided to put scene flags here to avoid 2 variables. But probably we can create additional variable later if it'll be required.
        SCENE_LIGHTS_MODIFIED = 1 << 31,
    };
	
    inline void AddFlag(int32 flagToAdd);
    inline void RemoveFlag(int32 flagToRemove);
    inline uint32 GetFlags() const;
    void AddFlagRecursive(int32 flagToAdd);
    void RemoveFlagRecursive(int32 flagToRemove);
    
	// animations 
	void ExecuteAnimation(SceneNodeAnimation * animation);	
	void DetachAnimation(SceneNodeAnimation * animation);
	virtual void StopAllAnimations(bool recursive = true);
	void RestoreOriginalTransforms();

	
    virtual SceneNode* Clone(SceneNode *dstNode = NULL);
	
    // Do not use variables 
    std::deque<SceneNodeAnimation *> nodeAnimations;

	enum
	{
		DEBUG_DRAW_NONE = 0,
		DEBUG_DRAW_AABBOX = 1,              
		DEBUG_DRAW_LOCAL_AXIS = 2,
		DEBUG_DRAW_AABOX_CORNERS = 4,
		DEBUG_DRAW_LIGHT_NODE = 8,
        DEBUG_DRAW_NORMALS = 16,
        DEBUG_DRAW_GRID = 32,
        DEBUG_DRAW_ALL = 0xFFFFFFFF,
	};
	/**
        \brief function to enable or disable debug drawing for particular node.
        By default it's not recursive. Some objects may support flags only partially.
        For example if node do not have bounding box flag DEBUG_DRAW_AABBOX will not produce any output
        These flags are mostly for debug purposes and we do not guarantee that logic of the debug rendering will remain unchanged between 
        framework versions.
     
        \param[in] debugFlags flags to be set
        \param[in] isRecursive do you want to set flags recursively
     
     */
	void SetDebugFlags(uint32 debugFlags, bool isRecursive = false);  
    /**
        \brief function returns debug flags of specific node
        \returns flags of this specific scene node
     */
    uint32 GetDebugFlags() const;
    	
    void SetSolid(bool isSolid);
    bool GetSolid();
    
	inline void SetUserData(BaseObject * newData)
	{
		SafeRelease(userData);
		userData = newData;
		SafeRetain(userData);
	}

	inline BaseObject * GetUserData(void)
	{
		return userData;
	}
	
    /**
        \brief function returns maximum bounding box of scene in world coordinates.
        \returns bounding box
     */
    virtual AABBox3 GetWTMaximumBoundingBox();
    
    /**
        \brief virtual function to save node to KeyedArchive
     */
    virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2);
    
    /**
        \brief virtual function to load node to KeyedArchive
     */
	virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFileV2);
    
    /**
        \brief Function to get node description for debug printing
     */
    virtual String GetDebugDescription();
    
    /**
        \brief Function returns keyed archive of custom properties for this object. 
        Custom properties can be set for each node in editor, and used in the game later to implement game logic.
     */
    KeyedArchive *GetCustomProperties();
    
    /**
        \brief This function should be implemented in each node that have data nodes inside it.
     */
    virtual void GetDataNodes(Set<DataNode*> & dataNodes);
    /**
        \brief Function to get data nodes of requested type to specific container you provide.
     */
    template<template <typename> class Container, class T>
	void GetDataNodes(Container<T> & container);
    
    /**
        \brief Function to get child nodes of requested type and move them to specific container you provide.
        For example if you want to get a list of MeshInstanceNodes you should do the following.
        \code   
        #include "Scene3D/SceneNode.h"
        #include "Scene3D/MeshInstanceNode.h"  // You should include MeshInstanceNode because SceneNode class do not know the type of node you want to convert to. 
        
        void YourClass::YourFunction()
        {
            List<MeshInstanceNode*> meshNodes;
            scene->GetChildNodes(meshNodes);
        }
        \endcode
     */
    template<template <typename> class Container, class T>
	void GetChildNodes(Container<T> & container);
        
    /**
        \brief This function is called after scene is loaded from file.
        You can perform additional initialization here.
     */
    virtual void SceneDidLoaded();
    
protected:

    String RecursiveBuildFullName(SceneNode * node, SceneNode * endNode);
    
//    virtual SceneNode* CopyDataTo(SceneNode *dstNode);
	void SetParent(SceneNode * node);
	BaseObject * userData;

	Scene * scene;
	SceneNode * parent;
	Vector<SceneNode*> children;
	Deque<SceneNode*> removedCache;
    bool inUpdate;

	String	name;
	int32	tag;

    uint32 flags;
    uint32 debugFlags;

	Matrix4 worldTransform;

    KeyedArchive *customProperties;
    
private:
    Matrix4 localTransform;
    Matrix4 defaultLocalTransform;
    
};

inline void SceneNode::SetVisible(bool isVisible)
{
    if (isVisible) 
    {
        AddFlag(NODE_VISIBLE);
    }
    else 
    {
        RemoveFlag(NODE_VISIBLE);
    }
}
	
inline bool SceneNode::GetVisible(void)
{
	return (flags & NODE_VISIBLE) != 0;
}
	
inline void SceneNode::SetUpdatable(bool isUpdatable)
{
    if (isUpdatable) 
    {
        AddFlag(NODE_UPDATABLE);
    }
    else 
    {
        RemoveFlag(NODE_UPDATABLE);
    }
}
    
inline bool SceneNode::GetUpdatable(void)
{
	return (flags & NODE_UPDATABLE) != 0;
}
    
inline bool SceneNode::IsLodPart(void)
{
	return (flags & NODE_IS_LOD_PART) != 0;
}


inline void SceneNode::AddFlag(int32 flagToAdd)
{
    flags |= flagToAdd;
}
    
inline void SceneNode::RemoveFlag(int32 flagToRemove)
{
    flags &= ~flagToRemove;
}
    
inline uint32 SceneNode::GetFlags() const
{
    return flags;
}

inline SceneNode * SceneNode::GetParent()
{
	return parent;
}
    
inline const String & SceneNode::GetName()
{
    return name;
}

inline const int32 SceneNode::GetTag() 
{ 
    return tag; 
}
    
inline const Matrix4 & SceneNode::GetLocalTransform() 
{ 
    return localTransform; 
}; 

inline const Matrix4 & SceneNode::GetWorldTransform() 
{ 
    return worldTransform; 
};
    
inline const Matrix4 & SceneNode::GetDefaultLocalTransform()
{
    return defaultLocalTransform;
}
    
inline Matrix4 & SceneNode::ModifyLocalTransform()
{
    flags &= ~(NODE_WORLD_MATRIX_ACTUAL | NODE_LOCAL_MATRIX_IDENTITY);
    return localTransform;
}

inline void SceneNode::SetLocalTransform(const Matrix4 & newMatrix)
{
    localTransform = newMatrix;
    flags &= ~NODE_WORLD_MATRIX_ACTUAL;
    if (newMatrix == Matrix4::IDENTITY)flags |= NODE_LOCAL_MATRIX_IDENTITY;
    else flags &= ~NODE_LOCAL_MATRIX_IDENTITY;
}
//
//inline void SceneNode::SetWorldTransform(const Matrix4 & newMatrix)
//{
//    worldTransform = newMatrix;
//}
//
    
inline void SceneNode::InvalidateLocalTransform()
{
    flags &= ~(NODE_WORLD_MATRIX_ACTUAL | NODE_LOCAL_MATRIX_IDENTITY);
}

    
inline void SceneNode::SetDefaultLocalTransform(const Matrix4 & newMatrix)
{
    defaultLocalTransform = newMatrix;
}
    
inline void SceneNode::SetTag(int32 _tag)
{
    tag = _tag;
}
    
inline uint32 SceneNode::GetDebugFlags() const
{
    return debugFlags;
}
    
template<template <typename> class Container, class T>
void SceneNode::GetDataNodes(Container<T> & container)
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
    
template<template <typename> class Container, class T>
void SceneNode::GetChildNodes(Container<T> & container)
{    
    Vector<SceneNode*>::const_iterator end = children.end();
    for (Vector<SceneNode*>::iterator t = children.begin(); t != end; ++t)
    {
        SceneNode* obj = *t;
        
        T res = dynamic_cast<T> (obj);
        if (res)
            container.push_back(res);
        
        obj->GetChildNodes(container);
    }	
}


};

#endif // __DAVAENGINE_SCENENODE_H__





