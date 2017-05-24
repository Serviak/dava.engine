#pragma once

#include <Base/BaseTypes.h>
#include <Base/BaseObject.h>
#include <FileSystem/FilePath.h>
#include <Functional/Signal.h>
#include <Math/Polygon2.h>

// Spine types forward declaration
struct spAtlas;
struct spSkeleton;
struct spAnimationState;
struct spActionsData;
struct spBone;

namespace DAVA
{
// Dava types forward declaration
struct BatchDescriptor;
class UIControl;
class Texture;

// Public wrappers
class SpineTrackEntry
{
};

class SpineBone : public BaseObject
{
public:
    SpineBone(spBone* bone);

    bool IsValid() const;
    Vector2 GetPosition() const;
    Vector2 GetScale() const;
    float32 GetAngle() const;

    //Color GetColor() const;

protected:
    ~SpineBone();

private:
    spBone* bonePtr = nullptr;
};

/**
    
*/
class SpineSkeleton final : public BaseObject
{
public:
    /** Default constructor. */
    SpineSkeleton();
    /** Copy constructor. */
    SpineSkeleton(const SpineSkeleton& src) = delete;

    SpineSkeleton& operator=(const SpineSkeleton&) = delete;

    /** Load Spine skeleton from binary/json data file and atlas. */
    void Load(const FilePath& dataPath, const FilePath& atlasPath);

    /** Update state of Spine skeleton with specified time delta. */
    void Update(const float32 timeElapsed);

    /** Reset all Spine skeleton's states. */
    void ResetSkeleton();
    /** Return render batch data for draw current state of Spine skeleton in UIControlBackground. */
    BatchDescriptor* GetRenderBatch() const;

    /** Return list of names of available animations. */
    const Vector<String>& GetAvailableAnimationsNames() const;
    /** Set current animation by specified name, track index and looping flag.
        Return track data of setted animation. */
    SpineTrackEntry* SetAnimation(int32 trackIndex, const String& name, bool loop);
    /** Add next animation by specified name, track index and looping flag.
        Return track data of added animation. */
    SpineTrackEntry* AddAnimation(int32 trackIndex, const String& name, bool loop, float32 delay);
    /** Return track data by specified track index. */
    SpineTrackEntry* GetTrack(int32 trackIndex);
    /** Create translation animation between two animation states specified by names. */
    void SetAnimationMix(const String& fromAnimation, const String& toAnimation, float32 duration);
    /** Clear all animation tracks and stop any animation. */
    void ClearTracks();
    /** Remove track from animations queue by specified track's index. */
    void CleatTrack(int32 trackIndex);

    /** Set time scale for change animation speed. */
    void SetTimeScale(float32 timeScale);
    /** Get current time scale. */
    float32 GetTimeScale() const;

    /** Select skin by specified skin's name. */
    bool SetSkin(const String& skinName);
    /** Return list of names of available skins. */
    const Vector<String>& GetAvailableSkinsNames() const;

    /** Find bone data by specified bone's name. Return nullptr if bone not found. */
    RefPtr<SpineBone> FindBone(const String& boneName);

    /** Emit then animation start */
    Signal<int32 /*trackIndex*/> onStart;
    /** Emit then all animations finish */
    Signal<int32 /*trackIndex*/> onFinish;
    /** Emit then one animation in queue finish */
    Signal<int32 /*trackIndex*/> onComplete;
    /** Emit on each event during spine animation. */
    Signal<int32 /*trackIndex*/, const String& /*event*/> onEvent;

protected:
    ~SpineSkeleton() override;

private:
    void ReleaseAtlas();
    void ReleaseSkeleton();

    BatchDescriptor* batchDescriptor = nullptr;

    FilePath mPath;
    FilePath mSequencePath;

    Vector<String> mAnimations;
    Vector<String> mSkins;
    int32 mAnimationType = 0;
    bool mRun = false;
    bool mLoop = false;
    bool mNeedInitialize = false;

    spActionsData* mActionsData = nullptr;
    spAtlas* mAtlas = nullptr;
    spSkeleton* mSkeleton = nullptr;
    spAnimationState* mState = nullptr;

    float32* mWorldVertices = nullptr;
    Texture* mTexture = nullptr;
    float32 mTimeScale = 1.0f;
    Vector<Vector2> mUVs;
    Vector<uint16> mSpriteClippedIndecex;
    Vector<uint32> mColors;
    Polygon2 mPolygon;
};
}