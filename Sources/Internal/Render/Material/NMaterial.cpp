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


#include "Render/Material/NMaterial.h"

#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Scene3D/SceneFile/SerializationContext.h"

#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Material/FXCache.h"
#include "Render/Shader.h"
#include "Render/Texture.h"

#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "FileSystem/YamlParser.h"

namespace DAVA
{
const float32 NMaterial::DEFAULT_LIGHTMAP_SIZE = 16.0f;

struct MaterialPropertyBinding
{
    rhi::ShaderProp::Type type;
    uint32 reg;
    uint32 regCount;
    uint32 updateSemantic;
    NMaterialProperty* source;
    MaterialPropertyBinding(rhi::ShaderProp::Type type_, uint32 reg_, uint32 regCount_, uint32 updateSemantic_, NMaterialProperty* source_)
        : type(type_)
        , reg(reg_)
        , regCount(regCount_)
        , updateSemantic(updateSemantic_)
        , source(source_)
    {
    }
};

struct MaterialBufferBinding
{
    rhi::HConstBuffer constBuffer;
    Vector<MaterialPropertyBinding> propBindings;
    uint32 lastValidPropertySemantic = 0;
};

uint32 NMaterialProperty::globalPropertyUpdateSemanticCounter = 0;

RenderVariantInstance::RenderVariantInstance()
    : shader(nullptr)
{
}

RenderVariantInstance::~RenderVariantInstance()
{
    rhi::ReleaseDepthStencilState(depthState);
    rhi::ReleaseTextureSet(textureSet);
    rhi::ReleaseSamplerState(samplerState);
}

NMaterial::NMaterial()
    : localConstBuffers(16, nullptr)
    , renderVariants(4, nullptr)
{
    materialConfigs.resize(1); //at least one config to emulate regular work
}

NMaterial::MaterialConfig::MaterialConfig()
    : localProperties(16, nullptr)
    , localTextures(8, nullptr)
    , localFlags(16, 0)
{
}
NMaterial::MaterialConfig::~MaterialConfig()
{
    for (auto& prop : localProperties)
        SafeDelete(prop.second);
    for (auto& texInfo : localTextures)
    {
        SafeRelease(texInfo.second->texture);
        SafeDelete(texInfo.second);
    }
}

NMaterial::~NMaterial()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    SetParent(nullptr);
    DVASSERT(children.size() == 0); //as children reference parent in our material scheme, this should not be released while it has children

    for (auto& buffer : localConstBuffers)
    {
        rhi::DeleteConstBuffer(buffer.second->constBuffer);
        SafeDelete(buffer.second);
    }
    for (auto& variant : renderVariants)
        delete variant.second;
}

void NMaterial::BindParams(rhi::Packet& target)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    //Logger::Info( "bind-params" );
    DVASSERT(activeVariantInstance); //trying to bind material that was not staged to render
    DVASSERT(activeVariantInstance->shader); //should have returned false on PreBuild!
    DVASSERT(activeVariantInstance->shader->IsValid()); //should have returned false on PreBuild!
    /*set pipeline state*/
    target.renderPipelineState = activeVariantInstance->shader->GetPiplineState();
    target.depthStencilState = activeVariantInstance->depthState;
    target.samplerState = activeVariantInstance->samplerState;
    target.textureSet = activeVariantInstance->textureSet;
    target.cullMode = activeVariantInstance->cullMode;
    if (activeVariantInstance->wireFrame)
        target.options |= rhi::Packet::OPT_WIREFRAME;

    activeVariantInstance->shader->UpdateDynamicParams();
    /*update values in material const buffers*/
    for (auto& materialBufferBinding : activeVariantInstance->materialBufferBindings)
    {
        if (materialBufferBinding->lastValidPropertySemantic == NMaterialProperty::GetCurrentUpdateSemantic()) //prevent buffer update if nothing changed
            continue;
        //assume that if we have no property - we bind default value on buffer allocation step - no binding is created in that case
        for (auto& materialBinding : materialBufferBinding->propBindings)
        {
            DVASSERT(materialBinding.source)
            if (materialBinding.updateSemantic != materialBinding.source->updateSemantic)
            {
                //Logger::Info( " upd-prop " );
                if (materialBinding.type < rhi::ShaderProp::TYPE_FLOAT4)
                {
                    DVASSERT(materialBinding.source->arraySize == 1);
                    rhi::UpdateConstBuffer1fv(materialBufferBinding->constBuffer, materialBinding.reg, materialBinding.regCount, materialBinding.source->data.get(), ShaderDescriptor::CalculateDataSize(materialBinding.type, materialBinding.source->arraySize));
                }
                else
                {
                    DVASSERT(materialBinding.source->arraySize <= materialBinding.regCount);
                    rhi::UpdateConstBuffer4fv(materialBufferBinding->constBuffer, materialBinding.reg, materialBinding.source->data.get(), ShaderDescriptor::CalculateRegsCount(materialBinding.type, materialBinding.source->arraySize));
                }
                materialBinding.updateSemantic = materialBinding.source->updateSemantic;

#if defined(__DAVAENGINE_RENDERSTATS__)
                ++Renderer::GetRenderStats().materialParamBindCount;
#endif
            }
        }
        materialBufferBinding->lastValidPropertySemantic = NMaterialProperty::GetCurrentUpdateSemantic();
    }

    target.vertexConstCount = static_cast<uint32>(activeVariantInstance->vertexConstBuffers.size());
    target.fragmentConstCount = static_cast<uint32>(activeVariantInstance->fragmentConstBuffers.size());
    /*bind material const buffers*/
    for (size_t i = 0, sz = activeVariantInstance->vertexConstBuffers.size(); i < sz; ++i)
        target.vertexConst[i] = activeVariantInstance->vertexConstBuffers[i];
    for (size_t i = 0, sz = activeVariantInstance->fragmentConstBuffers.size(); i < sz; ++i)
        target.fragmentConst[i] = activeVariantInstance->fragmentConstBuffers[i];
}

