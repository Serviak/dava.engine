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
#include "Scene3D/Scene.h"

#include "Render/Texture.h"
#include "Render/Material.h"
#include "Render/3D/StaticMesh.h"
#include "Render/3D/AnimatedMesh.h"
#include "Render/Image.h"
#include "Render/Highlevel/RenderSystem.h"


#include "Platform/SystemTimer.h"
#include "FileSystem/FileSystem.h"
#include "Debug/Stats.h"

#include "Scene3D/SceneNodeAnimationList.h"
#include "Scene3D/SceneFile.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/DataNode.h"
#include "Scene3D/ProxyNode.h"
#include "Scene3D/ShadowVolumeNode.h"
#include "Scene3D/ShadowRect.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/BVHierarchy.h"
#include "Scene3D/MeshInstanceNode.h"
#include "Scene3D/ImposterManager.h"
#include "Scene3D/ImposterNode.h"
#include "Render/Highlevel/LandscapeNode.h"

#include "Entity/SceneSystem.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Scene3D/Systems/RenderUpdateSystem.h"
#include "Scene3D/Systems/LodSystem.h"
#include "Scene3D/Systems/DebugRenderSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/ParticleEmitterSystem.h"
#include "Scene3D/Systems/ParticleEffectSystem.h"
#include "Scene3D/Systems/UpdatableSystem.h"
#include "Scene3D/Systems/DeleteSystem.h"
#include "Scene3D/Systems/LightUpdateSystem.h"

//#include "Entity/Entity.h"
//#include "Entity/EntityManager.h"
//#include "Entity/Components.h"
//
//#include "Entity/VisibilityAABBoxSystem.h"
//#include "Entity/MeshInstanceDrawSystem.h"
//#include "Entity/LandscapeGeometrySystem.h"

