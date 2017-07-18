#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Base/DynamicObjectCache.h"
#include "Render/2D/Sprite.h"

#include "FileSystem/YamlParser.h"
#include "Particles/Particle.h"
#include "Particles/ParticleForce.h"
#include "Particles/ParticlePropertyLine.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class ParticleEmitter;

/**
    In most cases you'll not need to use this class directly 
    and should use ParticleEffect instead. 
    
    Few cases when you actually need ParticleLayers: 
    - You want to get information about layer lifeTime or layer sprite
    - You want to change something on the fly inside layer
 */
struct ParticleLayer : public BaseObject
{
    float32 stripeLifetime = 20.0f;
    float32 stripeRate = 0.5f;
    float32 stripeStartSize = 1.0f;
    RefPtr<PropertyLine<float32>> stripeSizeOverLife;
    RefPtr<PropertyLine<float32>> stripeTextureTileOverLife;
    RefPtr<PropertyLine<float32>> stripeNoiseUScrollSpeedOverLife;
    RefPtr<PropertyLine<float32>> stripeNoiseVScrollSpeedOverLife;
    float32 stripeUScrollSpeed = 0.0f;
    float32 stripeVScrollSpeed = -0.01f;
    float32 stripeFadeDistanceFromTop = 0.0f;
    RefPtr<PropertyLine<Color>> stripeColorOverLife;
    bool usePerspectiveMapping = false;

    float32 maxStripeOverLife = 0.0f;
    bool isMaxStripeOverLifeDirty = true;

    enum eType
    {
        TYPE_SINGLE_PARTICLE,
        TYPE_PARTICLES, // default for any particle layer loaded from yaml file
        TYPE_PARTICLE_STRIPE,
        TYPE_SUPEREMITTER_PARTICLES
    };

    enum eParticleOrientation
    {
        PARTICLE_ORIENTATION_CAMERA_FACING = 1 << 0, //default
        PARTICLE_ORIENTATION_X_FACING = 1 << 1,
        PARTICLE_ORIENTATION_Y_FACING = 1 << 2,
        PARTICLE_ORIENTATION_Z_FACING = 1 << 3,
        PARTICLE_ORIENTATION_WORLD_ALIGN = 1 << 4,
        PARTICLE_ORIENTATION_CAMERA_FACING_STRIPE_SPHERICAL = 1 << 5
    };

    enum eDegradeStrategy
    {
        DEGRADE_KEEP = 0,
        DEGRADE_CUT_PARTICLES = 1,
        DEGRADE_REMOVE = 2
    };

    ParticleLayer();
    virtual ~ParticleLayer();
    virtual ParticleLayer* Clone();

    void LoadFromYaml(const FilePath& configPath, const YamlNode* node, bool preserveInheritPosition);
    void SaveToYamlNode(const FilePath& configPath, YamlNode* parentNode, int32 layerIndex);

    void SaveSpritePath(FilePath& path, const FilePath& configPath, YamlNode* layerNode, std::string name);

    void SaveForcesToYamlNode(YamlNode* layerNode);

    void AddForce(ParticleForce* force);
    void RemoveForce(ParticleForce* force);
    void RemoveForce(int32 forceIndex);
    void CleanupForces();
    float32 CalculateMaxStripeSizeOverLife();

    void GetModifableLines(List<ModifiablePropertyLineBase*>& modifiables);

    // Convert from Layer Type to its name and vice versa.
    eType StringToLayerType(const String& layerTypeName, eType defaultLayerType);
    String LayerTypeToString(eType layerType, const String& defaultLayerTypeName);

    void UpdateLayerTime(float32 startTime, float32 endTime);

    bool IsLodActive(int32 lod);
    void SetLodActive(int32 lod, bool active);

    ScopedPtr<Sprite> sprite;
    void SetSprite(const FilePath& spritePath);
    Vector2 layerPivotPoint;
    Vector2 layerPivotSizeOffsets; //precached for faster bbox computation
    void SetPivotPoint(Vector2 pivot);
    FilePath spritePath;

    FilePath flowmapPath;
    ScopedPtr<Sprite> flowmap;
    bool enableFlow = false;
    bool enableFlowAnimation = false;
    void SetFlowmap(const FilePath& spritePath_);

    FilePath noisePath;
    ScopedPtr<Sprite> noise;
    bool enableNoise = false;
    bool enableNoiseScroll = false;
    void SetNoise(const FilePath& spritePath_);

    //////////////////////////////////////////////////////////////////////////
    float32 alphaRemapLoopCount = 1.0f;
    FilePath alphaRemapPath;
    ScopedPtr<Sprite> alphaRemapSprite;
    bool enableAlphaRemap = false;
    void SetAlphaRemap(const FilePath& spritePath_);
    RefPtr<PropertyLine<float32>> alphaRemapOverLife;
    //////////////////////////////////////////////////////////////////////////

    bool isLooped;
    bool isLong;
    bool useFresnelToAlpha = false;
    eBlending blending;
    bool enableFog;
    bool enableFrameBlend;

    bool isDisabled;

    Vector<bool> activeLODS;

    String layerName;

    /*
     Properties of particle layer that describe particle system logic
     */
    RefPtr<PropertyLine<float32>> life; // in seconds
    RefPtr<PropertyLine<float32>> lifeVariation; // variation part of life that added to particle life during generation of the particle

    // Flow
    RefPtr<PropertyLine<float32>> flowSpeed;
    RefPtr<PropertyLine<float32>> flowSpeedVariation;

