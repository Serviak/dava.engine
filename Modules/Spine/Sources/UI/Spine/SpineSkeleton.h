#pragma once

namespace DAVA
{

class SpineSkeleton final : public BaseObject
{
public:
    // Spine types. Maybe add some wrappers for they in public API.
    class TackEntry;
    class Bone;

    /** Default contructor. */
    SpineSkeleton();

    /** Load Spine skeleton from binary/json data file and atlas. */
    void Load(const FilePath& dataPath, const FilePath& atlasPath);

    /** Update state of Spine skeleton with specified time delta. */
    void Update(const flaot32 timeElapsed);

    /** Reset all Spine skeleton's states. */
    void ResetSkeleton();

    /** Return render batch data for draw current state of Spine skeleton in UIControlBackground. */
    const RenderSystem2D::BatchDescriptor& GetRenderBatch() const;

    /** Return list of names of available animations. */
    Vector<String> GetAvailableAnimationsNames() const;
    /**
    Set current animation by specified name, track index and looping flag.
    Return track data of setted animation.
    */
    TrackEntry* SetAnimation(int32 trackIndex, const String& name, bool loop);
    /**
    Add next animation by specified name, track index and looping flag.
    Return track data of added animation.
    */
    TrackEntry* AddAnimation(int32 trackIndex, const String& name, bool loop, float32 delay);
    /** Return track data by specified track index. */
    TrackEntry* GetTrack(int32 trackIndex);
    /** Create translation animation between two animation states specified by names. */
    void SetAnimationMix(const String& fromAnimation, const String& toAnimation, float32 duration);
    /** Clear all animation tracs and stop any animtaion. */
    void ClearTracks();
    /** Remove track from animations queue by specified track's index. */
    void CleatTrack(int32 trackIndex);

    /** Set time scale for change animation speed. */
    void SetTimeScale(float32 timeScale);
    /** Get current time scale. */
    float32 GetTimeScale() const;

    /** Select skin by specified skin's name. */
    void SetSkin(const String& skinName);
    /** Select skin by specified skin's index. */
    void SetSkin(int32 skinNumber);
    /** Get index of current skin. */
    int32 GetSkinNumber();

    /** Find bone data by specified bone's name. Return nullptr if bone not found. */
    Bone* FindBone(const String& boneName);

    /** Emit on each event during spine animation. */
    Signal<const String& /*event*/> onEvent;

protected:
    ~SpineSkeleton() override;

};

}