namespace DAVA 
{
    
REGISTER_CLASS(Scene);

Scene * Scene::activeScene = 0;
    
Scene::Scene()
	:   SceneNode()
    ,   currentCamera(0)
    ,   clipCamera(0)
//    ,   forceLodLayer(-1)
	,	shadowRect(0)
	,	imposterManager(0)
	,	entityManager(0)
	,	referenceNodeSuffixChanged(false)
{   
	SetActiveScene(this);

	bvHierarchy = new BVHierarchy();
	bvHierarchy->ChangeScene(this);

//	entityManager = new EntityManager();

	CreateComponents();
	CreateSystems();

    Stats::Instance()->RegisterEvent("Scene", "Time spend in scene processing");
    Stats::Instance()->RegisterEvent("Scene.Update", "Time spend in draw function");
    Stats::Instance()->RegisterEvent("Scene.Draw", "Time spend in draw function");
}

void Scene::CreateComponents()
{
    
}

void Scene::CreateSystems()
{
	eventSystem = new EventSystem();
	deleteSystem = new DeleteSystem();

    transformSystem = new TransformSystem();
    AddSystem(transformSystem, (1 << Component::TRANSFORM_COMPONENT));

    renderUpdateSystem = new RenderUpdateSystem();
    AddSystem(renderUpdateSystem, (1 << Component::TRANSFORM_COMPONENT) | (1 << Component::RENDER_COMPONENT));

	lodSystem = new LodSystem();
	AddSystem(lodSystem, (1 << Component::LOD_COMPONENT));

    debugRenderSystem = new DebugRenderSystem();
    AddSystem(debugRenderSystem, (1 << Component::DEBUG_RENDER_COMPONENT));

	particleEmitterSystem = new ParticleEmitterSystem();
	AddSystem(particleEmitterSystem, (1 << Component::PARTICLE_EMITTER_COMPONENT));

	particleEffectSystem = new ParticleEffectSystem();
	AddSystem(particleEffectSystem, (1 << Component::PARTICLE_EFFECT_COMPONENT));

	updatableSystem = new UpdatableSystem();
	AddSystem(updatableSystem, (1 << Component::UPDATABLE_COMPONENT));
    
    lightUpdateSystem = new LightUpdateSystem();
    AddSystem(lightUpdateSystem, (1 << Component::TRANSFORM_COMPONENT) | (1 << Component::LIGHT_COMPONENT));
}

Scene::~Scene()
{
	for (Vector<AnimatedMesh*>::iterator t = animatedMeshes.begin(); t != animatedMeshes.end(); ++t)
	{
		AnimatedMesh * obj = *t;
		obj->Release();
	}
	animatedMeshes.clear();
	
	for (Vector<Camera*>::iterator t = cameras.begin(); t != cameras.end(); ++t)
	{
		Camera * obj = *t;
		obj->Release();
	}
	cameras.clear();
    
    SafeRelease(currentCamera);
    SafeRelease(clipCamera);
    
    for (Map<String, ProxyNode*>::iterator it = rootNodes.begin(); it != rootNodes.end(); ++it)
    {
        SafeRelease(it->second);
    }
    rootNodes.clear();

    // Children should be removed first because they should unregister themselves in managers
	RemoveAllChildren();
    
	SafeRelease(imposterManager);
	SafeRelease(shadowRect);
	SafeRelease(bvHierarchy);

    transformSystem = 0;
    renderUpdateSystem = 0;
	lodSystem = 0;
    uint32 size = (uint32)systems.size();
    for (uint32 k = 0; k < size; ++k)
        SafeDelete(systems[k]);
    systems.clear();

	SafeDelete(eventSystem);
	SafeDelete(deleteSystem);
}

void Scene::RegisterNode(SceneNode * node)
{
    //Logger::Debug("Register node: %s %p %s", node->GetFullName().c_str(), node, node->GetClassName().c_str());
    LightNode * light = dynamic_cast<LightNode*>(node);
    if (light)
    {
        lights.insert(light);
    }

	ImposterNode * imposter = dynamic_cast<ImposterNode*>(node);
	if(imposter)
	{
		RegisterImposter(imposter);
	}

	if (bvHierarchy)
	{
		bvHierarchy->RegisterNode(node);
	}

	//MeshInstanceNode * meshInstance = dynamic_cast<MeshInstanceNode*>(node);
	//if(meshInstance)
	//{
	//	Entity * entity = entityManager->CreateEntity();
	//	node->entity = entity;
	//	node->entity->AddComponent(VisibilityAABBoxComponent::Get());
	//	node->entity->AddComponent(MeshInstanceComponent::Get());
	//	node->entity->AddComponent(TransformComponent::Get());

	//	//TODO: move Flush and data init to some Init() function
	//	entityManager->Flush();
	//	node->entity->SetData("flags", (uint32)0);
	//}

	//LandscapeNode * landscapeNode = dynamic_cast<LandscapeNode*>(node);
	//if(landscapeNode)
	//{
	//	Entity * entity = entityManager->CreateEntity();
	//	node->entity = entity;
	//	node->entity->AddComponent(LandscapeGeometryComponent::Get());
	//	entityManager->Flush();
	//	node->entity->SetData("landscapeNode", landscapeNode);
	//}
    
//    Drawable * drawable = dynamic_cast<Drawable*>(node);
//    
//    if (drawable)
//    {
//        drawArray.push_back(drawable);
//    }
    
    uint32 systemsCount = systems.size();
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        uint32 requiredComponents = systems[k]->GetRequiredComponents();
        bool needAdd = ((requiredComponents & node->componentFlags) == requiredComponents);
        
        if (needAdd)
            systems[k]->AddEntity(node);
    }
}

void Scene::UnregisterNode(SceneNode * node)
{
    uint32 systemsCount = systems.size();
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        uint32 requiredComponents = systems[k]->GetRequiredComponents();
        bool needRemove = ((requiredComponents & node->componentFlags) == requiredComponents);
        
        if (needRemove)
            systems[k]->RemoveEntity(node);
    }
    
    //Logger::Debug("Unregister node: %s %p %s", node->GetFullName().c_str(), node, node->GetClassName().c_str());

    LightNode * light = dynamic_cast<LightNode*>(node);
    if (light)
        lights.erase(light);

	ImposterNode * imposter = dynamic_cast<ImposterNode*>(node);
	if(imposter)
	{
		UnregisterImposter(imposter);
	}

	if (bvHierarchy)
	{
		bvHierarchy->UnregisterNode(node);
	}

	//if(node->entity)
	//{
	//	entityManager->DestroyEntity(node->entity);
	//}
    
