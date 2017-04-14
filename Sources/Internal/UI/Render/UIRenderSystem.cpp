#include "UIRenderSystem.h"
//#include "UI/Render/UIDebugDrawComponent.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "UI/UIControl.h"
#include "UI/UIScreenshoter.h"
#include "Debug/ProfilerMarkerNames.h"

namespace DAVA
{
UIRenderSystem::UIRenderSystem(RenderSystem2D* _renderSystem2D)
    : renderSystem2D(_renderSystem2D)
    , screenshoter(std::make_unique<UIScreenshoter>())
{
    baseGeometricData.position = Vector2(0, 0);
    baseGeometricData.size = Vector2(0, 0);
    baseGeometricData.pivotPoint = Vector2(0, 0);
    baseGeometricData.scale = Vector2(1.0f, 1.0f);
    baseGeometricData.angle = 0;
}

UIRenderSystem::~UIRenderSystem() = default;

void UIRenderSystem::OnControlVisible(UIControl* control)
{
    if (dynamic_cast<UI3DView*>(control) != nullptr)
    {
        ++ui3DViewCount;
    }
}

void UIRenderSystem::OnControlInvisible(UIControl* control)
{
    if (dynamic_cast<UI3DView*>(control) != nullptr)
    {
        DVASSERT(ui3DViewCount > 0);
        --ui3DViewCount;
    }
}

void UIRenderSystem::Process(float32 elapsedTime)
{
    RenderSystem2D::RenderTargetPassDescriptor newDescr = renderSystem2D->GetMainTargetDescriptor();
    newDescr.clearTarget = (ui3DViewCount == 0 || currentScreenTransition.Valid()) && needClearMainPass;
    renderSystem2D->SetMainTargetDescriptor(newDescr);
}

void UIRenderSystem::Render()
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_RENDER_SYSTEM);

    if (currentScreenTransition.Valid())
    {
        DrawControlHierarhy(currentScreenTransition.Get(), baseGeometricData, nullptr);
    }
    else if (currentScreen.Valid())
    {
        DrawControlHierarhy(currentScreen.Get(), baseGeometricData, nullptr);
    }

    if (popupContainer.Valid())
    {
        DrawControlHierarhy(popupContainer.Get(), baseGeometricData, nullptr);
    }

    screenshoter->OnFrame();
}

void UIRenderSystem::ManualRender(UIControl* control)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::UI_RENDER_SYSTEM);
    if (control == nullptr)
        return;

    DrawControlHierarhy(control, baseGeometricData, nullptr);
}

const UIGeometricData& UIRenderSystem::GetBaseGeometricData() const
{
    return baseGeometricData;
}

UIScreenshoter* UIRenderSystem::GetScreenshoter() const
{
    return screenshoter.get();
}

int32 UIRenderSystem::GetUI3DViewCount() const
{
    return ui3DViewCount;
}

void UIRenderSystem::SetClearColor(const Color& clearColor)
{
    RenderSystem2D::RenderTargetPassDescriptor newDescr = renderSystem2D->GetMainTargetDescriptor();
    newDescr.clearColor = clearColor;
    renderSystem2D->SetMainTargetDescriptor(newDescr);
}

void UIRenderSystem::SetUseClearPass(bool useClearPass)
{
    needClearMainPass = useClearPass;
}

void UIRenderSystem::SetCurrentScreen(const RefPtr<UIScreen>& _screen)
{
    currentScreen = _screen;
}

void UIRenderSystem::SetCurrentScreenTransition(const RefPtr<UIScreenTransition>& _screenTransition)
{
    currentScreenTransition = _screenTransition;
}

void UIRenderSystem::SetPopupContainer(const RefPtr<UIControl>& _popupContainer)
{
    popupContainer = _popupContainer;
}