uint32 NMaterial::GetRequiredVertexFormat()
{
    uint32 res = 0;
    for (auto& variant : renderVariants)
    {
        bool shaderValid = (nullptr != variant.second) && (variant.second->shader->IsValid());
        DVASSERT_MSG(shaderValid, "Shader is invalid. Check log for details.");

        if (shaderValid)
        {
            res |= variant.second->shader->GetRequiredVertexFormat();
        }
    }
    return res;
}

MaterialBufferBinding* NMaterial::GetConstBufferBinding(UniquePropertyLayout propertyLayout)
{
    MaterialBufferBinding* res = localConstBuffers.at(propertyLayout);
    if ((res == nullptr) && (parent != nullptr) && (!NeedLocalOverride(propertyLayout)))
        res = parent->GetConstBufferBinding(propertyLayout);

    return res;
}

NMaterialProperty* NMaterial::GetMaterialProperty(const FastName& propName)
{
    NMaterialProperty* res = materialConfigs[currConfig].localProperties.at(propName);
    if ((res == nullptr) && (parent != nullptr))
    {
        res = parent->GetMaterialProperty(propName);
    }
    return res;
}

Texture* NMaterial::GetEffectiveTexture(const FastName& slotName)
{
    MaterialTextureInfo* localInfo = materialConfigs[currConfig].localTextures.at(slotName);
    if (localInfo)
    {
        if (localInfo->texture == nullptr)
            localInfo->texture = Texture::CreateFromFile(localInfo->path, slotName);
        return localInfo->texture;
    }

    if (parent != nullptr)
    {
        return parent->GetEffectiveTexture(slotName);
    }
    return nullptr;
}

void NMaterial::CollectLocalTextures(Set<MaterialTextureInfo*>& collection) const
{
    for (const auto& config : materialConfigs)
    {
        for (const auto& lc : config.localTextures)
        {
            const auto& path = lc.second->path;
            if (!path.IsEmpty())
            {
                collection.emplace(lc.second);
            }
        }
    }
}

bool NMaterial::ContainsTexture(Texture* texture) const
{
    for (const auto& config : materialConfigs)
    {
        for (const auto& lc : config.localTextures)
        {
            if (lc.second->texture == texture)
                return true;
        }
    }

    return false;
}

const HashMap<FastName, MaterialTextureInfo*>& NMaterial::GetLocalTextures() const
{
    return materialConfigs[currConfig].localTextures;
}

void NMaterial::SetFXName(const FastName& fx)
{
    materialConfigs[currConfig].fxName = fx;
    InvalidateRenderVariants();
}

const FastName& NMaterial::GetEffectiveFXName() const
{
    if ((!materialConfigs[currConfig].fxName.IsValid()) && (parent != nullptr))
    {
        return parent->GetEffectiveFXName();
    }
    return materialConfigs[currConfig].fxName;
}

const FastName& NMaterial::GetLocalFXName() const
{
    return materialConfigs[currConfig].fxName;
}

bool NMaterial::HasLocalFXName() const
{
    return materialConfigs[currConfig].fxName.IsValid();
}

const FastName& NMaterial::GetQualityGroup()
{
    if ((!qualityGroup.IsValid()) && (parent != nullptr))
    {
        return parent->GetQualityGroup();
    }
    return qualityGroup;
}

void NMaterial::SetQualityGroup(const FastName& quality)
{
    qualityGroup = quality;
}

void NMaterial::AddProperty(const FastName& propName, const float32* propData, rhi::ShaderProp::Type type, uint32 arraySize)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(materialConfigs[currConfig].localProperties.at(propName) == nullptr);
    NMaterialProperty* prop = new NMaterialProperty();
    prop->name = propName;
    prop->type = type;
    prop->arraySize = arraySize;
    prop->data.reset(new float[ShaderDescriptor::CalculateDataSize(type, arraySize)]);
    prop->SetPropertyValue(propData);
    materialConfigs[currConfig].localProperties[propName] = prop;

    InvalidateBufferBindings();
}

void NMaterial::RemoveProperty(const FastName& propName)
{
    NMaterialProperty* prop = materialConfigs[currConfig].localProperties.at(propName);
    DVASSERT(prop != nullptr);
    materialConfigs[currConfig].localProperties.erase(propName);
    SafeDelete(prop);

    InvalidateBufferBindings();
}

void NMaterial::SetPropertyValue(const FastName& propName, const float32* propData)
{
    NMaterialProperty* prop = materialConfigs[currConfig].localProperties.at(propName);
    DVASSERT(prop != nullptr);
    prop->SetPropertyValue(propData);
}

bool NMaterial::HasLocalProperty(const FastName& propName)
{
    return materialConfigs[currConfig].localProperties.at(propName) != nullptr;
}

rhi::ShaderProp::Type NMaterial::GetLocalPropType(const FastName& propName)
{
    NMaterialProperty* prop = materialConfigs[currConfig].localProperties.at(propName);
    DVASSERT(prop != nullptr);
    return prop->type;
}

const float32* NMaterial::GetLocalPropValue(const FastName& propName)
{
    NMaterialProperty* prop = materialConfigs[currConfig].localProperties.at(propName);
    DVASSERT(prop != nullptr);
    return prop->data.get();
}

uint32 NMaterial::GetLocalPropArraySize(const FastName& propName)
{
    NMaterialProperty* prop = materialConfigs[currConfig].localProperties.at(propName);
    DVASSERT(prop != nullptr);
    return prop->arraySize;
}

const float32* NMaterial::GetEffectivePropValue(const FastName& propName)
{
    NMaterialProperty* prop = materialConfigs[currConfig].localProperties.at(propName);
    if (prop)
        return prop->data.get();
    if (parent)
        return parent->GetEffectivePropValue(propName);
    return nullptr;
}