//    // Remove drawable remove
//    Drawable * drawable = dynamic_cast<Drawable*>(node);
//    
//    if (drawable)
//    {
//        int32 size = (uint32)drawArray.size();
//        uint32 pos = 0;
//        for (pos = 0; pos < size; ++pos)
//        {
//            if (drawArray[pos] == drawable)
//            {
//                drawArray[pos] = drawArray[size - 1];
//                drawArray.pop_back();
//                break;
//            }
//        }
//    }
}
    
void Scene::AddComponent(SceneNode * entity, Component * component)
{
    uint32 oldComponentFlags = entity->componentFlags;
    entity->componentFlags |= (1 << component->GetType());
    uint32 systemsCount = systems.size();
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        uint32 requiredComponents = systems[k]->GetRequiredComponents();
        bool wasBefore = ((requiredComponents & oldComponentFlags) == requiredComponents);
        bool needAdd = ((requiredComponents & entity->componentFlags) == requiredComponents);
        
        if ((!wasBefore) && (needAdd))
            systems[k]->AddEntity(entity);
    }
}
    
void Scene::RemoveComponent(SceneNode * entity, Component * component)
{
    uint32 oldComponentFlags = entity->componentFlags;
    entity->componentFlags &= ~(1 << component->GetType());
    
    uint32 systemsCount = systems.size();
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        uint32 requiredComponents = systems[k]->GetRequiredComponents();
        bool wasBefore = ((requiredComponents & oldComponentFlags) == requiredComponents);
        bool shouldBeNow = ((requiredComponents & entity->componentFlags) == requiredComponents);
        
        if ((wasBefore) && (!shouldBeNow))
            systems[k]->RemoveEntity(entity);
    }
}
    
void Scene::ImmediateEvent(SceneNode * entity, uint32 componentType, uint32 event)
{
    uint32 systemsCount = systems.size();
    uint32 updatedComponentFlag = 1 << componentType;
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        uint32 requiredComponentFlags = systems[k]->GetRequiredComponents();
        uint32 componentsInEntity = entity->GetAvailableComponentFlags();
        
        if (((requiredComponentFlags & updatedComponentFlag) != 0) && ((requiredComponentFlags & componentsInEntity) == requiredComponentFlags))
        {
			eventSystem->NotifySystem(systems[k], entity, event);
        }
    }
}
    
void Scene::AddSystem(SceneSystem * sceneSystem, uint32 componentFlags)
{
    sceneSystem->SetRequiredComponents(componentFlags);
    systems.push_back(sceneSystem);
}
    
void Scene::RemoveSystem(SceneSystem * sceneSystem, uint32 componentFlags)
{
    DVASSERT(0 && "Need to write remove system function");
}

Scene * Scene::GetScene()
{
    return this;
}
    
void Scene::AddAnimatedMesh(AnimatedMesh * mesh)
{
	if (mesh)
	{
		mesh->Retain();
		animatedMeshes.push_back(mesh);
	}	
}

void Scene::RemoveAnimatedMesh(AnimatedMesh * mesh)
{
	
}

AnimatedMesh * Scene::GetAnimatedMesh(int32 index)
{
	return animatedMeshes[index];
}
	
void Scene::AddAnimation(SceneNodeAnimationList * animation)
{
	if (animation)
	{
		animation->Retain();
		animations.push_back(animation);
	}
}

SceneNodeAnimationList * Scene::GetAnimation(int32 index)
{
	return animations[index];
}
	
SceneNodeAnimationList * Scene::GetAnimation(const String & name)
{
	int32 size = (int32)animations.size();
	for (int32 k = 0; k < size; ++k)
	{
		SceneNodeAnimationList * node = animations[k];
		if (node->GetName() == name)
			return node;
	}
	return 0;
}
	
	
	
void Scene::AddCamera(Camera * camera)
{
	if (camera)
	{
		camera->Retain();
		cameras.push_back(camera);
	}
}

Camera * Scene::GetCamera(int32 n)
{
	if (n >= 0 && n < (int32)cameras.size())
		return cameras[n];
	
	return NULL;
}


