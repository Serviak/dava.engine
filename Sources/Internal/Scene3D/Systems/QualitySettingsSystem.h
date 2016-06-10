#ifndef __DAVAENGINE_SCENE3D_QUALITYSETTINGSSYSTEM_H__
#define __DAVAENGINE_SCENE3D_QUALITYSETTINGSSYSTEM_H__

#include "Base/StaticSingleton.h"
#include "Base/FastNameMap.h"
#include "Math/Vector.h"
#include "Scene3D/Systems/ParticlesQualitySettings.h"

namespace DAVA
{
struct TextureQuality
{
    size_t albedoBaseMipMapLevel;
    size_t normalBaseMipMapLevel;
    Vector2 minSize;
    size_t weight;
};

struct MaterialQuality
{
    FastName qualityName;
    size_t weight;
};

struct LandscapeQuality
{
    union
    {
        struct
        {
            float32 normalMaxHeightError;
            float32 normalMaxPatchRadiusError;
            float32 normalMaxAbsoluteHeightError;

            float32 zoomMaxHeightError;
            float32 zoomMaxPatchRadiusError;
            float32 zoomMaxAbsoluteHeightError;
        };
        std::array<float32, 6> metricsArray;
    };
    bool morphing;
};

class QualitySettingsComponent;
class QualitySettingsSystem : public StaticSingleton<QualitySettingsSystem>
{
public:
    static const FastName QUALITY_OPTION_VEGETATION_ANIMATION;
    static const FastName QUALITY_OPTION_STENCIL_SHADOW;
    static const FastName QUALITY_OPTION_WATER_DECORATIONS;
    static const FastName QUALITY_OPTION_DISABLE_EFFECTS;
    static const FastName QUALITY_OPTION_LOD0_EFFECTS;

    static const FastName QUALITY_OPTION_DISABLE_FOG;
    static const FastName QUALITY_OPTION_DISABLE_FOG_ATMOSPHERE_ATTENUATION;
    static const FastName QUALITY_OPTION_DISABLE_FOG_ATMOSPHERE_SCATTERING;
    static const FastName QUALITY_OPTION_DISABLE_FOG_HALF_SPACE;

    QualitySettingsSystem();

    void Load(const FilePath& path);

    // textures quality
    size_t GetTextureQualityCount() const;
    FastName GetTextureQualityName(size_t index) const;

    FastName GetCurTextureQuality() const;
    void SetCurTextureQuality(const FastName& name);

    const TextureQuality* GetTxQuality(const FastName& name) const;

    // materials quality
    size_t GetMaterialQualityGroupCount() const;
    FastName GetMaterialQualityGroupName(size_t index) const;

    size_t GetMaterialQualityCount(const FastName& group) const;
    FastName GetMaterialQualityName(const FastName& group, size_t index) const;

    FastName GetCurMaterialQuality(const FastName& group) const;
    void SetCurMaterialQuality(const FastName& group, const FastName& quality);

    const MaterialQuality* GetMaterialQuality(const FastName& group, const FastName& quality) const;

    // sound quality
    size_t GetSFXQualityCount() const;
    FastName GetSFXQualityName(size_t index) const;

    FastName GetCurSFXQuality() const;
    void SetCurSFXQuality(const FastName& name);

    FilePath GetSFXQualityConfigPath(const FastName& name) const;
    FilePath GetSFXQualityConfigPath(size_t index) const;

    // landscape
    size_t GetLandscapeQualityCount() const;
    FastName GetLandscapeQualityName(size_t index) const;

    FastName GetCurLandscapeQuality() const;
    void SetCurLandscapeQuality(const FastName& name);

    const LandscapeQuality* GetLandscapeQuality(const FastName& name) const;

    // particles
    const ParticlesQualitySettings& GetParticlesQualitySettings() const;
    ParticlesQualitySettings& GetParticlesQualitySettings();

    // ------------------------------------------

    void EnableOption(const FastName& option, bool enabled);
    bool IsOptionEnabled(const FastName& option) const;
    int32 GetOptionsCount() const;
    FastName GetOptionName(int32 index) const;

    bool IsQualityVisible(const Entity* entity);

    void UpdateEntityAfterLoad(Entity* entity);

    bool GetAllowCutUnusedVertexStreams();
    void SetAllowCutUnusedVertexStreams(bool cut);

    void SetKeepUnusedEntities(bool keep);
    bool GetKeepUnusedEntities();

    //metal preview is set in resource editor for artist to see correct picture
    void SetMetalPreview(bool preview);
    bool GetMetalPreview();
    bool GetAllowMetalFeatures();

    void UpdateEntityVisibility(Entity* e);

protected:
    void UpdateEntityVisibilityRecursively(Entity* e, bool qualityVisible);

protected:
    struct TXQ
    {
        FastName name;
        TextureQuality quality;
    };

    struct MAGrQ
    {
        size_t curQuality;
        Vector<MaterialQuality> qualities;
    };

    struct SFXQ
    {
        FastName name;
        FilePath configPath;
    };

    struct LCQ
    {
        FastName name;
        LandscapeQuality quality;
    };

    // textures
    int32 curTextureQuality;
    Vector<TXQ> textureQualities;

    // materials
    FastNameMap<MAGrQ> materialGroups;

    //sounds
    int32 curSoundQuality;
    Vector<SFXQ> soundQualities;

    //landscape
    int32 curLandscapeQuality;
    Vector<LCQ> landscapeQualities;

    FastNameMap<bool> qualityOptions;

    ParticlesQualitySettings particlesQualitySettings;

    bool cutUnusedVertexStreams;

    bool keepUnusedQualityEntities; //for editor to prevent cutting entities with unused quality

    bool metalPreviewEnabled = false;
};

inline void QualitySettingsSystem::SetKeepUnusedEntities(bool keep)
{
    keepUnusedQualityEntities = keep;
}

inline bool QualitySettingsSystem::GetKeepUnusedEntities()
{
    return keepUnusedQualityEntities;
}

inline void QualitySettingsSystem::SetMetalPreview(bool preview)
{
    metalPreviewEnabled = preview;
}
inline bool QualitySettingsSystem::GetMetalPreview()
{
    return metalPreviewEnabled;
}
inline bool QualitySettingsSystem::GetAllowMetalFeatures()
{
    //metal is turned on in 3 cases: preview metal in editor, preview metal in render option or metal renderer backend initialized
    return metalPreviewEnabled || (Renderer::IsInitialized() && (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::PREVIEW_METAL_ON_GL) || (Renderer::GetAPI() == rhi::RHI_METAL)));
}
}

#endif //__DAVAENGINE_SCENE3D_QUALITYSETTINGSSYSTEM_H__