void NMaterial::AddTexture(const FastName& slotName, Texture* texture)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(materialConfigs[currConfig].localTextures.at(slotName) == nullptr);
    MaterialTextureInfo* texInfo = new MaterialTextureInfo();
    texInfo->texture = SafeRetain(texture);
    texInfo->path = texture->GetPathname();
    materialConfigs[currConfig].localTextures[slotName] = texInfo;
    InvalidateTextureBindings();
}
void NMaterial::RemoveTexture(const FastName& slotName)
{
    MaterialTextureInfo* texInfo = materialConfigs[currConfig].localTextures.at(slotName);
    DVASSERT(texInfo != nullptr);
    materialConfigs[currConfig].localTextures.erase(slotName);
    SafeRelease(texInfo->texture);
    SafeDelete(texInfo);
    InvalidateTextureBindings();
}
void NMaterial::SetTexture(const FastName& slotName, Texture* texture)
{
    MaterialTextureInfo* texInfo = materialConfigs[currConfig].localTextures.at(slotName);
    DVASSERT(texture != nullptr); //use RemoveTexture to remove texture!
    DVASSERT(texInfo != nullptr); //use AddTexture to add texture!

    if (texInfo->texture != texture)
    {
        SafeRelease(texInfo->texture);
        texInfo->texture = SafeRetain(texture);
        texInfo->path = texture->GetPathname();
    }

    InvalidateTextureBindings();
}

bool NMaterial::HasLocalTexture(const FastName& slotName)
{
    return materialConfigs[currConfig].localTextures.find(slotName) != materialConfigs[currConfig].localTextures.end();
}
Texture* NMaterial::GetLocalTexture(const FastName& slotName)
{
    DVASSERT(HasLocalTexture(slotName));
    MaterialTextureInfo* texInfo = materialConfigs[currConfig].localTextures.at(slotName);
    if (texInfo->texture == nullptr)
        texInfo->texture = Texture::CreateFromFile(texInfo->path);
    return texInfo->texture;
}

void NMaterial::AddFlag(const FastName& flagName, int32 value)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(materialConfigs[currConfig].localFlags.find(flagName) == materialConfigs[currConfig].localFlags.end());
    materialConfigs[currConfig].localFlags[flagName] = value;
    InvalidateRenderVariants();
}
void NMaterial::RemoveFlag(const FastName& flagName)
{
    DVASSERT(materialConfigs[currConfig].localFlags.find(flagName) != materialConfigs[currConfig].localFlags.end());
    materialConfigs[currConfig].localFlags.erase(flagName);
    InvalidateRenderVariants();
}
void NMaterial::SetFlag(const FastName& flagName, int32 value)
{
    DVASSERT(materialConfigs[currConfig].localFlags.find(flagName) != materialConfigs[currConfig].localFlags.end());
    materialConfigs[currConfig].localFlags[flagName] = value;
    InvalidateRenderVariants();
}

int32 NMaterial::GetEffectiveFlagValue(const FastName& flagName)
{
    HashMap<FastName, int32>::iterator it = materialConfigs[currConfig].localFlags.find(flagName);
    if (it != materialConfigs[currConfig].localFlags.end())
        return it->second;
    else if (parent)
        return parent->GetEffectiveFlagValue(flagName);
    return 0;
}

int32 NMaterial::GetLocalFlagValue(const FastName& flagName)
{
    DVASSERT(materialConfigs[currConfig].localFlags.find(flagName) != materialConfigs[currConfig].localFlags.end());
    return materialConfigs[currConfig].localFlags[flagName];
}

bool NMaterial::HasLocalFlag(const FastName& flagName)
{
    return materialConfigs[currConfig].localFlags.find(flagName) != materialConfigs[currConfig].localFlags.end();
}

bool NMaterial::NeedLocalOverride(UniquePropertyLayout propertyLayout)
{
    for (auto& descr : ShaderDescriptor::GetProps(propertyLayout))
    {
        if (materialConfigs[currConfig].localProperties.at(descr.uid) != nullptr)
            return true;
    }
    return false;
}

void NMaterial::SetParent(NMaterial* _parent)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DVASSERT(_parent != this);

    if (parent == _parent)
        return;

    if (parent)
    {
        parent->RemoveChildMaterial(this);
        SafeRelease(parent);
    }

    parent = _parent;
    sortingKey = (uint32)((uint64)parent);

    if (parent)
    {
        SafeRetain(parent);
        parent->AddChildMaterial(this);
    }

    InvalidateRenderVariants();
}

NMaterial* NMaterial::GetParent()
{
    return parent;
}

const Vector<NMaterial*>& NMaterial::GetChildren() const
{
    return children;
}

void NMaterial::AddChildMaterial(NMaterial* material)
{
    DVASSERT(material);
    children.push_back(material);
}

void NMaterial::RemoveChildMaterial(NMaterial* material)
{
    bool res = FindAndRemoveExchangingWithLast(children, material);
    DVASSERT(res);
}

void NMaterial::InjectChildBuffer(UniquePropertyLayout propLayoutId, MaterialBufferBinding* buffer)
{
    if (parent && !NeedLocalOverride(propLayoutId))
        parent->InjectChildBuffer(propLayoutId, buffer);
    else
    {
        DVASSERT(localConstBuffers.at(propLayoutId) == nullptr);
        localConstBuffers[propLayoutId] = buffer;
    }
}

void NMaterial::ClearLocalBuffers()
{
    for (auto& buffer : localConstBuffers)
    {
        rhi::DeleteConstBuffer(buffer.second->constBuffer);
        SafeDelete(buffer.second);
    }
    for (auto& variant : renderVariants)
        variant.second->materialBufferBindings.clear();
    localConstBuffers.clear();
}

void NMaterial::InvalidateBufferBindings()
{
    ClearLocalBuffers(); //RHI_COMPLETE - as local buffers can have binding for this property now just clear them all, later rethink to erase just buffers containing this property
    needRebuildBindings = true;
    for (auto& child : children)
        child->InvalidateBufferBindings();
}

void NMaterial::InvalidateTextureBindings()
{
    // reset existing handle?
    needRebuildTextures = true;
    for (auto& child : children)
        child->InvalidateTextureBindings();
}

void NMaterial::InvalidateRenderVariants()
{
    // release existing descriptor?
    ClearLocalBuffers(); // to avoid using incorrect buffers in certain situations (e.g chaning parent)
    needRebuildVariants = true;
    for (auto& child : children)
        child->InvalidateRenderVariants();
}

