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
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/SceneNode.h"
#include "Scene3D/MeshInstanceNode.h"
#include "Render/Texture.h"
#include "Render/Material.h"
#include "Render/3D/AnimatedMesh.h"
#include "Scene3D/PathManip.h"
#include "Scene3D/SkeletonNode.h"
#include "Scene3D/BoneNode.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Mesh.h"

#include "Scene3D/SceneNodeAnimationList.h"
#include "Scene3D/ReferenceNode.h"
#include "Scene3D/LodNode.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/ParticleEmitterNode.h"
#include "Scene3D/ParticleEffectNode.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/ParticleEmitterComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/LightComponent.h"

#include "Utils/StringFormat.h"
#include "FileSystem/FileSystem.h"
#include "Base/ObjectFactory.h"
#include "Base/TemplateHelpers.h"
#include "Render/Highlevel/LandscapeNode.h"

namespace DAVA
{
    
SceneFileV2::SceneFileV2()
{
    isDebugLogEnabled = false;
    isSaveForGame = false;
    lastError = ERROR_NO_ERROR;
}

SceneFileV2::~SceneFileV2()
{
}
    
const String & SceneFileV2::GetScenePath()
{
    return rootNodePath;
}
    
const String & SceneFileV2::GetSceneFilename()
{
    return rootNodeName;
}
    
static void replace(std::string & repString,const std::string & needle, const std::string & s)
{
    std::string::size_type lastpos = 0, thispos;
    while ((thispos = repString.find(needle, lastpos)) != std::string::npos)
    {
        repString.replace(thispos, needle.length(), s);
        lastpos = thispos + 1;
    }
}
    
void SceneFileV2::EnableSaveForGame(bool _isSaveForGame)
{
    isSaveForGame = _isSaveForGame;
}

String SceneFileV2::AbsoluteToRelative(const String & absolutePathname)
{
    String result = FileSystem::GetCanonicalPath(absolutePathname);
    
    if (isSaveForGame)
    {
        size_t pos = result.find("DataSource");
        if (pos != result.npos)
        {
            result.replace(pos, strlen("DataSource"), "Data");
        }
    }

    result = FileSystem::AbsoluteToRelativePath(GetScenePath(), result);
    return result;
}
    
String SceneFileV2::RelativeToAbsolute(const String & relativePathname)
{
    String result;
    result = GetScenePath() + relativePathname;
    result = FileSystem::GetCanonicalPath(result);
    return result;
}
    
void SceneFileV2::EnableDebugLog(bool _isDebugLogEnabled)
{
    isDebugLogEnabled = _isDebugLogEnabled;
}

bool SceneFileV2::DebugLogEnabled()
{
    return isDebugLogEnabled;
}
    
Material * SceneFileV2::GetMaterial(int32 index)
{
    return materials[index];
}
    
StaticMesh * SceneFileV2::GetStaticMesh(int32 index)
{
    return staticMeshes[index];
}

DataNode * SceneFileV2::GetNodeByPointer(uint64 pointer)
{
    Map<uint64, DataNode*>::iterator it = dataNodes.find(pointer);
    if (it != dataNodes.end())
    {
        return it->second;
    }
    return 0;
}

int32 SceneFileV2::GetVersion()
{
    return header.version;
}
    
void SceneFileV2::SetError(eError error)
{
    lastError = error;
}

SceneFileV2::eError SceneFileV2::GetError()
{
    return lastError;
}


SceneFileV2::eError SceneFileV2::SaveScene(const String & filename, DAVA::Scene *_scene)
{
    File * file = File::Create(filename, File::CREATE | File::WRITE);
    if (!file)
    {
        Logger::Error("SceneFileV2::SaveScene failed to create file: %s", filename.c_str());
        SetError(ERROR_FAILED_TO_CREATE_FILE);
        return GetError();
    }
    
    rootNodePathName = FileSystem::GetCanonicalPath(filename);
    FileSystem::Instance()->SplitPath(rootNodePathName, rootNodePath, rootNodeName);

    // save header
    header.signature[0] = 'S';
    header.signature[1] = 'F';
    header.signature[2] = 'V';
    header.signature[3] = '2';
    
    header.version = 5;
    header.nodeCount = _scene->GetChildrenCount();
    
    file->Write(&header, sizeof(Header));
    
    // save data objects
    if(isDebugLogEnabled)
    {
        Logger::Debug("+ save data objects");
        Logger::Debug("- save file path: %s", rootNodePath.c_str());
    }
    
//    // Process file paths
//    for (int32 mi = 0; mi < _scene->GetMaterials()->GetChildrenCount(); ++mi)
//    {
//        Material * material = dynamic_cast<Material*>(_scene->GetMaterials()->GetChild(mi));
//        for (int k = 0; k < Material::TEXTURE_COUNT; ++k)
//        {
//            if (material->names[k].length() > 0)
//            {
//                replace(material->names[k], rootNodePath, String(""));
//                Logger::Debug("- preprocess mat path: %s rpn: %s", material->names[k].c_str(), material->textures[k]->relativePathname.c_str());
//            }
//        }   
//    }
    
//    SaveDataHierarchy(_scene->GetMaterials(), file, 1);
//    SaveDataHierarchy(_scene->GetStaticMeshes(), file, 1);

    List<DataNode*> nodes;
    _scene->GetDataNodes(nodes);
    int32 dataNodesCount = (int32)nodes.size();
    file->Write(&dataNodesCount, sizeof(int32));
    for (List<DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        SaveDataNode(*it, file);
    
    // save hierarchy
    if(isDebugLogEnabled)
        Logger::Debug("+ save hierarchy");

    for (int ci = 0; ci < header.nodeCount; ++ci)
    {
        if (!SaveHierarchy(_scene->GetChild(ci), file, 1))
        {
            Logger::Error("SceneFileV2::SaveScene failed to save hierarchy file: %s", filename.c_str());
            SafeRelease(file);
            return GetError();
        }
    }
    
    SafeRelease(file);
    return GetError();
};	
    
SceneFileV2::eError SceneFileV2::LoadScene(const String & filename, Scene * _scene)
{
    File * file = File::Create(filename, File::OPEN | File::READ);
    if (!file)
    {
        Logger::Error("SceneFileV2::LoadScene failed to create file: %s", filename.c_str());
        SetError(ERROR_FAILED_TO_CREATE_FILE);
        return GetError();
    }   

    scene = _scene;
    rootNodePathName = FileSystem::GetCanonicalPath(filename);
    FileSystem::Instance()->SplitPath(rootNodePathName, rootNodePath, rootNodeName);

    file->Read(&header, sizeof(Header));
    int requiredVersion = 3;
    if (    (header.signature[0] != 'S') 
        ||  (header.signature[1] != 'F') 
        ||  (header.signature[2] != 'V') 
        ||  (header.signature[3] != '2'))
    {
        Logger::Error("SceneFileV2::LoadScene header version is wrong: %d, required: %d", header.version, requiredVersion);
        
        SafeRelease(file);
        SetError(ERROR_VERSION_IS_TOO_OLD);
        return GetError();
    }
    
    if(isDebugLogEnabled)
        Logger::Debug("+ load data objects");

    if (GetVersion() >= 2)
    {
        int32 dataNodeCount = 0;
        file->Read(&dataNodeCount, sizeof(int32));
        
        for (int k = 0; k < dataNodeCount; ++k)
            LoadDataNode(0, file);
    }
    
    if(isDebugLogEnabled)
        Logger::Debug("+ load hierarchy");
        
    SceneNode * rootNode = new SceneNode();
    rootNode->SetName(rootNodeName);
	rootNode->SetScene(_scene);
    for (int ci = 0; ci < header.nodeCount; ++ci)
    {
        LoadHierarchy(_scene, rootNode, file, 1);
    }
    
    OptimizeScene(rootNode);
    
	rootNode->SceneDidLoaded();
    
    if (GetError() == ERROR_NO_ERROR)
    {
        // TODO: Check do we need to releae root node here
        _scene->AddRootNode(rootNode, rootNodePathName);
    }
    else
    {
        SafeRelease(rootNode);
    }
    
    for (size_t mi = 0; mi < materials.size(); ++mi)
    {
        SafeRelease(materials[mi]);
    }
    materials.clear();
    
    for (size_t mi = 0; mi < staticMeshes.size(); ++mi)
    {
        SafeRelease(staticMeshes[mi]);
    }
    staticMeshes.clear();
    
    for (Map<uint64, DataNode*>::iterator it = dataNodes.begin(); it != dataNodes.end(); ++it)
    {
        SafeRelease(it->second);
    }
    dataNodes.clear();
    
    SafeRelease(rootNode);
    SafeRelease(file);
    return GetError();
}

bool SceneFileV2::SaveDataNode(DataNode * node, File * file)
{
    KeyedArchive * archive = new KeyedArchive();
    if (isDebugLogEnabled)
        Logger::Debug("- %s(%s)", node->GetName().c_str(), node->GetClassName().c_str());
    
    
    node->Save(archive, this);  
    archive->SetInt32("#childrenCount", node->GetChildrenCount());
    archive->Save(file);
    
    for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
    {
        DataNode * child = node->GetChild(ci);
        SaveDataNode(child, file);
    }
    
    SafeRelease(archive);
    return true;
}
    
void SceneFileV2::LoadDataNode(DataNode * parent, File * file)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    
    String name = archive->GetString("##name");
    DataNode * node = dynamic_cast<DataNode *>(ObjectFactory::Instance()->New(name));
    
    if (node)
    {
        if (node->GetClassName() == "DataNode")
        {
            SafeRelease(node);
            return;
        }   
        node->SetScene(scene);
        
        if (isDebugLogEnabled)
        {
            String name = archive->GetString("name");
            Logger::Debug("- %s(%s)", name.c_str(), node->GetClassName().c_str());
        }
        node->Load(archive, this);
        AddToNodeMap(node);
        
        if (parent)
            parent->AddNode(node);
        
        int32 childrenCount = archive->GetInt32("#childrenCount", 0);
        for (int ci = 0; ci < childrenCount; ++ci)
        {
            LoadDataNode(node, file);
        }
        
        SafeRelease(node);
    }
    SafeRelease(archive);
}

bool SceneFileV2::SaveDataHierarchy(DataNode * node, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    if (isDebugLogEnabled)
        Logger::Debug("%s %s(%s)", GetIndentString('-', level), node->GetName().c_str(), node->GetClassName().c_str());

    node->Save(archive, this);    
    
    
    archive->SetInt32("#childrenCount", node->GetChildrenCount());
    archive->Save(file);
    
	for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
	{
		DataNode * child = node->GetChild(ci);
		SaveDataHierarchy(child, file, level + 1);
	}
    
    SafeRelease(archive);
    return true;
}

void SceneFileV2::LoadDataHierarchy(Scene * scene, DataNode * root, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    
    // DataNode * node = dynamic_cast<DataNode*>(BaseObject::LoadFromArchive(archive));
    
    String name = archive->GetString("##name");
    DataNode * node = dynamic_cast<DataNode *>(ObjectFactory::Instance()->New(name));

    if (node)
    {
        if (node->GetClassName() == "DataNode")
        {
            SafeRelease(node);
            node = SafeRetain(root); // retain root here because we release it at the end
        }  
        
        node->SetScene(scene);
        
        // TODO: Rethink here
        Material * material = dynamic_cast<Material*>(node);
        if (material)
        {
            materials.push_back(SafeRetain(material));
        }
        StaticMesh * staticMesh = dynamic_cast<StaticMesh*>(node);
        if (staticMesh)
        {
            staticMeshes.push_back(SafeRetain(staticMesh));
        }
        if (isDebugLogEnabled)
        {
            String name = archive->GetString("name");
            Logger::Debug("%s %s(%s)", GetIndentString('-', level), name.c_str(), node->GetClassName().c_str());
        }
        node->Load(archive, this);
        
        
        AddToNodeMap(node);
        
        if (node != root)
            root->AddNode(node);
        
        int32 childrenCount = archive->GetInt32("#childrenCount", 0);
        for (int ci = 0; ci < childrenCount; ++ci)
        {
            LoadDataHierarchy(scene, node, file, level + 1);
        }
        SafeRelease(node);
    }
    
    SafeRelease(archive);
}
    
void SceneFileV2::AddToNodeMap(DataNode * node)
{
    uint64 ptr = node->GetPreviousPointer();
    
    if(isDebugLogEnabled)
        Logger::Debug("* add ptr: %llx class: %s(%s)", ptr, node->GetName().c_str(), node->GetClassName().c_str());
    
    dataNodes[ptr] = SafeRetain(node);
}
    
bool SceneFileV2::SaveHierarchy(SceneNode * node, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    if (isDebugLogEnabled)
        Logger::Debug("%s %s(%s) %d", GetIndentString('-', level), node->GetName().c_str(), node->GetClassName().c_str(), node->GetChildrenCount());
    node->Save(archive, this);    
	ReferenceNode * ref = dynamic_cast<ReferenceNode*>(node);
    
	if(!ref)
	{
		archive->SetInt32("#childrenCount", node->GetChildrenCount());
	}
 
    archive->Save(file);

	if(!ref)
	{
		for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
		{
			SceneNode * child = node->GetChild(ci);
			SaveHierarchy(child, file, level + 1);
		}
	}
    
    SafeRelease(archive);
    return true;
}

void SceneFileV2::LoadHierarchy(Scene * scene, SceneNode * parent, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    //SceneNode * node = dynamic_cast<SceneNode*>(BaseObject::LoadFromArchive(archive));
    
    String name = archive->GetString("##name");
    BaseObject * baseObject = 0;
    SceneNode * node = 0;
    
    bool skipNode = false;

    
    
    
    if (name == "LandscapeNode")
    {
        node = new SceneNode();
        baseObject = node;

        node->SetScene(scene);
        node->Load(archive, this);
        
        LandscapeNode * landscapeRenderObject = new LandscapeNode();
        landscapeRenderObject->Load(archive, this);
        
        node->AddComponent(new RenderComponent(landscapeRenderObject));

        parent->AddNode(node);
        
        SafeRelease(landscapeRenderObject);
        // Elegant fix became part of architecture....
        skipNode = true;
    }else if (name == "Camera")
    {
        node = new SceneNode();
        baseObject = node;
        
        node->SetScene(scene);
        node->Load(archive, this);
        
        Camera * cameraObject = new Camera();
        cameraObject->Load(archive, this);
        
        node->AddComponent(new CameraComponent(cameraObject));
        parent->AddNode(node);
        
        SafeRelease(cameraObject);
        skipNode = true;
    }else if (name == "LightNode")
    {
        node = new SceneNode();
        baseObject = node;
        
        node->SetScene(scene);
        node->Load(archive, this);
        
        LightNode * light = new LightNode();
        light->Load(archive, this);
        
        node->AddComponent(new LightComponent(light));
        parent->AddNode(node);
        
        SafeRelease(light);
        skipNode = true;
    }else
    {
        baseObject = ObjectFactory::Instance()->New(name);
        node = dynamic_cast<SceneNode*>(baseObject);
    }

	//TODO: refactor this elegant fix
	if(!node) //in case if editor class is loading in non-editor sprsoject
	{
		node = new SceneNode();
		skipNode = true;
	}

    //if(node)
    {
        if (isDebugLogEnabled)
        {
            String name = archive->GetString("name");
            Logger::Debug("%s %s(%s)", GetIndentString('-', level), name.c_str(), node->GetClassName().c_str());
        }

		if(!skipNode)
		{
			node->SetScene(scene);
			node->Load(archive, this);
            
            //ReplaceNodeAfterLoad(&node);
            
			parent->AddNode(node);
		}
        
		int32 childrenCount = archive->GetInt32("#childrenCount", 0);

		for (int ci = 0; ci < childrenCount; ++ci)
		{
			LoadHierarchy(scene, node, file, level + 1);
		}
        SafeRelease(node);
    }
    
    SafeRelease(archive);
}
    
bool SceneFileV2::RemoveEmptySceneNodes(DAVA::SceneNode * currentNode)
{
    for (int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
    {
        SceneNode * childNode = currentNode->GetChild(c);
        bool dec = RemoveEmptySceneNodes(childNode);
        if(dec)c--;
    }
    if ((currentNode->GetChildrenCount() == 0) && (typeid(*currentNode) == typeid(SceneNode)))
    {
        KeyedArchive *customProperties = currentNode->GetCustomProperties();
        bool doNotRemove = customProperties && customProperties->IsKeyExists("editor.donotremove");
        
        uint32 componentCount = currentNode->GetComponentCount();
        if (componentCount != 0)
        {
            doNotRemove = true;
        }
        
        if (!doNotRemove)
        {
            SceneNode * parent  = currentNode->GetParent();
            if (parent)
            {
                parent->RemoveNode(currentNode);
                removedNodeCount++;
                return true;
            }
        }
    }
    return false;
}
    
bool SceneFileV2::RemoveEmptyHierarchy(SceneNode * currentNode)
{
    for (int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
    {
        SceneNode * childNode = currentNode->GetChild(c);
        bool dec = RemoveEmptyHierarchy(childNode);
        if(dec)c--;
    }
    
//    if (currentNode->GetName() == "back_plain02.sc2")
//    {
//        int32 k = 0;
//        k++;
//        Logger::Debug("found node: %s %p", currentNode->GetName().c_str(), currentNode);
//    }

    if ((currentNode->GetChildrenCount() == 1) && (typeid(*currentNode) == typeid(SceneNode)))
    {
        if (currentNode->GetFlags() & SceneNode::NODE_LOCAL_MATRIX_IDENTITY)
        {
            SceneNode * parent  = currentNode->GetParent();
            if (parent)
            {
                SceneNode * childNode = SafeRetain(currentNode->GetChild(0));
                String currentName = currentNode->GetName();
                
                Logger::Debug("remove node: %s %p", currentNode->GetName().c_str(), currentNode);
                parent->RemoveNode(currentNode);
                parent->AddNode(childNode);
                
                childNode->SetName(currentName);
                removedNodeCount++;
                SafeRelease(childNode);
                return true;
            }
            //RemoveEmptyHierarchy(childNode);
        }
    }
    return false;
}

    
bool SceneFileV2::ReplaceNodeAfterLoad(SceneNode ** node)
{
    MeshInstanceNode * oldMeshInstanceNode = dynamic_cast<MeshInstanceNode*>(*node);
    if (oldMeshInstanceNode)
    {
        SceneNode * newMeshInstanceNode = new SceneNode();
        oldMeshInstanceNode->SceneNode::Clone(newMeshInstanceNode);
        newMeshInstanceNode->AddComponent(oldMeshInstanceNode->GetComponent(Component::TRANSFORM_COMPONENT)->Clone());
        
        Vector<PolygonGroupWithMaterial*> polygroups = oldMeshInstanceNode->GetPolygonGroups();
        
        Mesh * mesh = new Mesh();
        
        for (uint32 k = 0; k < (uint32)polygroups.size(); ++k)
        {
            PolygonGroupWithMaterial * group = polygroups[k];
            mesh->AddPolygonGroup(group->GetPolygonGroup(), group->GetMaterial());
        }
        
        RenderComponent * renderComponent = new RenderComponent;
        renderComponent->SetRenderObject(mesh);
        newMeshInstanceNode->AddComponent(renderComponent);
        
        SceneNode * parent = oldMeshInstanceNode->GetParent();
		if(parent)
		{
			parent->AddNode(newMeshInstanceNode);
			parent->RemoveNode(oldMeshInstanceNode);
		}
		else
		{
			DVASSERT(0 && "How we appeared here");
		}
		newMeshInstanceNode->Release();
        return true;
    }

	LodNode * lod = dynamic_cast<LodNode*>(*node);
	if(lod)
	{
		SceneNode * newNode = new SceneNode();
		lod->SceneNode::Clone(newNode);
		SceneNode * parent = lod->GetParent();

		DVASSERT(parent);
		if(parent)
		{
			parent->AddNode(newNode);
			parent->RemoveNode(lod);
		}

		newNode->AddComponent(new LodComponent());
		LodComponent * lc = DynamicTypeCheck<LodComponent*>(newNode->GetComponent(Component::LOD_COMPONENT));

		for(int32 iLayer = 0; iLayer < LodComponent::MAX_LOD_LAYERS; ++iLayer)
		{
			lc->lodLayersArray[iLayer].distance = lod->GetLodLayerDistance(iLayer);
			lc->lodLayersArray[iLayer].nearDistance = lod->GetLodLayerNear(iLayer);
			lc->lodLayersArray[iLayer].nearDistanceSq = lod->GetLodLayerNearSquare(iLayer);
			lc->lodLayersArray[iLayer].farDistance = lod->GetLodLayerFar(iLayer);
			lc->lodLayersArray[iLayer].farDistanceSq = lod->GetLodLayerFarSquare(iLayer);
		}

		List<LodNode::LodData*> oldLodData;
		lod->GetLodData(oldLodData);
		for(List<LodNode::LodData*>::iterator it = oldLodData.begin(); it != oldLodData.end(); ++it)
		{
			LodNode::LodData * oldDataItem = *it;
			LodComponent::LodData newLodDataItem;
			newLodDataItem.indexes = oldDataItem->indexes;
			newLodDataItem.isDummy = oldDataItem->isDummy;
			newLodDataItem.layer = oldDataItem->layer;
			newLodDataItem.nodes = oldDataItem->nodes;

			lc->lodLayers.push_back(newLodDataItem);
		}

		newNode->GetScene()->transformSystem->ImmediateEvent(newNode, EventSystem::LOCAL_TRANSFORM_CHANGED);
		newNode->Release();
		return true;
	}

	ParticleEmitterNode * particleEmitterNode = dynamic_cast<ParticleEmitterNode*>(*node);
	if(particleEmitterNode)
	{
		SceneNode * newNode = new SceneNode();
		particleEmitterNode->SceneNode::Clone(newNode);
		SceneNode * parent = particleEmitterNode->GetParent();

		DVASSERT(parent);
		if(parent)
		{
			parent->AddNode(newNode);
			parent->RemoveNode(particleEmitterNode);
		}

		ParticleEmitter * emitter = particleEmitterNode->GetEmitter();
		ParticleEmitterComponent * particleComponent = new ParticleEmitterComponent();
		newNode->AddComponent(particleComponent);
		particleComponent->SetParticleEmitter(emitter);

		RenderComponent * renderComponent = new RenderComponent;
		renderComponent->SetRenderObject(emitter);
		
		newNode->AddComponent(renderComponent);
		newNode->Release();
		return true;
	}

	ParticleEffectNode * particleEffectNode = dynamic_cast<ParticleEffectNode*>(*node);
	if(particleEffectNode)
	{
		SceneNode * newNode = new SceneNode();
		particleEffectNode->SceneNode::Clone(newNode);
		SceneNode * parent = particleEffectNode->GetParent();

		DVASSERT(parent);
		if(parent)
		{
			parent->AddNode(newNode);
			parent->RemoveNode(particleEffectNode);
		}

		ParticleEffectComponent * effectComponent = new ParticleEffectComponent();
		newNode->AddComponent(effectComponent);
		newNode->Release();
		return true;
	}

	return false;
}

void SceneFileV2::ReplaceOldNodes(SceneNode * currentNode)
{
	for(int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
	{
		SceneNode * childNode = currentNode->GetChild(c);
		bool wasReplace = ReplaceNodeAfterLoad(&childNode);
		ReplaceOldNodes(childNode);
		if(wasReplace)
		{
			c--;
		}
	}
}
    
void SceneFileV2::OptimizeScene(SceneNode * rootNode)
{
    int32 beforeCount = rootNode->GetChildrenCountRecursive();
    removedNodeCount = 0;
    rootNode->BakeTransforms();
    
	//MERGE: commented
    //RemoveEmptySceneNodes(rootNode);
    //RemoveEmptyHierarchy(rootNode);
	ReplaceOldNodes(rootNode);
    
//    for (int32 k = 0; k < rootNode->GetChildrenCount(); ++k)
//    {
//        SceneNode * node = rootNode->GetChild(k);
//        if (node->GetName() == "instance_0")
//            node->SetName(rootNodeName);
//    }
    int32 nowCount = rootNode->GetChildrenCountRecursive();
    Logger::Debug("nodes removed: %d before: %d, now: %d, diff: %d", removedNodeCount, beforeCount, nowCount, beforeCount - nowCount);
}
 
};
