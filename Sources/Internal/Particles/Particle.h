#ifndef __DAVAENGINE_PARTICLE_H__
#define __DAVAENGINE_PARTICLE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/AllocatorFactory.h"

namespace DAVA
{
struct StripeNode
{
    float32 lifeime = 0.0f;
    Vector3 position = {};
    Vector3 speed = {};

    StripeNode(float32 lifetime_, Vector3 position_, Vector3 speed_)
        : lifeime(lifetime_)
        , position(position_)
        , speed(speed_)
    {}

    StripeNode()
    {}
};

struct StripeData
{
    List<StripeNode> strpeNodes; // List of stripe control points.
    StripeNode baseNode = {};
    float32 spawnTimer = 0;
    Vector3 inheritPositionOffset = {};
};

struct Particle
{
    IMPLEMENT_POOL_ALLOCATOR(Particle, 1000);

    Particle* next;

    float32 life;
    float32 lifeTime;

    Vector3 position;
    Vector3 speed;

    float32 angle;
    float32 spin;

    int32 frame;
    float32 animTime;

    float32 baseFlowSpeed;
    float32 currFlowSpeed;
    float32 baseFlowOffset;
    float32 currFlowOffset;

    float32 baseNoiseScale;
    float32 currNoiseScale;
    float32 baseNoiseUScrollSpeed;
    float32 currNoiseUOffset;
    float32 baseNoiseVScrollSpeed;
    float32 currNoiseVOffset;

    float32 fresnelToAlphaPower;
    float32 fresnelToAlphaBias;

    float currRadius; //for bbox computation
    Vector2 baseSize, currSize;

    Color color;

    int32 positionTarget; //superemitter particles only

    StripeData stripe;
};
}

#endif // __DAVAENGINE_PARTICLE_H__