void NMaterial::PreCacheFX()
{
    HashMap<FastName, int32> flags(16, 0);
    CollectMaterialFlags(flags);
    flags.erase(NMaterialFlagName::FLAG_ILLUMINATION_USED);
    flags.erase(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER);
    flags.erase(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER);
    FXCache::GetFXDescriptor(GetEffectiveFXName(), flags, QualitySettingsSystem::Instance()->GetCurMaterialQuality(GetQualityGroup()));
}

void NMaterial::PreCacheFXWithFlags(const HashMap<FastName, int32>& extraFlags, const FastName& extraFxName)
{
    HashMap<FastName, int32> flags(16, 0);
    CollectMaterialFlags(flags);
    flags.erase(NMaterialFlagName::FLAG_ILLUMINATION_USED);
    flags.erase(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER);
    flags.erase(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER);
    for (auto& it : extraFlags)
    {
        if (it.second == 0)
            flags.erase(it.first);
        else
            flags[it.first] = it.second;
    }
    FXCache::GetFXDescriptor(extraFxName.IsValid() ? extraFxName : GetEffectiveFXName(), flags, QualitySettingsSystem::Instance()->GetCurMaterialQuality(GetQualityGroup()));
}

void NMaterial::RebuildRenderVariants()
{
    HashMap<FastName, int32> flags(16, 0);
    CollectMaterialFlags(flags);
    flags.erase(NMaterialFlagName::FLAG_ILLUMINATION_USED);
    flags.erase(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER);
    flags.erase(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER);
    const FXDescriptor& fxDescr = FXCache::GetFXDescriptor(GetEffectiveFXName(), flags, QualitySettingsSystem::Instance()->GetCurMaterialQuality(GetQualityGroup()));

    if (fxDescr.renderPassDescriptors.size() == 0)
    {
        // dragon: because I'm fucking sick and tired of Render2D-init crashing (when I don't even need it)
        return;
    }

    /*at least in theory flag changes can lead to changes in number of render passes*/
    activeVariantInstance = nullptr;
    activeVariantName = FastName();
    for (auto& variant : renderVariants)
    {
        delete variant.second;
    }
    renderVariants.clear();

    for (auto& variantDescr : fxDescr.renderPassDescriptors)
    {
        RenderVariantInstance* variant = new RenderVariantInstance();
        variant->renderLayer = variantDescr.renderLayer;
        variant->depthState = rhi::AcquireDepthStencilState(variantDescr.depthStateDescriptor);
        variant->shader = variantDescr.shader;
        variant->cullMode = variantDescr.cullMode;
        variant->wireFrame = variantDescr.wireframe;
        renderVariants[variantDescr.passName] = variant;
    }

    ClearLocalBuffers();
    activeVariantName = FastName();
    activeVariantInstance = nullptr;
    needRebuildVariants = false;
    needRebuildBindings = true;
    needRebuildTextures = true;
}

void NMaterial::CollectMaterialFlags(HashMap<FastName, int32>& target)
{
    if (parent)
        parent->CollectMaterialFlags(target);
    for (auto& it : materialConfigs[currConfig].localFlags)
    {
        if (it.second == 0) //ZERO is a special value that means flag is off - at least all shaders are consider it to be this right now
            target.erase(it.first);
        else
            target[it.first] = it.second;
    }
}

void NMaterial::RebuildBindings()
{
    ClearLocalBuffers();
    for (auto& variant : renderVariants)
    {
        RenderVariantInstance* currRenderVariant = variant.second;
        ShaderDescriptor* currShader = currRenderVariant->shader;
        if (!currShader->IsValid()) //cant build for empty shader
            continue;
        currRenderVariant->vertexConstBuffers.resize(currShader->GetVertexConstBuffersCount());
        currRenderVariant->fragmentConstBuffers.resize(currShader->GetFragmentConstBuffersCount());

        for (auto& bufferDescr : currShader->GetConstBufferDescriptors())
        {
            rhi::HConstBuffer bufferHandle;
            MaterialBufferBinding* bufferBinding = nullptr;
            //for static buffers resolve sharing and bindings
            if (bufferDescr.updateType == rhi::ShaderProp::STORAGE_STATIC)
            {
                bufferBinding = GetConstBufferBinding(bufferDescr.propertyLayoutId);
                bool needLocalOverride = NeedLocalOverride(bufferDescr.propertyLayoutId);
                //Create local buffer and build it's bindings if required;
                if ((bufferBinding == nullptr) || needLocalOverride)
                {
                    //create buffer
                    bufferBinding = new MaterialBufferBinding();

                    //create handles
                    if (bufferDescr.type == ConstBufferDescriptor::Type::Vertex)
                        bufferBinding->constBuffer = rhi::CreateVertexConstBuffer(currShader->GetPiplineState(), bufferDescr.targetSlot);
                    else
                        bufferBinding->constBuffer = rhi::CreateFragmentConstBuffer(currShader->GetPiplineState(), bufferDescr.targetSlot);

                    if (bufferBinding->constBuffer != rhi::InvalidHandle)
                    {
                        //if const buffer is InvalidHandle this means that whole const buffer was cut by shader compiler/linker
                        //it should not be updated but still can be shared as other shader variants can use it

                        //create bindings for this buffer
                        for (auto& propDescr : ShaderDescriptor::GetProps(bufferDescr.propertyLayoutId))
                        {
                            NMaterialProperty* prop = GetMaterialProperty(propDescr.uid);
                            if ((prop != nullptr)) //has property of the same type
                            {
                                DVASSERT(prop->type == propDescr.type);

                                // create property binding

                                bufferBinding->propBindings.emplace_back(propDescr.type,
                                                                         propDescr.bufferReg, propDescr.bufferRegCount, 0, prop);
                            }
                            else
                            {
                                //just set default property to const buffer
                                if (propDescr.type < rhi::ShaderProp::TYPE_FLOAT4)
                                {
                                    rhi::UpdateConstBuffer1fv(bufferBinding->constBuffer, propDescr.bufferReg, propDescr.bufferRegCount, propDescr.defaultValue, ShaderDescriptor::CalculateDataSize(propDescr.type, 1));
                                }
                                else
                                {
                                    rhi::UpdateConstBuffer4fv(bufferBinding->constBuffer, propDescr.bufferReg, propDescr.defaultValue, propDescr.bufferRegCount);
                                }
                            }
                        }
                    }

                    //store it locally or at parent
                    if (needLocalOverride || (!parent))
                    {
                        //buffer should be handled locally
                        localConstBuffers[bufferDescr.propertyLayoutId] = bufferBinding;
                    }
                    else
                    {
                        //buffer can be propagated upward
                        parent->InjectChildBuffer(bufferDescr.propertyLayoutId, bufferBinding);
                    }
                }
                currRenderVariant->materialBufferBindings.push_back(bufferBinding);

                bufferHandle = bufferBinding->constBuffer;
            }

            else //if (bufferDescr.updateType == ConstBufferDescriptor::ConstBufferUpdateType::Static)
            {
                //for dynamic buffers just copy it's handle to corresponding slot
                bufferHandle = currShader->GetDynamicBuffer(bufferDescr.type, bufferDescr.targetSlot);
            }

            if (bufferHandle.IsValid())
            {
                if (bufferDescr.type == ConstBufferDescriptor::Type::Vertex)
                    currRenderVariant->vertexConstBuffers[bufferDescr.targetSlot] = bufferHandle;
                else
                    currRenderVariant->fragmentConstBuffers[bufferDescr.targetSlot] = bufferHandle;
            }
        }
    }

    needRebuildBindings = false;
}

