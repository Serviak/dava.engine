#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/SceneNode.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/LandscapeNode.h"
#include "Render/Highlevel/RenderObject.h"


namespace DAVA
{

RenderObject * GetRenerObject(SceneNode * fromEntity)
{
	RenderObject * object = 0;

	if(NULL != fromEntity)
	{
		RenderComponent * component = static_cast<RenderComponent*>(fromEntity->GetComponent(Component::RENDER_COMPONENT));
		if(component)
		{
			object = component->GetRenderObject();
		}
	}

	return object;
}

ParticleEmitter * GetEmitter(SceneNode * fromEntity)
{
	ParticleEmitter * emitter = 0;

	if(NULL != fromEntity)
	{
		RenderObject * object = GetRenerObject(fromEntity);
		if(object && object->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			emitter = static_cast<ParticleEmitter*>(object);
		}
	}

	return emitter;
}


Light * GetLight( SceneNode * fromEntity )
{
	if(NULL != fromEntity)
	{
		LightComponent * component = static_cast<LightComponent*>(fromEntity->GetComponent(Component::LIGHT_COMPONENT));
		if(component)
		{
			return component->GetLightObject();
		}
	}

	return NULL;
}

LandscapeNode * GetLandscape( SceneNode * fromEntity )
{
	if(NULL != fromEntity)
	{
		RenderObject * object = GetRenerObject(fromEntity);
		if(object && object->GetType() == RenderObject::TYPE_LANDSCAPE)
		{
			LandscapeNode *landscape = static_cast<LandscapeNode *>(object);
			return landscape;
		}
	}

	return NULL;
}

Camera * GetCamera(SceneNode * fromEntity)
{
	if(NULL != fromEntity)
	{
		CameraComponent *component = static_cast<CameraComponent *>(fromEntity->GetComponent(Component::CAMERA_COMPONENT));
		if(component)
		{
			return component->GetCamera();
		}
	}
    
    return NULL;
}
    
LodComponent * GetLodComponent(SceneNode *fromEntity)
{
    if(fromEntity)
    {
        return static_cast<LodComponent*>(fromEntity->GetComponent(Component::LOD_COMPONENT));
    }
    
    return NULL;
}


void RecursiveProcessMeshNode(SceneNode * curr, void * userData, void(*process)(SceneNode*, void *))
{
	RenderComponent * comp = (RenderComponent*)curr->GetComponent(Component::RENDER_COMPONENT);
	if (comp)
	{
		RenderObject * renderObject = comp->GetRenderObject();
		if (renderObject->GetType() == RenderObject::TYPE_MESH)
		{
			process(curr, userData);
		}
	}
	else
	{
		for (int32 i = 0; i < curr->GetChildrenCount(); i++)
			RecursiveProcessMeshNode(curr->GetChild(i), userData, process);
	}
}



void RecursiveProcessLodNode(SceneNode * curr, int32 lod, void * userData, void(*process)(SceneNode*, void*))
{
	LodComponent * lodComp = (LodComponent*)curr->GetComponent(Component::LOD_COMPONENT);
	if (lodComp)
	{
		Vector<LodComponent::LodData*> retLodLayers;
		lodComp->GetLodData(retLodLayers);
		for (Vector<LodComponent::LodData*>::iterator it = retLodLayers.begin(); it != retLodLayers.end(); ++it)
		{
			LodComponent::LodData * data = *it;
			if (data->layer == lod)
			{
				for (Vector<SceneNode*>::iterator i = data->nodes.begin(); i != data->nodes.end(); ++i)
				{
					process((*i), userData);
				}
				break;
			}
		}
	}
	else
	{
		for (int32 i = 0; i < curr->GetChildrenCount(); i++)
			RecursiveProcessLodNode(curr->GetChild(i), lod, userData, process);
	}
}


}
