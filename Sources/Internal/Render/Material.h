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
#ifndef __DAVAENGINE_MATERIAL_H__
#define __DAVAENGINE_MATERIAL_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Scene3D/DataNode.h"
#include "Render/RenderStateBlock.h"

namespace DAVA
{


class UberShader;
class Shader;
class Texture;    
class SceneFileV2;
class LightNode;
class PolygonGroup;
class RenderDataObject;
    
    
// TODO: move Material to Scene3D
    
class InstanceMaterialState : public BaseObject
{
    static const int32 LIGHT_NODE_MAX_COUNT = 4;
public:
    InstanceMaterialState();
    virtual ~InstanceMaterialState();
    
    void SetLight(int32 lightIndex, LightNode * lightNode);
    LightNode * GetLight(int32 lightIndex);
    
    void SetLightmap(Texture * texture, const String & lightmapName);
    void SetUVOffsetScale(const Vector2 & uvOffset, const Vector2 uvScale);
    
private:
    Texture * lightmapTexture;
    String lightmapName;
    Vector2 uvOffset;
    Vector2 uvScale;
    
    LightNode * lightNodes[LIGHT_NODE_MAX_COUNT];
    
    friend class Material;
public:
    INTROSPECTION_EXTEND(InstanceMaterialState, BaseObject,
                         //MEMBER(lightmapTexture, "Texture:", INTROSPECTION_EDITOR)
                         MEMBER(lightmapName, "Lightmap Name:", INTROSPECTION_EDITOR)
                         MEMBER(uvOffset, "UV Offset", INTROSPECTION_EDITOR)
                         MEMBER(uvScale, "UV Scale", INTROSPECTION_EDITOR)
                         //MEMBER(aabbox, "AABBox", INTROSPECTION_EDITOR)
                         );
};
    
    
class Material : public DataNode
{
public:
    enum eType
    {
        // Normal Materials
        MATERIAL_UNLIT_TEXTURE = 0,                 // texture
        MATERIAL_UNLIT_TEXTURE_DETAIL,              // texture * detail texture * 2.0
        MATERIAL_UNLIT_TEXTURE_DECAL,               // texture * decal 
        MATERIAL_UNLIT_TEXTURE_LIGHTMAP,            // texture * lightmap
        
        MATERIAL_VERTEX_LIT_TEXTURE,                // single texture with vertex lighting
        MATERIAL_VERTEX_LIT_DETAIL,                 // single texture * detail texture * 2.0 with vertex lighting
        MATERIAL_VERTEX_LIT_DECAL,
        MATERIAL_VERTEX_LIT_LIGHTMAP,               // vertex lit lighting + lightmaps
        
        MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE,
        MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR,     // single texture + diffuse light normal mapping
        MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP, // single texture + diffuse light normal mapping

		MATERIAL_VERTEX_COLOR_ALPHABLENDED,
        
        // MATERIAL_TEXTURE, 
        // MATERIAL_LIGHTMAPPED_TEXTURE,   
        // MATERIAL_VERTEX_LIGHTING,       // flag
        // MATERIAL_NORMAL_MAPPED,         // flag
        MATERIAL_TYPES_COUNT
    };
    
    
    /*
        Plan of supported materials:
            Default per vertex lighting material
            Default per pixel lighting material
            Default deferred lighting material
     */
    
    enum eMaterialGeneratorInput
    {
        MATERIAL_INPUT_DIFFUSE = (1 << 1),
        MATERIAL_INPUT_SPECULAR = (1 << 2),
        MATERIAL_INPUT_SPECULAR_POWER = (1 << 3),
        MATERIAL_INPUT_NORMAL = (1 << 4),
        MATERIAL_INPUT_OPACITY = (1 << 5),
        MATERIAL_INPUT_OPACITY_MASK = (1 << 6),
        MATERIAL_INPUT_EMISSIVE = (1 << 7),
    };
    
