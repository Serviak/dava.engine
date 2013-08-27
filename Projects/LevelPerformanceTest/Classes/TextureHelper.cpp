#include "TextureHelper.h"

#include "Render/LibPVRHelper.h"
#include "Render/TextureDescriptor.h"

using namespace DAVA;

uint32 TextureHelper::GetSceneTextureMemory(DAVA::Scene *scene)
{
	return EnumerateSceneTextures(scene);
}

DAVA::uint32 TextureHelper::GetSceneTextureFilesSize(DAVA::Scene* scene)
{
	return EnumerateSceneTexturesFileSize(scene);
}



uint32 TextureHelper::EnumerateSceneTextures(DAVA::Scene* scene)
{
	Map<String, Texture *> textureMap;
	EnumerateTextures(scene, textureMap);

	uint32 sceneTextureMemory = 0;
	for(Map<String, Texture *>::const_iterator it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		Texture *t = it->second;
		//We need real info about textures size. In Editor on desktop pvr textures are decompressed to RGBA8888, so they have not real size.
        TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(t->GetPathname());
        if(descriptor)
        {
            FilePath imageFileName = GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, (eGPUFamily)descriptor->exportedAsGpuFamily);
            if(imageFileName.IsEqualToExtension(".pvr"))
            {
                sceneTextureMemory += LibPVRHelper::GetDataSize(imageFileName);
            }
            else if(imageFileName.IsEqualToExtension(".dds"))
            {
                sceneTextureMemory += LibDxtHelper::GetDataSize(imageFileName);
            }
            else
            {
                sceneTextureMemory += t->GetDataSize();
            }
            
            descriptor->Release();
        }
	}

	return sceneTextureMemory;
}

DAVA::uint32 TextureHelper::EnumerateSceneTexturesFileSize(DAVA::Scene* scene)
{
	Map<String, Texture *> textureMap;
	EnumerateTextures(scene, textureMap);

	uint32 sceneTextureFilesSize = 0;
	for(Map<String, Texture *>::const_iterator it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		Texture *t = it->second;

        TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(t->GetPathname());
        if(descriptor)
        {
            FilePath imageFileName = GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, (eGPUFamily)descriptor->exportedAsGpuFamily);

            File * textureFile = File::Create(imageFileName, File::OPEN | File::READ);
            if(textureFile)
            {
                sceneTextureFilesSize += textureFile->GetSize();
                
                textureFile->Release();
            }
            
            descriptor->Release();
        }
	}

	return sceneTextureFilesSize;
}

void TextureHelper::EnumerateTextures(DAVA::Entity *forNode, Map<String, Texture *> &textures)
{
	if(!forNode)  return;

	Vector<Entity *> nodes;
	forNode->GetChildNodes(nodes);

	nodes.push_back(forNode);

	for(int32 n = 0; n < (int32)nodes.size(); ++n)
	{
		RenderComponent *rc = static_cast<RenderComponent *>(nodes[n]->GetComponent(Component::RENDER_COMPONENT));
		if(!rc) continue;

		RenderObject *ro = rc->GetRenderObject();
		if(!ro) continue;

		uint32 count = ro->GetRenderBatchCount();
		for(uint32 b = 0; b < count; ++b)
		{
			RenderBatch *renderBatch = ro->GetRenderBatch(b);

			Material *material = renderBatch->GetMaterial();
			if(material)
			{
				for(int32 t = 0; t < Material::TEXTURE_COUNT; ++t)
				{
					CollectTexture(textures, material->GetTextureName((DAVA::Material::eTextureLevel)t), material->GetTexture((DAVA::Material::eTextureLevel)t));
				}
			}

			InstanceMaterialState *instanceMaterial = renderBatch->GetMaterialInstance();
			if(instanceMaterial)
			{
				CollectTexture(textures, instanceMaterial->GetLightmapName(), instanceMaterial->GetLightmap());
			}
		}

		Landscape *land = dynamic_cast<Landscape *>(ro);
		if(land)
		{
			CollectLandscapeTextures(textures, land);
		}
	}
}

void TextureHelper::CollectLandscapeTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, Landscape *forNode)
{
	for(int32 t = 0; t < Landscape::TEXTURE_COUNT; t++)
	{
		CollectTexture(textures, forNode->GetTextureName((Landscape::eTextureLevel)t), forNode->GetTexture((Landscape::eTextureLevel)t));
	}
}



void TextureHelper::CollectTexture(Map<String, Texture *> &textures, const FilePath &name, Texture *tex)
{
	if (!name.IsEmpty())
	{
		textures[name.GetAbsolutePathname()] = tex;
	}
}