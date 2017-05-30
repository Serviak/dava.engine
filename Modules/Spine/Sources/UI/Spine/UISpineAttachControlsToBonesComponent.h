#pragma once

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>
#include <UI/Components/UIComponent.h>

namespace DAVA
{
class UISpineAttachControlsToBonesComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UISpineAttachControlsToBonesComponent, UIComponent);
    IMPLEMENT_UI_COMPONENT(UISpineAttachControlsToBonesComponent);

public:
    struct AttachInfo
    {
        String boneName;
        String controlPath;

        bool operator==(const AttachInfo& other) const;
        bool operator!=(const AttachInfo& other) const;
    };

    UISpineAttachControlsToBonesComponent();
    UISpineAttachControlsToBonesComponent(const UISpineAttachControlsToBonesComponent& src);
    UISpineAttachControlsToBonesComponent& operator=(const UISpineAttachControlsToBonesComponent&) = delete;

    UISpineAttachControlsToBonesComponent* Clone() const override;

    const Vector<AttachInfo>& GetBinds() const;
    void SetBinds(const Vector<AttachInfo>& binds);

protected:
    ~UISpineAttachControlsToBonesComponent() override;

    const String& GetBindsAsString() const;
    void SetBindsFromString(const String& bindsStr);

private:
    Vector<AttachInfo> bonesBinds;
    String cachedBindsString;

    void MakeBindsString();
    void Modify();
};

inline const Vector<UISpineAttachControlsToBonesComponent::AttachInfo>& UISpineAttachControlsToBonesComponent::GetBinds() const
{
    return bonesBinds;
}

inline const String& UISpineAttachControlsToBonesComponent::GetBindsAsString() const
{
    return cachedBindsString;
}
}