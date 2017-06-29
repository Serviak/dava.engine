#include "LandscapeEditorSystem.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"

#include "Scene/SceneEditor2.h"
#include "Main/QtUtils.h"

LandscapeEditorSystem::LandscapeEditorSystem(DAVA::Scene* scene, const DAVA::FilePath& cursorPath)
    : DAVA::SceneSystem(scene)
    , cursorPathName(cursorPath)
    , cursorPosition(-100.f, -100.f)
    , prevCursorPos(-1.f, -1.f)
{
    cursorTexture = CreateSingleMipTexture(cursorPathName);
    cursorTexture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);

    collisionSystem = ((SceneEditor2*)GetScene())->collisionSystem;
    modifSystem = ((SceneEditor2*)GetScene())->modifSystem;
    drawSystem = ((SceneEditor2*)GetScene())->landscapeEditorDrawSystem;

    DAVA::Renderer::GetSignals().needRestoreResources.Connect(this, &LandscapeEditorSystem::RenderRestoreCallback);
}

LandscapeEditorSystem::~LandscapeEditorSystem()
{
    DAVA::Renderer::GetSignals().needRestoreResources.Disconnect(this);
    SafeRelease(cursorTexture);

    collisionSystem = nullptr;
    modifSystem = nullptr;
    drawSystem = nullptr;
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorSystem::IsCanBeEnabled() const
{
    return drawSystem->VerifyLandscape();
}

bool LandscapeEditorSystem::IsLandscapeEditingEnabled() const
{
    return enabled;
}

void LandscapeEditorSystem::UpdateCursorPosition()
{
    DAVA::Vector3 landPos;
    isIntersectsLandscape = collisionSystem->LandRayTestFromCamera(landPos);
    if (isIntersectsLandscape)
    {
        landPos.x = std::floor(landPos.x);
        landPos.y = std::floor(landPos.y);

        const DAVA::AABBox3& box = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();

        cursorPosition.x = (landPos.x - box.min.x) / (box.max.x - box.min.x);
        cursorPosition.y = (landPos.y - box.min.y) / (box.max.y - box.min.y);
        cursorPosition.x = cursorPosition.x;
        cursorPosition.y = 1.f - cursorPosition.y;

        drawSystem->SetCursorPosition(cursorPosition);
    }
    else
    {
        // hide cursor
        drawSystem->SetCursorPosition(DAVA::Vector2(-100.f, -100.f));
    }
}

void LandscapeEditorSystem::RenderRestoreCallback()
{
    if (rhi::NeedRestoreTexture(cursorTexture->handle))
    {
        DAVA::ScopedPtr<DAVA::Image> image(DAVA::ImageSystem::LoadSingleMip(cursorPathName));
        rhi::UpdateTexture(cursorTexture->handle, image->GetData(), 0);
    }
}