void Scene::AddRootNode(SceneNode *node, const String &rootNodePath)
{
    ProxyNode * proxyNode = new ProxyNode();
    proxyNode->SetNode(node);
    
    rootNodes[rootNodePath] = proxyNode;
    proxyNode->SetName(rootNodePath);
}

SceneNode *Scene::GetRootNode(const String &rootNodePath)
{
//    ProxyNode * proxyNode = dynamic_cast<ProxyNode*>(scenes->FindByName(rootNodePath));
//    if (proxyNode)
//    {
//        return proxyNode->GetNode();
//    }
//    
//    String ext = FileSystem::Instance()->GetExtension(rootNodePath);
//    if(ext == ".sce")
//    {
//        SceneFile *file = new SceneFile();
//        file->SetDebugLog(true);
//        file->LoadScene(rootNodePath, this);
//        SafeRelease(file);
//    }
//    else if(ext == ".sc2")
//    {
//        SceneFileV2 *file = new SceneFileV2();
//        file->EnableDebugLog(true);
//        file->LoadScene(rootNodePath.c_str(), this);
//        SafeRelease(file);
//    }
//
//    proxyNode = dynamic_cast<ProxyNode*>(scenes->FindByName(rootNodePath));
//    if (proxyNode)
//    {
//        return proxyNode->GetNode();
//    }
//    return 0;
    
	Map<String, ProxyNode*>::const_iterator it;
	it = rootNodes.find(rootNodePath);
	if (it != rootNodes.end())
	{
        ProxyNode * node = it->second;
		return node->GetNode();
	}
    
    String ext = FileSystem::Instance()->GetExtension(rootNodePath);
    if(ext == ".sce")
    {
        SceneFile *file = new SceneFile();
        file->SetDebugLog(true);
        file->LoadScene(rootNodePath, this);
        SafeRelease(file);
    }
    else if(ext == ".sc2")
    {
        uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
        SceneFileV2 *file = new SceneFileV2();
        file->EnableDebugLog(false);
        file->LoadScene(rootNodePath.c_str(), this);
        SafeRelease(file);
        uint64 deltaTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
        Logger::Info("[GETROOTNODE TIME] %dms (%ld)", deltaTime, deltaTime);
    }
    
	it = rootNodes.find(rootNodePath);
	if (it != rootNodes.end())
	{
        ProxyNode * node = it->second;
        int32 nowCount = node->GetNode()->GetChildrenCountRecursive();
		return node->GetNode();
	}
    return 0;
}

void Scene::ReleaseRootNode(const String &rootNodePath)
{
	Map<String, ProxyNode*>::iterator it;
	it = rootNodes.find(rootNodePath);
	if (it != rootNodes.end())
	{
        it->second->Release();
        rootNodes.erase(it);
	}
}
    
void Scene::ReleaseRootNode(SceneNode *nodeToRelease)
{
//	for (Map<String, SceneNode*>::iterator it = rootNodes.begin(); it != rootNodes.end(); ++it)
//	{
//        if (nodeToRelease == it->second) 
//        {
//            SceneNode * obj = it->second;
//            obj->Release();
//            rootNodes.erase(it);
//            return;
//        }
//	}
}
    
void Scene::SetupTestLighting()
{
#ifdef __DAVAENGINE_IPHONE__
//	glShadeModel(GL_SMOOTH);
//	// enable lighting
//	glEnable(GL_LIGHTING);
//	glEnable(GL_NORMALIZE);
//	
//	// deactivate all lights
//	for (int i=0; i<8; i++)  glDisable(GL_LIGHT0 + i);
//	
//	// ambiental light to nothing
//	GLfloat ambientalLight[]= {0.2f, 0.2f, 0.2f, 1.0f};
//	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientalLight);
//	
////	GLfloat light_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };  // delete
//	//GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//	
//	GLfloat light_diffuse[4];
//	light_diffuse[0]=1.0f;
//	light_diffuse[1]=1.0f;
//	light_diffuse[2]=1.0f;
//	light_diffuse[3]=1.0f;
//	
//	GLfloat lightPos[] = { 0.0f, 0.0f, 1.0f, 0.0f };
//	
//	// activate this light
//	glEnable(GL_LIGHT0);
//	
//	//always position 0,0,0 because light  is moved with transformations
//	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
//	
//	// colors 
//	glLightfv(GL_LIGHT0, GL_AMBIENT, light_diffuse); // now like diffuse color
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
//	glLightfv(GL_LIGHT0, GL_SPECULAR,light_specular);
//	
//	//specific values for this light
//	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1);
//	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0);
//	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0);
//	
//	//other values
//	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 30.0f);
//	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0f);
//	GLfloat spotdirection[] = { 0.0f, 0.0f, -1.0f, 0.0f }; // irrelevant for this light (I guess)
//	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotdirection); 
#endif
}
    
