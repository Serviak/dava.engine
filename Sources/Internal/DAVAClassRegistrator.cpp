#include "DAVAEngine.h"
#include "DAVAClassRegistrator.h"
#include "Render/Highlevel/ShadowVolume.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UI/Layouts/UILinearLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutHintComponent.h"
#include "UI/Layouts/UIIgnoreLayoutComponent.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UIAnchorComponent.h"
#include "UI/Input/UIModalInputComponent.h"
#include "UI/Focus/UIFocusComponent.h"
#include "UI/Focus/UIFocusGroupComponent.h"
#include "UI/Focus/UINavigationComponent.h"
#include "UI/Focus/UITabOrderComponent.h"
#include "UI/Input/UIActionComponent.h"
#include "UI/Input/UIActionBindingComponent.h"
#include "UI/Scroll/UIScrollBarDelegateComponent.h"

using namespace DAVA;

void DAVA::RegisterDAVAClasses()
{
    //this code do nothing. Needed to compiler generate code from this cpp file
    Logger* log = Logger::Instance();
    if (log)
        log->Log(Logger::LEVEL__DISABLE, "");

    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIControlBackground, "Background");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UILinearLayoutComponent, "LinearLayout");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIFlowLayoutComponent, "FlowLayout");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIFlowLayoutHintComponent, "FlowLayoutHint");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIIgnoreLayoutComponent, "IgnoreLayout");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UISizePolicyComponent, "SizePolicy");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIAnchorComponent, "Anchor");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIModalInputComponent, "ModalInput");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIFocusComponent, "Focus");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIFocusGroupComponent, "FocusGroup");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UINavigationComponent, "Navigation");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UITabOrderComponent, "TabOrder");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIActionComponent, "Action");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIActionBindingComponent, "ActionBinding");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIScrollBarDelegateComponent, "ScrollBarDelegate");
}

#if !defined(__DAVAENGINE_ANDROID__)
REGISTER_CLASS(TheoraPlayer);
#endif

REGISTER_CLASS(BaseObject);
REGISTER_CLASS(PolygonGroup);
REGISTER_CLASS(StaticMesh);
REGISTER_CLASS(Camera);
REGISTER_CLASS(UIScrollViewContainer);
REGISTER_CLASS(UISlider);
REGISTER_CLASS(UISpinner);
REGISTER_CLASS(UIStaticText);
REGISTER_CLASS(UISwitch);
REGISTER_CLASS(UITextField);
REGISTER_CLASS(Landscape);
REGISTER_CLASS(AnimationData);
REGISTER_CLASS(Light);
REGISTER_CLASS(Mesh);
REGISTER_CLASS(SkinnedMesh);
REGISTER_CLASS(SpeedTreeObject);
REGISTER_CLASS(RenderBatch);
REGISTER_CLASS(RenderObject);
REGISTER_CLASS(ShadowVolume);
REGISTER_CLASS(NMaterial);
REGISTER_CLASS(DataNode);
REGISTER_CLASS(Entity);
REGISTER_CLASS(Scene);
REGISTER_CLASS(UIButton);
REGISTER_CLASS(UIControl);
REGISTER_CLASS(UIList);
REGISTER_CLASS(UIListCell);
REGISTER_CLASS(UIScrollBar);
REGISTER_CLASS(UIScrollView);
REGISTER_CLASS_WITH_ALIAS(PartilceEmitterLoadProxy, "ParticleEmitter3D");
REGISTER_CLASS(UIWebView);
REGISTER_CLASS(UIMovieView);
REGISTER_CLASS(UIParticles);
REGISTER_CLASS(UIJoypad);
REGISTER_CLASS(VegetationRenderObject);
REGISTER_CLASS(BillboardRenderObject);
REGISTER_CLASS(SpriteObject);
REGISTER_CLASS(UI3DView);
REGISTER_CLASS(AnimationComponent);
REGISTER_CLASS(TransformComponent);
REGISTER_CLASS(UpdatableComponent);
REGISTER_CLASS(RenderComponent);
REGISTER_CLASS(CustomPropertiesComponent);
REGISTER_CLASS(ActionComponent);
REGISTER_CLASS(DebugRenderComponent);
REGISTER_CLASS(SoundComponent);
REGISTER_CLASS(BulletComponent);
REGISTER_CLASS(LightComponent);
REGISTER_CLASS(SpeedTreeComponent);
REGISTER_CLASS(WindComponent);
REGISTER_CLASS(WaveComponent);
REGISTER_CLASS(QualitySettingsComponent);
REGISTER_CLASS(UserComponent);
REGISTER_CLASS(SwitchComponent);
REGISTER_CLASS(ParticleEffectComponent);
REGISTER_CLASS(CameraComponent);
REGISTER_CLASS(StaticOcclusionComponent);
REGISTER_CLASS(StaticOcclusionDataComponent);
REGISTER_CLASS(PathComponent);
REGISTER_CLASS(WASDControllerComponent);
REGISTER_CLASS(RotationControllerComponent);
REGISTER_CLASS(SnapToLandscapeControllerComponent);
