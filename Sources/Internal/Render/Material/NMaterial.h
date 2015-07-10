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


#ifndef __DAVAENGINE_NMATERIAL_H__
#define __DAVAENGINE_NMATERIAL_H__

#include <memory>
#include "Base/FastNameMap.h"
#include "NMaterialNames.h"
#include "NMaterialStateDynamicFlagsInsp.h"
#include "NMaterialStateDynamicPropertiesInsp.h"
#include "NMaterialStateDynamicTexturesInsp.h"
#include "Render/Shader.h"
#include "Scene3D/DataNode.h"

namespace DAVA
{

struct NMaterialProperty
{
    FastName name;
    rhi::ShaderProp::Type type;
    uint32 arraySize;
    std::unique_ptr<float32[]> data;
    uint32 updateSemantic;

    inline void SetPropertyValue(const float32 *newValue);    
    inline static uint32 GetCurrentUpdateSemantic(){ return globalPropertyUpdateSemanticCounter; }
private:
    static uint32 globalPropertyUpdateSemanticCounter;
};

struct MaterialPropertyBinding
{
    rhi::ShaderProp::Type type;
    uint32 reg;
    uint32 regCount; //offset for props less than 1 reg size
    uint32 updateSemantic;
    NMaterialProperty *source;
};

struct MaterialBufferBinding
{
    rhi::HConstBuffer constBuffer;
    Vector<MaterialPropertyBinding> propBindings;
    uint32 lastValidPropertySemantic;
};

struct MaterialTextureInfo
{
    Texture *texture = nullptr;
    FilePath path;
};

class RenderVariantInstance
{
    friend class NMaterial;
    ShaderDescriptor *shader;

    rhi::HDepthStencilState depthState;
    rhi::HSamplerState samplerState;
    rhi::HTextureSet textureSet;
    rhi::CullMode cullMode;

    Vector<rhi::HConstBuffer> vertexConstBuffers;
    Vector<rhi::HConstBuffer> fragmentConstBuffers;

    Vector<MaterialBufferBinding *> materialBufferBindings;
    
    uint32 renderLayer;

    RenderVariantInstance();
    RenderVariantInstance(const RenderVariantInstance&) = delete;
    ~RenderVariantInstance();
};

class NMaterial : public DataNode
{
    // this classed need access to be able to generate
    // dynamic introspection for NMaterial class
    friend class NMaterialStateDynamicFlagsInsp;
    friend class NMaterialStateDynamicPropertiesInsp;
    friend class NMaterialStateDynamicTexturesInsp;

public:
    NMaterial();    
    ~NMaterial();

    void Load(KeyedArchive * archive, SerializationContext * serializationContext) override;
    void Save(KeyedArchive * archive, SerializationContext * serializationContext) override;
    NMaterial* Clone();

    void SetFXName(const FastName & fxName);
    bool HasLocalFXNmae() const;
    const FastName& GetLocalFXName() const;
    const FastName& GetEffectiveFxName() const;

    inline void SetMaterialName(const FastName & name);
    inline const FastName& GetMaterialName() const;

    uint32 GetRequiredVertexFormat();

    /*properties*/
    void AddProperty(const FastName& propName, const float32 *propData, rhi::ShaderProp::Type type, uint32 arraySize = 1);
    void RemoveProperty(const FastName& propName);
    void SetPropertyValue(const FastName& propName, const float32 *propData);    
    bool HasLocalProperty(const FastName& propName);   
    rhi::ShaderProp::Type GetLocalPropType(const FastName& propName);
    const float32* GetLocalPropValue(const FastName& propName);
    const float32* GetEffectivePropValue(const FastName& propName);

    /*textures*/
    void AddTexture(const FastName& slotName, Texture *texture);
    void RemoveTexture(const FastName& slotName);
    void SetTexture(const FastName& slotName, Texture *texture);
    bool HasLocalTexture(const FastName& slotName);
    Texture* GetLocalTexture(const FastName& slotName);
    Texture* GetEffectiveTexture(const FastName& slotName);

    /*flags*/
    void AddFlag(const FastName& flagName, int32 value);
    void RemoveFlag(const FastName& flagName);
    void SetFlag(const FastName& flagName, int32 value);
    bool HasLocalFlag(const FastName& flagName);
    int32 GetLocalFlagValue(const FastName& flagName);
    int32 GetEffectiveFlagValue(const FastName& flagName);