void Scene::Update(float timeElapsed)
{
	SetActiveScene(this);

    Stats::Instance()->BeginTimeMeasure("Scene.Update", this);
    uint64 time = SystemTimer::Instance()->AbsoluteMS();

    
	updatableSystem->UpdatePreTransform();

    transformSystem->Process();

	updatableSystem->UpdatePostTransform();

	lodSystem->SetCamera(currentCamera);
	lodSystem->Process();

	particleEffectSystem->Process();
	particleEmitterSystem->Process();
    
//	entityManager->Flush();

    // lights 
    flags &= ~SCENE_LIGHTS_MODIFIED;
    
	int32 size = (int32)animations.size();
	for (int32 animationIndex = 0; animationIndex < size; ++animationIndex)
	{
		SceneNodeAnimationList * anim = animations[animationIndex];
		anim->Update(timeElapsed);
	}

	referenceNodeSuffixChanged = false;
	
	size = (int32)animatedMeshes.size();
	for (int32 animatedMeshIndex = 0; animatedMeshIndex < size; ++animatedMeshIndex)
	{
		AnimatedMesh * mesh = animatedMeshes[animatedMeshIndex];
		mesh->Update(timeElapsed);
	}

	if(imposterManager)
	{
		imposterManager->Update(timeElapsed);
	}

	deleteSystem->Process();
    
    updateTime = SystemTimer::Instance()->AbsoluteMS() - time;
    Stats::Instance()->EndTimeMeasure("Scene.Update", this);
}		