    static const char8 * GetTypeName(eType type);

    Material();
    virtual ~Material();
    
    
    enum eValidationResult
    {
        VALIDATE_COMPATIBLE = 1,
        VALIDATE_INCOMPATIBLE,
    };
    
    eValidationResult Validate(PolygonGroup * polygonGroup);
    
    virtual void SetScene(Scene * _scene);
   
	virtual int32 Release();
    
    void SetType(eType _type);
    
    void SetOpaque(bool _isOpaque);
    bool GetOpaque();

	void SetAlphablend(bool isAlphablend);
	bool GetAlphablend();
    
    void SetWireframe(bool isWireframe);
    bool GetWireframe();
    
    void SetFog(bool _fogEnabled);
    bool IsFogEnabled() const;
    void SetFogDensity(float32 _fogDensity);
    float32 GetFogDensity() const;
    void SetFogColor(const Color & _fogColor);
    const Color & GetFogColor() const;
    
    
    void SetTwoSided(bool _isTwoSided);
    bool GetTwoSided();
    
    void SetAmbientColor(const Color & color);
    void SetDiffuseColor(const Color & color);
    void SetSpecularColor(const Color & color);
    void SetEmissiveColor(const Color & color);
        
    const Color & GetAmbientColor() const;
    const Color & GetDiffuseColor() const;
    const Color & GetSpecularColor() const;
    const Color & GetEmissiveColor() const;
    
    void SetShininess(float32 shininess);
    float32 GetShininess() const;

	void SetSetupLightmap(bool isSetupLightmap);
	bool GetSetupLightmap();
	void SetSetupLightmapSize(int32 setupLightmapSize);

    /**
        \brief Bind material to render system.
        Function should be used if you want to render something with this material.
     */
    //void BindMaterial();
	void PrepareRenderState(InstanceMaterialState * instanceMaterialState = 0);
    void Draw(PolygonGroup * group, InstanceMaterialState * state);
    
    /**
        \brief Unbind material. 
        Restore some default properties that can influence to rendering in the future.
     */
    //void UnbindMaterial();
    
    
//    eType   type; //TODO: waiting for enums at introspection
    uint8 type;

	Vector4 reflective;
	float32	reflectivity;

	Vector4 transparent;
	float	transparency; 
	float	indexOfRefraction;


    enum eTextureLevel
    {
        TEXTURE_DIFFUSE = 0,
        TEXTURE_DETAIL = 1,
        TEXTURE_DECAL = 1,
		TEXTURE_NORMALMAP = 2,
        
        TEXTURE_COUNT, 
    };    

    void Save(KeyedArchive * keyedArchive, SceneFileV2 * sceneFile);
    void Load(KeyedArchive * keyedArchive, SceneFileV2 * sceneFile);
    
    
    //void SetTextureSlotName(uint32 index, const String & string);

    uint32 GetTextureSlotCount() const;
    const String & GetTextureSlotName(uint32 index) const;
    uint32 GetTextureSlotIndexByName(const String & string) const;
    
    
    void SetTexture(eTextureLevel level, Texture * texture);
    void SetTexture(eTextureLevel level, const String & textureName);
	inline Texture * GetTexture(eTextureLevel level) const;
	inline const String & GetTextureName(eTextureLevel level) const;

	RenderStateBlock * GetRenderStateBlock();
    
    inline void SetUvOffset(const Vector2 & uvOffset);
    inline void SetUvScale(const Vector2 & uvScale);
    inline const Vector2 & GetUvOffset() const;
    inline const Vector2 & GetUvScale() const;
    
    inline void SetBlendSrc(eBlendMode _blendSrc);
    inline void SetBlendDest(eBlendMode _blendDest);
    inline eBlendMode GetBlendSrc() const;
    inline eBlendMode GetBlendDest() const;
    
private:
    void RetrieveTextureSlotNames();
    
    
    Texture * textures[TEXTURE_COUNT];
    String names[TEXTURE_COUNT];
    String textureSlotNames[TEXTURE_COUNT];
    uint32 textureSlotCount;
    
