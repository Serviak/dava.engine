/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "CommandLine/Dump/SceneDumper.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/2D/Sprite.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"
#include "Render/TextureDescriptor.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

#include "Qt/Main/QtUtils.h"
#include "StringConstants.h"

using namespace DAVA;

SceneDumper::SceneLinks SceneDumper::DumpLinks(const FilePath &scenePath, Set<String> &errorLog)
{
	SceneLinks links;
	SceneDumper dumper(scenePath, errorLog);

	if (nullptr != dumper.scene)
	{
		dumper.DumpLinksRecursive(dumper.scene, links);
	}

	return links;
}

SceneDumper::SceneDumper(const FilePath &scenePath, Set<String> &errorLog)
{
	scene = new Scene();
	if (SceneFileV2::ERROR_NO_ERROR != scene->LoadScene(scenePath))
	{
		errorLog.insert(Format("[SceneDumper::SceneDumper] Can't open file %s", scenePath.GetStringValue().c_str()));
		SafeRelease(scene);
	}
}

SceneDumper::~SceneDumper()
{
	SafeRelease(scene);
}

void SceneDumper::DumpLinksRecursive(Entity *entity, SceneDumper::SceneLinks &links) const
{
	//Recursiveness
	const uint32 count = entity->GetChildrenCount();
	for (uint32 ch = 0; ch < count; ++ch)
	{
		DumpLinksRecursive(entity->GetChild(ch), links);
	}

	// Custom Property Links
	DumpCustomProperties(GetCustomPropertiesArchieve(entity), links);

	//Render object
	DumpRenderObject(GetRenderObject(entity), links);

	//Effects
	DumpEffect(GetEffectComponent(entity), links);
}

void SceneDumper::DumpCustomProperties(DAVA::KeyedArchive *properties, SceneLinks &links) const
{
	if (nullptr == properties) return;

	auto SaveProp = [&properties, &links](const String & name)
	{
		auto str = properties->GetString(name);
		links.insert(str);
	};

	SaveProp(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
	SaveProp(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);
	SaveProp("touchdownEffect");
}

void SceneDumper::DumpRenderObject(DAVA::RenderObject *renderObject, SceneLinks &links) const
{
	if (nullptr == renderObject) return;

	switch (renderObject->GetType())
	{
		case RenderObject::TYPE_LANDSCAPE:
		{
			Landscape *landscape = static_cast<Landscape *> (renderObject);
			links.insert(landscape->GetHeightmapPathname());
			break;
		}
		case RenderObject::TYPE_PARTICLE_EMTITTER:
		{

			break;
		}
		case RenderObject::TYPE_VEGETATION:
		{
			VegetationRenderObject *vegetation = static_cast<VegetationRenderObject *>(renderObject);
			links.insert(vegetation->GetHeightmapPath());
			links.insert(vegetation->GetLightmapPath());
			links.insert(vegetation->GetTextureSheetPath());
			links.insert(vegetation->GetVegetationTexture());
			break;
		}

		default:
			break;
	}

	const uint32 count = renderObject->GetRenderBatchCount();
	for (uint32 rb = 0; rb < count; ++rb)
	{
		auto renderBatch = renderObject->GetRenderBatch(rb);
		auto material = renderBatch->GetMaterial();

		uint32 texCount = material->GetTextureCount();
		for (uint32 t = 0; t < texCount; ++t)
		{
			const auto & descriptorPath = material->GetTexturePath(t);
			links.insert(descriptorPath);

			//TODO: create image texture path
			TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(descriptorPath);
			links.insert(descriptor->GetSourceTexturePathname());

			for (int gpu = 0; gpu < GPU_DEVICE_COUNT; ++gpu)
			{
				links.insert(descriptor->CreatePathnameForGPU(static_cast<eGPUFamily>(gpu)));
			}

			delete descriptor;
		}
	}
}

void SceneDumper::DumpEffect(ParticleEffectComponent *effect, SceneLinks &links) const
{
	if (nullptr == effect) return;

	SceneLinks gfxFolders;

	const int32 emittersCount = effect->GetEmittersCount();
	for (int32 em = 0; em < emittersCount; ++em)
	{
		DumpEmitter(effect->GetEmitter(em), links, gfxFolders);
	}

	for (auto & folder : gfxFolders)
	{
		FilePath flagsTXT = folder + "flags.txt";
		if (flagsTXT.Exists())
		{
			links.insert(flagsTXT);
		}
	}
}

void SceneDumper::DumpEmitter(DAVA::ParticleEmitter *emitter, SceneLinks &links, SceneLinks &gfxFolders) const
{
	links.insert(emitter->configPath);

	const Vector<ParticleLayer*> &layers = emitter->layers;

	const uint32 count = (uint32)layers.size();
	for (uint32 i = 0; i < count; ++i)
	{
		if (layers[i]->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
		{
			DumpEmitter(layers[i]->innerEmitter, links, gfxFolders);
		}
		else
		{
			Sprite *sprite = layers[i]->sprite;
			if (nullptr == sprite) continue;

			FilePath psdPath = ReplaceInString(sprite->GetRelativePathname().GetAbsolutePathname(), "/Data/", "/DataSource/");
			psdPath.ReplaceExtension(".psd");
			links.insert(psdPath);

			gfxFolders.insert(psdPath.GetDirectory());
		}
	}
}