    RefPtr<PropertyLine<float32>> flowOffset;
    RefPtr<PropertyLine<float32>> flowOffsetVariation;

    // Noise
    RefPtr<PropertyLine<float32>> noiseScale;
    RefPtr<PropertyLine<float32>> noiseScaleVariation;
    RefPtr<PropertyLine<float32>> noiseScaleOverLife;

    RefPtr<PropertyLine<float32>> noiseUScrollSpeed;
    RefPtr<PropertyLine<float32>> noiseUScrollSpeedVariation;
    RefPtr<PropertyLine<float32>> noiseUScrollSpeedOverLife; // Noise texcoord u scrollSpeed;

    RefPtr<PropertyLine<float32>> noiseVScrollSpeed;
    RefPtr<PropertyLine<float32>> noiseVScrollSpeedVariation;
    RefPtr<PropertyLine<float32>> noiseVScrollSpeedOverLife; // Noise texcoord v scrollSpeed;

    // Number
    RefPtr<PropertyLine<float32>> number; // number of particles per second
    RefPtr<PropertyLine<float32>> numberVariation; // variation part of number that added to particle count during generation of the particle

    RefPtr<PropertyLine<Vector2>> size; // size of particles in pixels
    RefPtr<PropertyLine<Vector2>> sizeVariation; // size variation in pixels
    RefPtr<PropertyLine<Vector2>> sizeOverLifeXY;

    RefPtr<PropertyLine<float32>> velocity; // velocity in pixels
    RefPtr<PropertyLine<float32>> velocityVariation;
    RefPtr<PropertyLine<float32>> velocityOverLife;

    Vector<ParticleForce*> forces;

    RefPtr<PropertyLine<float32>> spin; // spin of angle / second
    RefPtr<PropertyLine<float32>> spinVariation;
    RefPtr<PropertyLine<float32>> spinOverLife;
    bool randomSpinDirection;

    RefPtr<PropertyLine<Color>> colorRandom;
    RefPtr<PropertyLine<float32>> alphaOverLife;
    RefPtr<PropertyLine<Color>> colorOverLife;

    RefPtr<PropertyLine<float32>> angle; // sprite angle in degrees
    RefPtr<PropertyLine<float32>> angleVariation; // variations in degrees

    RefPtr<PropertyLine<float32>> animSpeedOverLife;

    float32 startTime;
    float32 endTime;
    // Layer loop paremeters
    float32 deltaTime;
    float32 deltaVariation;
    float32 loopVariation;
    float32 loopEndTime;

    eType type;

    eDegradeStrategy degradeStrategy;

    int32 particleOrientation;

    bool frameOverLifeEnabled;
    float32 frameOverLifeFPS;
    bool randomFrameOnStart;
    bool loopSpriteAnimation;

    //for long particles
    float32 scaleVelocityBase;
    float32 scaleVelocityFactor;

    float32 fresnelToAlphaBias;
    float32 fresnelToAlphaPower;

    ParticleEmitter* innerEmitter = nullptr;
    FilePath innerEmitterPath;

    bool GetInheritPosition() const;
    void SetInheritPosition(bool inheritPosition_);

    bool GetInheritPositionForStripeBase() const;
    void SetInheritPositionForStripeBase(bool inheritPositionForBase_);

private:
    struct LayerTypeNamesInfo
    {
        eType layerType;
        String layerTypeName;
    };
    static const LayerTypeNamesInfo layerTypeNamesInfoMap[];

    void FillSizeOverlifeXY(RefPtr<PropertyLine<float32>> sizeOverLife);
    void UpdateSizeLine(PropertyLine<Vector2>* line, bool rescaleSize, bool swapXY); //conversion from old format

    bool stripeInheritPositionOnlyForBaseVertex = false; // For stripe particles. Move only base vertex when in stripe.
    bool inheritPosition; //for super emitter - if true the whole emitter would be moved, otherwise just emission point

public:
    INTROSPECTION_EXTEND(ParticleLayer, BaseObject, nullptr);
};

inline float32 ParticleLayer::CalculateMaxStripeSizeOverLife()
{
    if (!isMaxStripeOverLifeDirty)
        return maxStripeOverLife;
    if (stripeSizeOverLife.Get() == nullptr)
        return 1.0f;

    isMaxStripeOverLifeDirty = false;
    Vector<PropertyLine<float32>::PropertyKey>& keys = stripeSizeOverLife->GetValues();
    auto max = std::max_element(keys.begin(), keys.end(),
                                [](PropertyLine<float32>::PropertyKey& a, PropertyLine<float32>::PropertyKey& b)
                                {
                                    return a.value < b.value;
                                }
                                );
    maxStripeOverLife = (*max).value;
    return maxStripeOverLife;
}

inline bool ParticleLayer::GetInheritPosition() const
{
    return inheritPosition;
}

inline void ParticleLayer::SetInheritPosition(bool inheritPosition_)
{
    if (inheritPosition_)
        stripeInheritPositionOnlyForBaseVertex = false;

    inheritPosition = inheritPosition_;
}

inline bool ParticleLayer::GetInheritPositionForStripeBase() const
{
    return stripeInheritPositionOnlyForBaseVertex;
}

inline void ParticleLayer::SetInheritPositionForStripeBase(bool inheritPositionOnlyForBaseVertex_)
{
    if (inheritPositionOnlyForBaseVertex_)
        inheritPosition = false;

    stripeInheritPositionOnlyForBaseVertex = inheritPositionOnlyForBaseVertex_;
}
}