void Scene::Draw()
{
    Stats::Instance()->BeginTimeMeasure("Scene.Draw", this);

    
    //Sprite * fboSprite = Sprite::CreateAsRenderTarget(512, 512, FORMAT_RGBA8888);
	//RenderManager::Instance()->SetRenderTarget(fboSprite);
	//RenderManager::Instance()->SetViewport(Rect(0, 0, 512, 512), false);
    nodeCounter = 0;
    uint64 time = SystemTimer::Instance()->AbsoluteMS();

	shadowVolumes.clear();
    
    //const GLenum discards[]  = {GL_DEPTH_ATTACHMENT, GL_COLOR_ATTACHMENT0};
    //RENDER_VERIFY(glDiscardFramebufferEXT(GL_FRAMEBUFFER,2,discards));
    //glDepthMask(GL_TRUE);
    //RENDER_VERIFY(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
    
    if(imposterManager)
	{
		//imposterManager->ProcessQueue();
	}
    
    RenderManager::Instance()->SetCullMode(FACE_BACK);
    RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
    RenderManager::Instance()->FlushState();
	RenderManager::Instance()->ClearDepthBuffer();
    //glDepthMask(GL_TRUE);
    //RENDER_VERIFY(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
    
	
    if (currentCamera)
    {
        currentCamera->Set();
    }
    
	//if(bvHierarchy)
    //    bvHierarchy->Cull();
	//VisibilityAABBoxSystem::Run(this);

	//entityManager->Dump();
    
//    uint32 size = (uint32)drawArray.size();
//    for (uint32 k = 0; k < size; ++k)
//    {
//        drawArray[k]->Draw();
//    }
    
    RenderSystem * renderSystem = RenderSystem::Instance();
    
    Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
    renderSystem->SetCamera(currentCamera);
    renderUpdateSystem->Process();
    renderSystem->Render();
    debugRenderSystem->SetCamera(currentCamera);
    debugRenderSystem->Process();

    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
    
    //SceneNode::Draw();
    
    if(imposterManager)
	{
		imposterManager->Draw();
	}
    
	//LandscapeGeometrySystem::Run(this);
	//MeshInstanceDrawSystem::Run(this);

	if(RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SHADOWVOLUME_DRAW) && shadowVolumes.size() > 0)
	{
		if(!shadowRect)
		{
			shadowRect = ShadowRect::Create();
		}

		//2nd pass
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_CULL);
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_DEPTH_WRITE);
		RenderManager::Instance()->AppendState(RenderStateBlock::STATE_BLEND);
		RenderManager::Instance()->SetBlendMode(BLEND_ZERO, BLEND_ONE);

		RenderManager::Instance()->ClearStencilBuffer(0);
		RenderManager::Instance()->AppendState(RenderStateBlock::STATE_STENCIL_TEST);
		
		RenderManager::State()->SetStencilFunc(FACE_FRONT_AND_BACK, CMP_ALWAYS);
		RenderManager::State()->SetStencilRef(1);
		RenderManager::State()->SetStencilMask(0xFFFFFFFF);

		RenderManager::State()->SetStencilPass(FACE_FRONT, STENCILOP_KEEP);
		RenderManager::State()->SetStencilFail(FACE_FRONT, STENCILOP_KEEP);
		RenderManager::State()->SetStencilZFail(FACE_FRONT, STENCILOP_DECR_WRAP);

		RenderManager::State()->SetStencilPass(FACE_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilFail(FACE_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilZFail(FACE_BACK, STENCILOP_INCR_WRAP);
		
		RenderManager::Instance()->FlushState();
		Vector<ShadowVolumeNode*>::iterator itEnd = shadowVolumes.end();
		for(Vector<ShadowVolumeNode*>::iterator it = shadowVolumes.begin(); it != itEnd; ++it)
		{
			(*it)->DrawShadow();
		}

		//3rd pass
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_CULL);
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_DEPTH_TEST);
		
		RenderManager::State()->SetStencilRef(0);
		RenderManager::State()->SetStencilFunc(FACE_FRONT_AND_BACK, CMP_NOTEQUAL);
		RenderManager::State()->SetStencilPass(FACE_FRONT_AND_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilFail(FACE_FRONT_AND_BACK, STENCILOP_KEEP);
		RenderManager::State()->SetStencilZFail(FACE_FRONT_AND_BACK, STENCILOP_KEEP);

		RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		shadowRect->Draw();
	}

	RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_2D_STATE_BLEND);
	drawTime = SystemTimer::Instance()->AbsoluteMS() - time;

    Stats::Instance()->EndTimeMeasure("Scene.Draw", this);
	//Image * image = Image::Create(512, 512, FORMAT_RGBA8888);
	//RENDER_VERIFY(glReadPixels(0, 0, 512, 512, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)image->data));
	//image->Save("img.png");
	//RenderManager::Instance()->RestoreRenderTarget();
}

	
void Scene::StopAllAnimations(bool recursive )
{
	int32 size = (int32)animations.size();
	for (int32 animationIndex = 0; animationIndex < size; ++animationIndex)
	{
		SceneNodeAnimationList * anim = animations[animationIndex];
		anim->StopAnimation();
	}
	SceneNode::StopAllAnimations(recursive);
}
    
    
void Scene::SetCurrentCamera(Camera * _camera)
{
    SafeRelease(currentCamera);
    currentCamera = SafeRetain(_camera);
    SafeRelease(clipCamera);
    clipCamera = SafeRetain(_camera);
}

Camera * Scene::GetCurrentCamera() const
{
    return currentCamera;
}

void Scene::SetClipCamera(Camera * _camera)
{
    SafeRelease(clipCamera);
    clipCamera = SafeRetain(_camera);
}

Camera * Scene::GetClipCamera() const
{
    return clipCamera;
}
 
