#pragma once

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <UI/UISystem.h>
#include <UI/Components/UISingleComponent.h>
#include <Functional/Signal.h>

namespace DAVA
{

class UIControl;
class UIComponent;
class UISpineBonesComponent;
class UISpineComponent;
class UIControlBackground;
class SpineBone;
class SpineSkeleton;

struct UISpineSingleComponent : public UISingleComponent
{
    UnorderedSet<UIControl*> spineModified;
    UnorderedSet<UIControl*> spineNeedReload;
    UnorderedSet<UIControl*> spineBonesModified;

    void Clear() override;
};

class UISpineSystem final : public UISystem
{
public:
    UISpineSystem();
    ~UISpineSystem() override;

    void RegisterControl(UIControl* control) override;
    void UnregisterControl(UIControl* control) override;
    void RegisterComponent(UIControl* control, UIComponent* component) override;
    void UnregisterComponent(UIControl* control, UIComponent* component) override;

    void OnControlVisible(UIControl* control) override;
    void OnControlInvisible(UIControl* control) override;

    void Process(float32 elapsedTime) override;

    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/> onAnimationStart;
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/> onAnimationFinish;
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/> onAnimationComplete;
    Signal<UISpineComponent* /*component*/, int32 /*trackIndex*/, const String& /*event*/> onAnimationEvent;

private:
    struct BoneLink
    {
        RefPtr<SpineBone> bone;
        RefPtr<UIControl> control;
    };

    struct SpineNode
    {
        RefPtr<UISpineComponent> spine;
        RefPtr<UISpineBonesComponent> bones;
        RefPtr<UIControlBackground> bg;
        RefPtr<SpineSkeleton> skeleton;
        Vector<BoneLink> boneLinks;
    };

    void AddNode(UISpineComponent* spine);
    void RemoveNode(UISpineComponent* spine);
    void BindBones(UISpineBonesComponent* bones);
    void UnbindBones(UISpineBonesComponent* bones);
    void BindBackground(UIControlBackground* bg);
    void UnbindBackground(UIControlBackground* bg);

    void BuildBoneLinks(SpineNode& node);

    Map<UIControl*, SpineNode> nodes;
};

}