    void SetParent(NMaterial *parent);
    NMaterial* GetParent();
    const Vector<NMaterial *>& GetChildren() const;

    inline uint32 GetRenderLayerID() const;
    inline uint32 GetSortingKey() const;
    inline uint64 GetMaterialKey() const;

    void BindParams(rhi::Packet& target);

    //returns true if has variant for this pass, false otherwise - if material doesn't support pass active variant will be not changed
    bool PreBuildMaterial(const FastName& passName); //later add engine flags here

private:
    void LoadOldNMaterial(KeyedArchive * archive, SerializationContext * serializationContext);

    void InvalidateBufferBindings();
    void InvalidateTextureBindings();
    void InvalidateRenderVariants();

    void RebuildBindings();
    void RebuildTextureBindings();
    void RebuildRenderVariants();

    bool NeedLocalOverride(UniquePropertyLayout propertyLayout);
    void ClearLocalBuffers();
    void InjectChildBuffer(UniquePropertyLayout propLayoutId, MaterialBufferBinding* buffer);

    /*the following functions will collect data recursively*/
    MaterialBufferBinding* GetConstBufferBinding(UniquePropertyLayout propertyLayout);
    NMaterialProperty* GetMaterialProperty(const FastName& propName);    
    void CollectMaterialFlags(HashMap<FastName, int32>& target);
    
    const FastName& GetQualityGroup();
    void SetQualityGroup(const FastName& quality);

    void AddChildMaterial(NMaterial *material);
    void RemoveChildMaterial(NMaterial *material);    

private:
    //config time
    FastName materialName;
    FastName fxName;
    FastName qualityGroup;

    HashMap<FastName, NMaterialProperty *> localProperties;
    HashMap<FastName, MaterialTextureInfo*> localTextures;
    HashMap<FastName, int32> localFlags; //integer flags are just more generic then boolean (eg. #if SHADING == HIGH), it has nothing in common with eFlagValue bullshit from old NMaterial    

    //runtime
    NMaterial *parent;
    Vector<NMaterial *> children;
    HashMap<UniquePropertyLayout, MaterialBufferBinding*> localConstBuffers;

    /*this is for render passes - not used right now - only active variant instance*/
    HashMap<FastName, RenderVariantInstance *> renderVariants;

    FastName activeVariantName;
    RenderVariantInstance *activeVariantInstance;

    bool needRebuildBindings;
    bool needRebuildTextures;
    bool needRebuildVariants;    

    uint32 sortingKey;
    uint64 materialKey;

public:
    INTROSPECTION_EXTEND(NMaterial, DataNode,
        PROPERTY("materialName", "Material name", GetMaterialName, SetMaterialName, I_VIEW | I_EDIT)
        PROPERTY("fxName", "FX Name", GetLocalFXName, SetFXName, I_VIEW | I_EDIT)
        PROPERTY("qualityGroup", "Quality group", GetQualityGroup, SetQualityGroup, I_VIEW | I_EDIT)
        DYNAMIC(localFlags, "Material flags", new NMaterialStateDynamicFlagsInsp(), I_EDIT | I_VIEW)
        DYNAMIC(localProperties, "Material properties", new NMaterialStateDynamicPropertiesInsp(), I_EDIT | I_VIEW)
        DYNAMIC(localTextures, "Material textures", new NMaterialStateDynamicTexturesInsp(), I_EDIT | I_VIEW)
        );
};

void NMaterialProperty::SetPropertyValue(const float32 *newValue)
{
    //4 is because register size is float4
    Memcpy(data.get(), newValue, sizeof(float32) * ShaderDescriptor::CalculateDataSize(type, arraySize));
    updateSemantic = ++globalPropertyUpdateSemanticCounter;
}

void NMaterial::SetMaterialName(const FastName & name)
{
    materialName = name;
}
const FastName& NMaterial::GetMaterialName() const
{
    return materialName;
}
uint32 NMaterial::GetRenderLayerID() const
{
    if (activeVariantInstance)
        return activeVariantInstance->renderLayer;
    else
        return (uint32)-1;
}
uint32 NMaterial::GetSortingKey() const
{
    return sortingKey;
}
uint64 NMaterial::GetMaterialKey() const
{
    return materialKey;
}
};

#endif // __DAVAENGINE_MATERIAL_H__

