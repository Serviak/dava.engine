#pragma once

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <UI/UISystem.h>
#include <Functional/Signal.h>

namespace DAVA
{

class UIControl;
class UIComponent;
class UISpineComponent;
class UIControlBackground;
class SpineSkeleton;

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
    struct SpineNode
    {
        UISpineComponent* component = nullptr;
        RefPtr<SpineSkeleton> skeleton;
    };

    void AddNode(UISpineComponent* component);
    void RemoveNode(UISpineComponent* component);
    
    Vector<SpineNode> nodes;
};

}