void UIRenderSystem::DrawControlHierarhy(UIControl* control, const UIGeometricData& geometricData, const UIControlBackground* parentBackground)
{
    if (!control->GetVisibilityFlag())
        return;

    UIGeometricData drawData = control->GetLocalGeometricData();
    drawData.AddGeometricData(geometricData);

    const Color& parentColor = parentBackground ? parentBackground->GetDrawColor() : Color::White;

    control->SetParentColor(parentColor);

    const Rect& unrotatedRect = drawData.GetUnrotatedRect();

    if (control->GetClipContents())
    { //WARNING: for now clip contents don't work for rotating controls if you have any ideas you are welcome
        renderSystem2D->PushClip();
        renderSystem2D->IntersectClipRect(unrotatedRect); //anyway it doesn't work with rotation
    }

    control->Draw(drawData);

    const UIControlBackground* bg = control->GetComponent<UIControlBackground>();
    const UIControlBackground* parentBgForChild = bg ? bg : parentBackground;
    control->isIteratorCorrupted = false;
    for (UIControl* child : control->GetChildren())
    {
        DrawControlHierarhy(child, drawData, parentBgForChild);
        DVASSERT(!control->isIteratorCorrupted);
    }

    control->DrawAfterChilds(drawData);

    if (control->GetClipContents())
    {
        renderSystem2D->PopClip();
    }

    if (control->GetDebugDraw())
    {
        DebugDraw(control, drawData, unrotatedRect);
    }
}

void UIRenderSystem::DebugDraw(UIControl* control, UIGeometricData drawData, const Rect& unrotatedRect)
{
    renderSystem2D->PushClip();
    renderSystem2D->RemoveClip();
    DrawDebugRect(control, drawData, false);
    UIControl::eDebugDrawPivotMode drawMode = control->GetDrawPivotPointMode();
    if (drawMode != UIControl::DRAW_NEVER &&
        (drawMode != UIControl::DRAW_ONLY_IF_NONZERO || !control->GetPivotPoint().IsZero()))
    {
        DrawPivotPoint(control, unrotatedRect);
    }
    renderSystem2D->PopClip();
}

void UIRenderSystem::DrawDebugRect(UIControl* control, const UIGeometricData& gd, bool useAlpha)
{
    auto drawColor = control->GetDebugDrawColor();
    if (useAlpha)
    {
        drawColor.a = 0.4f;
    }

    if (gd.angle != 0.0f)
    {
        Polygon2 poly;
        gd.GetPolygon(poly);

        renderSystem2D->DrawPolygon(poly, true, drawColor);
    }
    else
    {
        renderSystem2D->DrawRect(gd.GetUnrotatedRect(), drawColor);
    }
}

void UIRenderSystem::DrawPivotPoint(UIControl* control, const Rect& drawRect)
{
    static const float32 PIVOT_POINT_MARK_RADIUS = 10.0f;
    static const float32 PIVOT_POINT_MARK_HALF_LINE_LENGTH = 13.0f;
    static const Color drawColor(1.0f, 0.0f, 0.0f, 1.0f);

    Vector2 pivotPointCenter = drawRect.GetPosition() + control->GetPivotPoint();
    renderSystem2D->DrawCircle(pivotPointCenter, PIVOT_POINT_MARK_RADIUS, drawColor);

    // Draw the cross mark.
    Vector2 lineStartPoint = pivotPointCenter;
    Vector2 lineEndPoint = pivotPointCenter;
    lineStartPoint.y -= PIVOT_POINT_MARK_HALF_LINE_LENGTH;
    lineEndPoint.y += PIVOT_POINT_MARK_HALF_LINE_LENGTH;
    renderSystem2D->DrawLine(lineStartPoint, lineEndPoint, drawColor);

    lineStartPoint = pivotPointCenter;
    lineEndPoint = pivotPointCenter;
    lineStartPoint.x -= PIVOT_POINT_MARK_HALF_LINE_LENGTH;
    lineEndPoint.x += PIVOT_POINT_MARK_HALF_LINE_LENGTH;
    renderSystem2D->DrawLine(lineStartPoint, lineEndPoint, drawColor);
}
}