void NMaterial::RebuildTextureBindings()
{
    for (auto& variant : renderVariants)
    {
        RenderVariantInstance* currRenderVariant = variant.second;

        //release existing
        rhi::ReleaseTextureSet(currRenderVariant->textureSet);
        rhi::ReleaseSamplerState(currRenderVariant->samplerState);

        ShaderDescriptor* currShader = currRenderVariant->shader;
        if (!currShader->IsValid()) //cant build for empty shader
            continue;
        rhi::TextureSetDescriptor textureDescr;
        rhi::SamplerState::Descriptor samplerDescr;
        const rhi::ShaderSamplerList& fragmentSamplerList = currShader->GetFragmentSamplerList();
        const rhi::ShaderSamplerList& vertexSamplerList = currShader->GetVertexSamplerList();

        textureDescr.fragmentTextureCount = static_cast<uint32>(fragmentSamplerList.size());
        samplerDescr.fragmentSamplerCount = static_cast<uint32>(fragmentSamplerList.size());
        for (size_t i = 0, sz = textureDescr.fragmentTextureCount; i < sz; ++i)
        {
            RuntimeTextures::eDynamicTextureSemantic textureSemantic = RuntimeTextures::GetDynamicTextureSemanticByName(currShader->GetFragmentSamplerList()[i].uid);
            if (textureSemantic == RuntimeTextures::TEXTURE_STATIC)
            {
                Texture* tex = GetEffectiveTexture(fragmentSamplerList[i].uid);
                if (tex)
                {
                    textureDescr.fragmentTexture[i] = tex->handle;
                    samplerDescr.fragmentSampler[i] = tex->samplerState;
                }
                else
                {
                    textureDescr.fragmentTexture[i] = Renderer::GetRuntimeTextures().GetPinkTexture(fragmentSamplerList[i].type);
                    samplerDescr.fragmentSampler[i] = Renderer::GetRuntimeTextures().GetPinkTextureSamplerState(fragmentSamplerList[i].type);

                    Logger::FrameworkDebug(" no texture for slot : %s", fragmentSamplerList[i].uid.c_str());
                }
            }
            else
            {
                textureDescr.fragmentTexture[i] = Renderer::GetRuntimeTextures().GetDynamicTexture(textureSemantic);
                samplerDescr.fragmentSampler[i] = Renderer::GetRuntimeTextures().GetDynamicTextureSamplerState(textureSemantic);
            }
            DVASSERT(textureDescr.fragmentTexture[i].IsValid());
        }

        textureDescr.vertexTextureCount = static_cast<uint32>(vertexSamplerList.size());
        samplerDescr.vertexSamplerCount = static_cast<uint32>(vertexSamplerList.size());
        for (size_t i = 0, sz = textureDescr.vertexTextureCount; i < sz; ++i)
        {
            Texture* tex = GetEffectiveTexture(vertexSamplerList[i].uid);
            if (tex)
            {
                textureDescr.vertexTexture[i] = tex->handle;
                samplerDescr.vertexSampler[i] = tex->samplerState;
            }
            else
            {
                textureDescr.vertexTexture[i] = Renderer::GetRuntimeTextures().GetPinkTexture(vertexSamplerList[i].type);
                samplerDescr.vertexSampler[i] = Renderer::GetRuntimeTextures().GetPinkTextureSamplerState(vertexSamplerList[i].type);
            }
            samplerDescr.vertexSampler[i].mipFilter = rhi::TEXMIPFILTER_NONE;
        }

        currRenderVariant->textureSet = rhi::AcquireTextureSet(textureDescr);
        currRenderVariant->samplerState = rhi::AcquireSamplerState(samplerDescr);
    }

    needRebuildTextures = false;
}

bool NMaterial::PreBuildMaterial(const FastName& passName)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    //shader rebuild first - as it sets needRebuildBindings and needRebuildTextures
    if (needRebuildVariants)
        RebuildRenderVariants();
    if (needRebuildBindings)
        RebuildBindings();
    if (needRebuildTextures)
        RebuildTextureBindings();

    bool res = (activeVariantInstance != nullptr) && (activeVariantInstance->shader->IsValid());
    if (activeVariantName != passName)
    {
        RenderVariantInstance* targetVariant = renderVariants[passName];

        if (targetVariant != nullptr)
        {
            activeVariantName = passName;
            activeVariantInstance = targetVariant;

            res = (activeVariantInstance->shader->IsValid());
        }
        else
        {
            res = false;
        }
    }
    return res;
}