    Vector2 uvOffset;
	Vector2 uvScale;

//	eBlendMode blendSrc; //TODO: waiting for enums at introspection
//	eBlendMode blendDst; //TODO: waiting for enums at introspection
	uint8 blendSrc;
	uint8 blendDst;


    void RebuildShader();
    
    bool    isOpaque;  
    bool    isTwoSided;

	bool	isSetupLightmap;
	int32	setupLightmapSize;
    
    float32	shininess;
    
    Color ambientColor;
	Color diffuseColor;
	Color specularColor;
	Color emissiveColor;
    
    bool    isFogEnabled;
    float32 fogDensity;
    Color   fogColor;

	bool isAlphablend;
    
    bool isWireframe;
    
    Shader  * shader;
    
    int32 uniformTexture0;
    int32 uniformTexture1;
    int32 uniformLightPosition0;
    int32 uniformMaterialLightAmbientColor;
    int32 uniformMaterialLightDiffuseColor;
    int32 uniformMaterialLightSpecularColor;
    int32 uniformMaterialSpecularShininess;
    int32 uniformLightIntensity0;
    int32 uniformLightAttenuationQ;
	int32 uniformUvOffset;
	int32 uniformUvScale;
    int32 uniformFogDensity;
    int32 uniformFogColor;

	RenderStateBlock renderStateBlock;
    
    
    /*
        TODO: Uniform array, with set of all uniforms, with one set.
     */
    
    
    
    static UberShader * uberShader;
    
public:
    
    INTROSPECTION_EXTEND(Material, DataNode,
        //MEMBER(type, "Type", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(isOpaque, "Is Opaque", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(isTwoSided, "Is Two Sided", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(isSetupLightmap, "Is Setup Lightmap", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(setupLightmapSize, "Setup Lightmap Size", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(shininess, "Shininess", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)

        MEMBER(ambientColor, "Ambient Color", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(diffuseColor, "Diffuse Color", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(specularColor, "Specular Color", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(emissiveColor, "Emissive Color", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)

        MEMBER(isFogEnabled, "Is Fog Enabled", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(fogDensity, "Fog Density", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(fogColor, "Fog Color", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
                         
        MEMBER(isAlphablend, "Is Alphablended", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(blendSrc, "Blend Source", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(blendDst, "Blend Destination", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)

        MEMBER(isWireframe, "Is Wire Frame", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
                         
         MEMBER(type, "Type", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
                         
//        COLLECTION(names, "Names", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};

Texture * Material::GetTexture(eTextureLevel level) const
{
	DVASSERT(level < TEXTURE_COUNT);
	return textures[level];
}

inline const String & Material::GetTextureName(eTextureLevel level) const
{
	DVASSERT(level < TEXTURE_COUNT);
	return names[level];
}
    
inline void Material::SetUvOffset(const Vector2 & _uvOffset)
{
    uvOffset = _uvOffset;
}

inline void Material::SetUvScale(const Vector2 & _uvScale)
{
    uvScale = _uvScale;
}
inline const Vector2 & Material::GetUvOffset() const
{
    return uvOffset;
}
inline const Vector2 & Material::GetUvScale() const
{
    return uvScale;
}

inline void Material::SetBlendSrc(eBlendMode _blendSrc)
{
    blendSrc = _blendSrc;
}
inline void Material::SetBlendDest(eBlendMode _blendDest)
{
    blendDst = _blendDest;
}
inline eBlendMode Material::GetBlendSrc() const
{
    return (eBlendMode)blendSrc;
}
inline eBlendMode Material::GetBlendDest() const
{
    return (eBlendMode)blendDst;
}

};

#endif // __DAVAENGINE_MATERIAL_H__