//void Scene::SetForceLodLayer(int32 layer)
//{
//    forceLodLayer = layer;
//}
//int32 Scene::GetForceLodLayer()
//{
//    return forceLodLayer;
//}
//
//int32 Scene::RegisterLodLayer(float32 nearDistance, float32 farDistance)
//{
//    LodLayer newLevel;
//    newLevel.nearDistance = nearDistance;
//    newLevel.farDistance = farDistance;
//    newLevel.nearDistanceSq = nearDistance * nearDistance;
//    newLevel.farDistanceSq = farDistance * farDistance;
//    int i = 0;
//    
//    for (Vector<LodLayer>::iterator it = lodLayers.begin(); it < lodLayers.end(); it++)
//    {
//        if (nearDistance < it->nearDistance)
//        {
//            lodLayers.insert(it, newLevel);
//            return i;
//        }
//        i++;
//    }
//    
//    lodLayers.push_back(newLevel);
//    return i;
//}
//    
//void Scene::ReplaceLodLayer(int32 layerNum, float32 nearDistance, float32 farDistance)
//{
//    DVASSERT(layerNum < (int32)lodLayers.size());
//    
//    lodLayers[layerNum].nearDistance = nearDistance;
//    lodLayers[layerNum].farDistance = farDistance;
//    lodLayers[layerNum].nearDistanceSq = nearDistance * nearDistance;
//    lodLayers[layerNum].farDistanceSq = farDistance * farDistance;
//    
//    
////    LodLayer newLevel;
////    newLevel.nearDistance = nearDistance;
////    newLevel.farDistance = farDistance;
////    newLevel.nearDistanceSq = nearDistance * nearDistance;
////    newLevel.farDistanceSq = farDistance * farDistance;
////    int i = 0;
////    
////    for (Vector<LodLayer>::iterator it = lodLayers.begin(); it < lodLayers.end(); it++)
////    {
////        if (nearDistance < it->nearDistance)
////        {
////            lodLayers.insert(it, newLevel);
////            return i;
////        }
////        i++;
////    }
////    
////    lodLayers.push_back(newLevel);
////    return i;
//}
//    
    

void Scene::AddDrawTimeShadowVolume(ShadowVolumeNode * shadowVolume)
{
	shadowVolumes.push_back(shadowVolume);
}

    
void Scene::UpdateLights()
{
    
    
    
    
}
    
LightNode * Scene::GetNearestDynamicLight(LightNode::eType type, Vector3 position)
{
    switch(type)
    {
        case LightNode::TYPE_DIRECTIONAL:
            
            break;
            
        default:
            break;
    };
    
	float32 squareMinDistance = 10000000.0f;
	LightNode * nearestLight = 0;

	Set<LightNode*> & lights = GetLights();
	const Set<LightNode*>::iterator & endIt = lights.end();
	for (Set<LightNode*>::iterator it = lights.begin(); it != endIt; ++it)
	{
		LightNode * node = *it;
		if(node->IsDynamic())
		{
			const Vector3 & lightPosition = node->GetPosition();

			float32 squareDistanceToLight = (position - lightPosition).SquareLength();
			if (squareDistanceToLight < squareMinDistance)
			{
				squareMinDistance = squareDistanceToLight;
				nearestLight = node;
			}
		}
	}

	return nearestLight;
}

Set<LightNode*> & Scene::GetLights()
{
    return lights;
}

void Scene::RegisterImposter(ImposterNode * imposter)
{
	if(!imposterManager)
	{
		imposterManager = new ImposterManager(this);
	}
	
	imposterManager->Add(imposter);
}

void Scene::UnregisterImposter(ImposterNode * imposter)
{
	imposterManager->Remove(imposter);

	if(imposterManager->IsEmpty())
	{
		SafeRelease(imposterManager);
	}
}

void Scene::SetReferenceNodeSuffix(const String & suffix)
{
	referenceNodeSuffix = suffix;
	referenceNodeSuffixChanged = true;
}

const String & Scene::GetReferenceNodeSuffix()
{
	return referenceNodeSuffix;
}

bool Scene::IsReferenceNodeSuffixChanged()
{
	return referenceNodeSuffixChanged;
}

void Scene::SetActiveScene(Scene * scene)
{
	activeScene = scene;
}

Scene * Scene::GetActiveScene()
{
	return activeScene;
}

EventSystem * Scene::GetEventSystem()
{
	return eventSystem;
}

/*void Scene::Save(KeyedArchive * archive)
{
    // Perform refactoring and add Matrix4, Vector4 types to VariantType and KeyedArchive
    SceneNode::Save(archive);
    
    
    
    
    
}

void Scene::Load(KeyedArchive * archive)
{
    SceneNode::Load(archive);
}*/
    



};




