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

namespace DAVA
{
// Dava types forward declaration
struct BatchDescriptor;
class UIControl;
class Texture;
class SpineBone;
class SpineTrackEntry;

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
    std::shared_ptr<SpineTrackEntry> SetAnimation(int32 trackIndex, const String& name, bool loop);
    /** Add next animation by specified name, track index and looping flag.
        Return track data of added animation. */
    std::shared_ptr<SpineTrackEntry> AddAnimation(int32 trackIndex, const String& name, bool loop, float32 delay);
    /** Return track data by specified track index. */
    std::shared_ptr<SpineTrackEntry> GetTrack(int32 trackIndex);
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
    /** Return current skin name. */
    String GetSkinName() const;
    /** Return list of names of available skins. */
    const Vector<String>& GetAvailableSkinsNames() const;

    /** Find bone data by specified bone's name. Return nullptr if bone not found. */
    std::shared_ptr<SpineBone> FindBone(const String& boneName);

    /** Emit then animation start.
    Start is raised when an animation starts playing:
    - This applies to right when you call SetAnimation.
    - I can also be raised when a queued animation starts playing.
    */
    Signal<int32 /*trackIndex*/> onStart;
    /** Emit then all animations finish.
    End is raised when an animation is cleared (or interrupted):
    - This applies to immediately after the animation is done mixing/transitioning out.
    - After end is fired, that entry will never be applied again.
    - This is also raised when you clear the track using ClearTrack or ClearTracks.
    - *NEVER* handle the End event with a method that calls SetAnimation. See the warning below.
    */
    Signal<int32 /*trackIndex*/> onFinish;
    /** Emit then one animation in queue finish.
    Complete is raised an animation completes its full duration:
    - This is raised when a non-looping animation finishes playing, whether or not a next animation is queued.
    - This is also raised every time a looping animation finishes an loop.
    */
    Signal<int32 /*trackIndex*/> onComplete;
    /** Emit on each event during spine animation.
    Event is raised whenever ANY user-defined event is detected:
    - These are events you keyed in animations in Spine editor.
      They are purple keys. A purple icon can also be found in the Tree view.
    - To distinguish between different events, you need to check for its `event` string.
    - This is useful for when you have to play sounds according to points
      the animation like footsteps. It can also be used to synchronize or
      signal non-Spine systems according to Spine animations.
    - During a transition/mixing out, this is fired depending on the TrackEntry::EventThreshold.
    */
    Signal<int32 /*trackIndex*/, const String& /*event*/> onEvent;

protected:
    ~SpineSkeleton() override;

private:
    void ReleaseAtlas();
    void ReleaseSkeleton();

    BatchDescriptor* batchDescriptor = nullptr;

    Vector<String> animationsNames;
    Vector<String> skinsNames;

    spAtlas* atlas = nullptr;
    spSkeleton* skeleton = nullptr;
    spAnimationState* state = nullptr;

    float32* worldVertices = nullptr;
    Texture* currentTexture = nullptr;
    float32 timeScale = 1.0f;
    Vector<Vector2> verticesUVs;
    Vector<uint16> clippedIndecex;
    Vector<uint32> verticesColors;
    Polygon2 verticesPolygon;
};
}