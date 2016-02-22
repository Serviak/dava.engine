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


#ifndef __DAVAENGINE_SCENE_NODE_ANIMATION_H__
#define __DAVAENGINE_SCENE_NODE_ANIMATION_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneNodeAnimationKey.h"
namespace DAVA
{
/*
	TODO: efficient pack & unpack of animations (Vector / Quaternion spline approximation)
 */
class SceneNodeAnimationList;
class SceneNodeAnimation : public BaseObject
{
protected:
    virtual ~SceneNodeAnimation();

public:
    SceneNodeAnimation(uint32 keyCount);

    SceneNodeAnimationKey& Intepolate(float32 t);

    void SetKey(int32 index, const SceneNodeAnimationKey& key);

    inline int32 GetKeyCount();
    inline SceneNodeAnimationKey* GetKeys();

    void SetDuration(float32 _duration);
    inline float32 GetDuration();

    void SetBindName(const FastName& bindName);
    void SetBindNode(Entity* bindNode);

    void SetInvPose(const Matrix4& mat);
    const Matrix4& GetInvPose() const;

    virtual void Update(float32 timeElapsed);
    virtual void Execute();

    inline float32 GetCurrentTime();
    inline float32 GetNormalDuration();

    Vector3 SetStartPosition(const Vector3& position);
    void ShiftStartPosition(const Vector3& position);

    void SetParent(SceneNodeAnimationList* list);
    SceneNodeAnimationList* GetParent();

    // this is node of animation this animation is supposed for
    Entity* bindNode = nullptr;
    FastName bindName;
    float32 weight = 0.0f;
    float32 delayTime = 0.0f;
    float32 currentTime = 0.0f;
    float32 duration = 0.0f;
    int32 startIdx = 0;
    uint32 keyCount = 0;
    SceneNodeAnimationKey* keys = nullptr;
    SceneNodeAnimationKey currentValue;
    Matrix4 invPose;
    bool apply = true;

private:
    SceneNodeAnimationList* parent = nullptr;
};

inline float32 SceneNodeAnimation::GetCurrentTime()
{
    return currentTime;
}

inline float32 SceneNodeAnimation::GetDuration()
{
    return duration;
}

inline float32 SceneNodeAnimation::GetNormalDuration()
{
    if (keyCount == 0)
        return 0;

    return keys[keyCount - 1].time;
}

inline int32 SceneNodeAnimation::GetKeyCount()
{
    return keyCount;
}

inline SceneNodeAnimationKey* SceneNodeAnimation::GetKeys()
{
    return keys;
}
};

#endif // __DAVAENGINE_SCENE_NODE_ANIMATION_KEY_H__