NMaterial* NMaterial::Clone()
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    NMaterial* clonedMaterial = new NMaterial();
    clonedMaterial->materialName = materialName;
    clonedMaterial->qualityGroup = qualityGroup;

    clonedMaterial->materialConfigs.resize(materialConfigs.size());
    for (size_t i = 0, sz = materialConfigs.size(); i < sz; ++i)
    {
        clonedMaterial->materialConfigs[i].fxName = materialConfigs[i].fxName;
        for (auto prop : materialConfigs[i].localProperties)
        {
            NMaterialProperty* newProp = new NMaterialProperty();
            newProp->name = prop.first;
            newProp->type = prop.second->type;
            newProp->arraySize = prop.second->arraySize;
            newProp->data.reset(new float[ShaderDescriptor::CalculateDataSize(newProp->type, newProp->arraySize)]);
            newProp->SetPropertyValue(prop.second->data.get());
            clonedMaterial->materialConfigs[i].localProperties[newProp->name] = newProp;
        }

        for (auto tex : materialConfigs[i].localTextures)
        {
            MaterialTextureInfo* res = new MaterialTextureInfo();
            res->path = tex.second->path;
            res->texture = SafeRetain(tex.second->texture);
            clonedMaterial->materialConfigs[i].localTextures[tex.first] = res;
        }

        for (auto flag : materialConfigs[i].localFlags)
            materialConfigs[i].localFlags[flag.first] = flag.second;
    }

    clonedMaterial->SetParent(parent);

    // DataNode properties
    clonedMaterial->id = 0;
    clonedMaterial->scene = scene;
    clonedMaterial->isRuntime = isRuntime;

    return clonedMaterial;
}

void NMaterial::SaveConfigToArchive(uint32 configId, KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (materialConfigs[configId].fxName.IsValid())
        archive->SetString(NMaterialSerializationKey::FXName, materialConfigs[configId].fxName.c_str());

    if (materialConfigs[configId].presetName.IsValid())
        archive->SetString(NMaterialSerializationKey::ConfigName, materialConfigs[configId].presetName.c_str());

    ScopedPtr<KeyedArchive> propertiesArchive(new KeyedArchive());
    for (HashMap<FastName, NMaterialProperty *>::iterator it = materialConfigs[configId].localProperties.begin(), itEnd = materialConfigs[configId].localProperties.end(); it != itEnd; ++it)
    {
        NMaterialProperty* property = it->second;

        uint32 dataSize = ShaderDescriptor::CalculateDataSize(property->type, property->arraySize) * sizeof(float32);
        uint32 storageSize = sizeof(uint8) + sizeof(uint32) + dataSize;
        uint8* propertyStorage = new uint8[storageSize];

        memcpy(propertyStorage, &property->type, sizeof(uint8));
        memcpy(propertyStorage + sizeof(uint8), &property->arraySize, sizeof(uint32));
        memcpy(propertyStorage + sizeof(uint8) + sizeof(uint32), property->data.get(), dataSize);

        propertiesArchive->SetByteArray(it->first.c_str(), propertyStorage, storageSize);

        SafeDeleteArray(propertyStorage);
    }
    archive->SetArchive("properties", propertiesArchive);

    ScopedPtr<KeyedArchive> texturesArchive(new KeyedArchive());
    for (auto it = materialConfigs[configId].localTextures.begin(), itEnd = materialConfigs[configId].localTextures.end(); it != itEnd; ++it)
    {
        if (!it->second->path.IsEmpty())
        {
            String textureRelativePath = it->second->path.GetRelativePathname(serializationContext->GetScenePath());
            if (textureRelativePath.size() > 0)
            {
                texturesArchive->SetString(it->first.c_str(), textureRelativePath);
            }
        }
    }
    archive->SetArchive("textures", texturesArchive);

    ScopedPtr<KeyedArchive> flagsArchive(new KeyedArchive());
    for (HashMap<FastName, int32>::iterator it = materialConfigs[configId].localFlags.begin(), itEnd = materialConfigs[configId].localFlags.end(); it != itEnd; ++it)
    {
        if (!NMaterialFlagName::IsRuntimeFlag(it->first))
            flagsArchive->SetInt32(it->first.c_str(), it->second);
    }
    archive->SetArchive("flags", flagsArchive);
}

void NMaterial::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    DataNode::Save(archive, serializationContext);

    if (parent)
        archive->SetUInt64(NMaterialSerializationKey::ParentMaterialKey, parent->GetNodeID());

    if (materialName.IsValid())
        archive->SetString(NMaterialSerializationKey::MaterialName, materialName.c_str());

    if (qualityGroup.IsValid())
        archive->SetString(NMaterialSerializationKey::QualityGroup, qualityGroup.c_str());

    uint32 configsCount = materialConfigs.size();
    if (configsCount == 1)
    {
        //preserve old storage format
        SaveConfigToArchive(0, archive, serializationContext);
    }
    else
    {
        archive->SetUInt32(NMaterialSerializationKey::ConfigCount, configsCount);
        for (uint32 i = 0, sz = configsCount; i < sz; ++i)
        {
            ScopedPtr<KeyedArchive> configArchive(new KeyedArchive());
            SaveConfigToArchive(i, configArchive, serializationContext);
            archive->SetArchive(Format(NMaterialSerializationKey::ConfigArchvie.c_str(), i), configArchive);
        }
    }
}

