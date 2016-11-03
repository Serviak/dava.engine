#include "Utils/Dump/SceneDumper.h"
#include "Utils/SceneUtils/SceneUtils.h"
#include "Classes/Qt/DataStructures/ProjectManagerData.h"

#include "FileSystem/KeyedArchive.h"
#include "Render/2D/Sprite.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"
#include "Render/TextureDescriptor.h"
#include "Render/Material/NMaterial.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

#include "Main/QtUtils.h"

#include "StringConstants.h"

using namespace DAVA;

SceneDumper::SceneLinks SceneDumper::DumpLinks(const FilePath& scenePath)
{
    SceneLinks links;
    SceneDumper dumper(scenePath);

    if (nullptr != dumper.scene)
    {
        dumper.DumpLinksRecursive(dumper.scene, links);
    }

    return links;
}

SceneDumper::SceneDumper(const FilePath& scenePath)
    : scenePathname(scenePath)
{
    scene = new Scene();
    if (SceneFileV2::ERROR_NO_ERROR != scene->LoadScene(scenePathname))
    {
        Logger::Error("[SceneDumper::SceneDumper] Can't open file %s", scenePathname.GetStringValue().c_str());
        SafeRelease(scene);
    }
}

SceneDumper::~SceneDumper()
{
    SafeRelease(scene);
}

void SceneDumper::DumpLinksRecursive(Entity* entity, SceneDumper::SceneLinks& links) const
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

void SceneDumper::DumpCustomProperties(DAVA::KeyedArchive* properties, SceneLinks& links) const
{
    if (nullptr == properties)
        return;

    auto SaveProp = [&properties, &links](const String& name)
    {
        auto str = properties->GetString(name);
        links.insert(str);
    };

    SaveProp(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
    SaveProp("touchdownEffect");

    //save custom colors
    String pathname = properties->GetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);
    if (!pathname.empty())
    {
        FilePath projectPath = ProjectManagerData::CreateProjectPathFromPath(scenePathname);
        links.emplace(projectPath + pathname);
    }
}

void SceneDumper::DumpRenderObject(DAVA::RenderObject* renderObject, SceneLinks& links) const
{
    if (nullptr == renderObject)
        return;

    Set<FilePath> descriptorPathnames;

    switch (renderObject->GetType())
    {
    case RenderObject::TYPE_LANDSCAPE:
    {
        Landscape* landscape = static_cast<Landscape*>(renderObject);
        links.insert(landscape->GetHeightmapPathname());
        break;
    }
    case RenderObject::TYPE_VEGETATION:
    {
        VegetationRenderObject* vegetation = static_cast<VegetationRenderObject*>(renderObject);
        links.insert(vegetation->GetHeightmapPath());
        links.insert(vegetation->GetCustomGeometryPath());

        Set<DataNode*> dataNodes;
        vegetation->GetDataNodes(dataNodes);
        for (DataNode* node : dataNodes)
        {
            NMaterial* material = dynamic_cast<NMaterial*>(node);
            if (material != nullptr)
            {
                DumpMaterial(material, links, descriptorPathnames);
            }
        }

        descriptorPathnames.insert(vegetation->GetLightmapPath());
        break;
    }

    default:
        break;
    }

    const uint32 count = renderObject->GetRenderBatchCount();
    for (uint32 rb = 0; rb < count; ++rb)
    {
        DAVA::RenderBatch* renderBatch = renderObject->GetRenderBatch(rb);
        DAVA::NMaterial* material = renderBatch->GetMaterial();
        DumpMaterial(material, links, descriptorPathnames);
    }

    // create pathnames for textures
    for (const auto& descriptorPath : descriptorPathnames)
    {
        DVASSERT(descriptorPath.IsEmpty() == false);

        links.insert(descriptorPath);

        std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPath));
        if (descriptor)
        {
            if (descriptor->IsCompressedFile())
            {
                Vector<FilePath> compressedTexureNames;
                descriptor->CreateLoadPathnamesForGPU(descriptor->gpu, compressedTexureNames);
                links.insert(compressedTexureNames.begin(), compressedTexureNames.end());
            }
            else
            {
                bool isCompressedSource = TextureDescriptor::IsSupportedCompressedFormat(descriptor->dataSettings.sourceFileFormat);
                if (descriptor->IsCubeMap() && !isCompressedSource)
                {
                    Vector<FilePath> faceNames;
                    descriptor->GetFacePathnames(faceNames);

                    links.insert(faceNames.cbegin(), faceNames.cend());
                }
                else
                {
                    links.insert(descriptor->GetSourceTexturePathname());
                }

                for (int gpu = 0; gpu < GPU_DEVICE_COUNT; ++gpu)
                {
                    const auto& compression = descriptor->compression[gpu];
                    if (compression.format != FORMAT_INVALID)
                    {
                        Vector<FilePath> compressedTexureNames;
                        descriptor->CreateLoadPathnamesForGPU(static_cast<eGPUFamily>(gpu), compressedTexureNames);
                        links.insert(compressedTexureNames.begin(), compressedTexureNames.end());
                    }
                }
            }
        }
    }
}

void SceneDumper::DumpMaterial(DAVA::NMaterial* material, SceneLinks& links, DAVA::Set<DAVA::FilePath>& descriptorPathnames) const
{
    //Enumerate textures from materials
    Set<MaterialTextureInfo*> materialTextures;

    while (nullptr != material)
    {
        material->CollectLocalTextures(materialTextures);
        material = material->GetParent();
    }

    // enumerate descriptor pathnames
    for (const MaterialTextureInfo* matTex : materialTextures)
    {
        descriptorPathnames.insert(matTex->path);
    }
}

void SceneDumper::DumpEffect(ParticleEffectComponent* effect, SceneLinks& links) const
{
    if (nullptr == effect)
    {
        return;
    }

    SceneLinks gfxFolders;

    const int32 emittersCount = effect->GetEmittersCount();
    for (int32 em = 0; em < emittersCount; ++em)
    {
        DumpEmitter(effect->GetEmitterInstance(em), links, gfxFolders);
    }

    for (auto& folder : gfxFolders)
    {
        FilePath flagsTXT = folder + "flags.txt";
        if (FileSystem::Instance()->Exists(flagsTXT))
        {
            links.insert(flagsTXT);
        }
    }
}

void SceneDumper::DumpEmitter(DAVA::ParticleEmitterInstance* instance, SceneLinks& links, SceneLinks& gfxFolders) const
{
    DVASSERT(nullptr != instance);

    auto emitter = instance->GetEmitter();

    links.insert(emitter->configPath);
    for (auto layer : emitter->layers)
    {
        DVASSERT(nullptr != layer);

        if (layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
        {
            ScopedPtr<ParticleEmitterInstance> instance(new ParticleEmitterInstance(nullptr, layer->innerEmitter, true));
            DumpEmitter(instance, links, gfxFolders);
        }
        else
        {
            Sprite* sprite = layer->sprite;
            if (nullptr == sprite)
            {
                continue;
            }

            FilePath psdPath = ReplaceInString(sprite->GetRelativePathname().GetAbsolutePathname(), "/Data/", "/DataSource/");
            psdPath.ReplaceExtension(".psd");
            links.insert(psdPath);

            gfxFolders.insert(psdPath.GetDirectory());
        }
    }
}