void NMaterial::LoadConfigFromArchive(uint32 configId, KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (!archive)
    {
        Logger::Error("material %s has no archive for config %d", materialName.c_str(), configId);
        return;
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::FXName))
    {
        materialConfigs[configId].fxName = FastName(archive->GetString(NMaterialSerializationKey::FXName));
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::ConfigName))
    {
        materialConfigs[configId].presetName = FastName(archive->GetString(NMaterialSerializationKey::ConfigName));
    }

    if (archive->IsKeyExists("properties"))
    {
        const KeyedArchive::UnderlyingMap& propsMap = archive->GetArchive("properties")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = propsMap.begin(); it != propsMap.end(); ++it)
        {
            const VariantType* propVariant = it->second;
            DVASSERT(VariantType::TYPE_BYTE_ARRAY == propVariant->type);
            DVASSERT(propVariant->AsByteArraySize() >= static_cast<int32>(sizeof(uint8) + sizeof(uint32)));

            const uint8* ptr = propVariant->AsByteArray();
            FastName propName = FastName(it->first);
            uint8 propType = *ptr;
            ptr += sizeof(uint8);
            uint32 propSize = *(uint32*)ptr;
            ptr += sizeof(uint32);
            float32* data = (float32*)ptr;

            NMaterialProperty* newProp = new NMaterialProperty();
            newProp->name = propName;
            newProp->type = (rhi::ShaderProp::Type)propType;
            newProp->arraySize = propSize;
            newProp->data.reset(new float[ShaderDescriptor::CalculateDataSize(newProp->type, newProp->arraySize)]);
            newProp->SetPropertyValue(data);
            materialConfigs[configId].localProperties[newProp->name] = newProp;
        }
    }

    if (archive->IsKeyExists("textures"))
    {
        const KeyedArchive::UnderlyingMap& texturesMap = archive->GetArchive("textures")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = texturesMap.begin(); it != texturesMap.end(); ++it)
        {
            String relativePathname = it->second->AsString();
            MaterialTextureInfo* texInfo = new MaterialTextureInfo();
            texInfo->path = serializationContext->GetScenePath() + relativePathname;
            materialConfigs[configId].localTextures[FastName(it->first)] = texInfo;
        }
    }

    if (archive->IsKeyExists("flags"))
    {
        const KeyedArchive::UnderlyingMap& flagsMap = archive->GetArchive("flags")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = flagsMap.begin(); it != flagsMap.end(); ++it)
        {
            materialConfigs[configId].localFlags[FastName(it->first)] = it->second->AsInt32();
        }
    }
}

void NMaterial::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();

    materialConfigs.clear();
    currConfig = 0;
    DataNode::Load(archive, serializationContext);

    if (serializationContext->GetVersion() < RHI_SCENE_VERSION)
    {
        LoadOldNMaterial(archive, serializationContext);
        return;
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::MaterialName))
    {
        materialName = FastName(archive->GetString(NMaterialSerializationKey::MaterialName));
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::MaterialKey))
    {
        uint64 materialKey = archive->GetUInt64(NMaterialSerializationKey::MaterialKey);
        id = materialKey;
    }

    uint64 parentKey(0);
    if (archive->IsKeyExists(NMaterialSerializationKey::ParentMaterialKey))
    {
        parentKey = archive->GetUInt64(NMaterialSerializationKey::ParentMaterialKey);
    }
    serializationContext->AddBinding(parentKey, this); //parentKey == 0 is global material if it exists, or no-parent otherwise

    if (archive->IsKeyExists(NMaterialSerializationKey::QualityGroup))
    {
        qualityGroup = FastName(archive->GetString(NMaterialSerializationKey::QualityGroup).c_str());
    }

    uint32 configCount = archive->GetUInt32(NMaterialSerializationKey::MaterialKey, 1);
    materialConfigs.resize(configCount);
    if (configCount == 1)
    {
        LoadConfigFromArchive(0, archive, serializationContext);
    }
    else
    {
        for (uint32 i = 0; i < configCount; ++i)
            LoadConfigFromArchive(i, archive->GetArchive(Format(NMaterialSerializationKey::ConfigArchvie.c_str(), i)), serializationContext);
    }
}

void NMaterial::LoadOldNMaterial(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DAVA_MEMORY_PROFILER_CLASS_ALLOC_SCOPE();
    materialConfigs.resize(1);
    /*the following stuff is for importing old NMaterial stuff*/

    if (archive->IsKeyExists(NMaterialSerializationKey::MaterialName))
    {
        materialName = FastName(archive->GetString(NMaterialSerializationKey::MaterialName));
    }

    if (archive->IsKeyExists(NMaterialSerializationKey::MaterialKey))
    {
        uint64 materialKey = archive->GetUInt64(NMaterialSerializationKey::MaterialKey);
        id = materialKey;
    }

    int32 oldType = 0;
    if (archive->IsKeyExists("materialType"))
    {
        oldType = archive->GetInt32("materialType");
    }

    uint64 parentKey(0);
    if (archive->IsKeyExists(NMaterialSerializationKey::ParentMaterialKey))
    {
        parentKey = archive->GetUInt64(NMaterialSerializationKey::ParentMaterialKey);
    }
    serializationContext->AddBinding(parentKey, this); //parentKey == 0 is global material if it exists, or no-parent otherwise

    if (archive->IsKeyExists("materialGroup"))
    {
        qualityGroup = FastName(archive->GetString("materialGroup").c_str());
    }

    // don't load fxName from material instance (type = 2)
    if (archive->IsKeyExists("materialTemplate") && oldType != 2)
    {
        auto materialTemplate = archive->GetString("materialTemplate");
        materialConfigs[0].fxName = materialTemplate.empty() ? FastName() : FastName(materialTemplate);
    }

    if (archive->IsKeyExists("textures"))
    {
        const KeyedArchive::UnderlyingMap& texturesMap = archive->GetArchive("textures")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = texturesMap.begin();
             it != texturesMap.end();
             ++it)
        {
            String relativePathname = it->second->AsString();
            MaterialTextureInfo* texInfo = new MaterialTextureInfo();
            texInfo->path = serializationContext->GetScenePath() + relativePathname;
            materialConfigs[0].localTextures[FastName(it->first)] = texInfo;
        }
    }

    if (archive->IsKeyExists("setFlags"))
    {
        const KeyedArchive::UnderlyingMap& flagsMap = archive->GetArchive("setFlags")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = flagsMap.begin(); it != flagsMap.end(); ++it)
        {
            AddFlag(FastName(it->first), it->second->AsInt32());
        }
    }

    //NMaterial hell - for some reason property types were saved as GL_XXX defines O_o
    const uint32 originalTypesCount = 5;
    struct
    {
        uint32 originalType;
        rhi::ShaderProp::Type newType;
    } propertyTypeRemapping[originalTypesCount] =
    {
      { 0x1406 /*GL_FLOAT*/, rhi::ShaderProp::TYPE_FLOAT1 },
      { 0x8B50 /*GL_FLOAT_VEC2*/, rhi::ShaderProp::TYPE_FLOAT2 },
      { 0x8B51 /*GL_FLOAT_VEC3*/, rhi::ShaderProp::TYPE_FLOAT3 },
      { 0x8B52 /*GL_FLOAT_VEC4*/, rhi::ShaderProp::TYPE_FLOAT4 },
      { 0x8B5C /*GL_FLOAT_MAT4*/, rhi::ShaderProp::TYPE_FLOAT4X4 }
    };

    Array<FastName, 8> propertyFloat4toFloat3 =
    { {
    NMaterialParamName::PARAM_FOG_COLOR,
    NMaterialParamName::PARAM_FOG_ATMOSPHERE_COLOR_SKY,
    NMaterialParamName::PARAM_FOG_ATMOSPHERE_COLOR_SUN,
    Landscape::PARAM_TILE_COLOR0,
    Landscape::PARAM_TILE_COLOR1,
    Landscape::PARAM_TILE_COLOR2,
    Landscape::PARAM_TILE_COLOR3,
    } };

    Array<FastName, 2> propertyFloat3toFloat4 =
    { { NMaterialParamName::PARAM_FLAT_COLOR,
        NMaterialParamName::PARAM_DECAL_TILE_COLOR } };

    Array<FastName, 1> propertyFloat1toFloat2 =
    {
      { NMaterialParamName::PARAM_DECAL_TILE_SCALE }
    };

    if (archive->IsKeyExists("properties"))
    {
        const KeyedArchive::UnderlyingMap& propsMap = archive->GetArchive("properties")->GetArchieveData();
        for (KeyedArchive::UnderlyingMap::const_iterator it = propsMap.begin(); it != propsMap.end(); ++it)
        {
            const VariantType* propVariant = it->second;
            DVASSERT(VariantType::TYPE_BYTE_ARRAY == propVariant->type);
            DVASSERT(propVariant->AsByteArraySize() >= static_cast<int32>(sizeof(uint32) + sizeof(uint32)));

            const uint8* ptr = propVariant->AsByteArray();

            FastName propName = FastName(it->first);
            uint32 propType = *(uint32*)ptr;
            ptr += sizeof(uint32);
            uint8 propSize = *(uint8*)ptr;
            ptr += sizeof(uint8);
            float32* data = (float32*)ptr;
            for (uint32 i = 0; i < originalTypesCount; i++)
            {
                if (propType == propertyTypeRemapping[i].originalType)
                {
                    if (propertyTypeRemapping[i].newType == rhi::ShaderProp::TYPE_FLOAT4)
                    {
                        if (std::find(propertyFloat4toFloat3.begin(), propertyFloat4toFloat3.end(), propName) != propertyFloat4toFloat3.end())
                        {
                            AddProperty(propName, data, rhi::ShaderProp::TYPE_FLOAT3, 1);
                            continue;
                        }
                    }
                    else if (propertyTypeRemapping[i].newType == rhi::ShaderProp::TYPE_FLOAT3)
                    {
                        if (std::find(propertyFloat3toFloat4.begin(), propertyFloat3toFloat4.end(), propName) != propertyFloat3toFloat4.end())
                        {
                            float32 data4[4];
                            Memcpy(data4, data, 3 * sizeof(float32));
                            data[3] = 1.f;

                            AddProperty(propName, data, rhi::ShaderProp::TYPE_FLOAT4, 1);
                            continue;
                        }
                    }
                    else if (propertyTypeRemapping[i].newType == rhi::ShaderProp::TYPE_FLOAT1)
                    {
                        if (std::find(propertyFloat1toFloat2.begin(), propertyFloat1toFloat2.end(), propName) != propertyFloat1toFloat2.end())
                        {
                            Vector2 v2(*data, *data);
                            AddProperty(propName, v2.data, rhi::ShaderProp::TYPE_FLOAT2, 1);
                            continue;
                        }
                    }

                    AddProperty(propName, data, propertyTypeRemapping[i].newType, 1);
                }
            }
        }
    }

    bool illuminationUsed = false;
    if (archive->IsKeyExists("illumination.isUsed"))
    {
        illuminationUsed = archive->GetBool("illumination.isUsed");
        AddFlag(NMaterialFlagName::FLAG_ILLUMINATION_USED, illuminationUsed);
    }

    if (archive->IsKeyExists("illumination.castShadow"))
    {
        AddFlag(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER, archive->GetBool("illumination.castShadow"));
    }
    else if (illuminationUsed)
    {
        AddFlag(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER, false); // need for material editor
    }

    if (archive->IsKeyExists("illumination.receiveShadow"))
    {
        AddFlag(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER, archive->GetBool("illumination.receiveShadow"));
    }
    else if (illuminationUsed)
    {
        AddFlag(NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER, false); // need for material editor
    }

    if (archive->IsKeyExists("illumination.lightmapSize"))
    {
        float32 lighmapSize = (float32)archive->GetInt32("illumination.lightmapSize", static_cast<int32>(DEFAULT_LIGHTMAP_SIZE));
        if (HasLocalProperty(NMaterialParamName::PARAM_LIGHTMAP_SIZE))
            SetPropertyValue(NMaterialParamName::PARAM_LIGHTMAP_SIZE, &lighmapSize);
        else
            AddProperty(NMaterialParamName::PARAM_LIGHTMAP_SIZE, &lighmapSize, rhi::ShaderProp::TYPE_FLOAT1, 1);
    }
    else if (illuminationUsed)
    {
        AddProperty(NMaterialParamName::PARAM_LIGHTMAP_SIZE, &DEFAULT_LIGHTMAP_SIZE, rhi::ShaderProp::TYPE_FLOAT1, 1);
    }
}

const HashMap<FastName, int32>& NMaterial::GetLocalFlags() const
{
    return materialConfigs[currConfig].localFlags;
}
};
