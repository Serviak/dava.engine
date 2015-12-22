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

#include "VisibilityCheckSystem.h"

#include "Constants.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/VisibilityCheckComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Math/MathHelpers.h"
#include "Utils/Random.h"

#include "Render/Renderer.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/Landscape.h"

VisibilityCheckSystem::VisibilityCheckSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
    renderer.SetDelegate(this);

    for (DAVA::uint32 i = 0; i < CUBEMAPS_COUNT; ++i)
    {
        cubemapTarget[i] = DAVA::Texture::CreateFBO(CUBEMAP_SIZE, CUBEMAP_SIZE, DAVA::PixelFormat::FORMAT_RGBA8888, true, rhi::TEXTURE_TYPE_CUBE);
    }
}

VisibilityCheckSystem::~VisibilityCheckSystem()
{
    for (DAVA::uint32 i = 0; i < CUBEMAPS_COUNT; ++i)
    {
        SafeRelease(cubemapTarget[i]);
    }
}

void VisibilityCheckSystem::ProcessCommand(const Command2*, bool)
{
    shouldPrerender = true;
}

void VisibilityCheckSystem::RegisterEntity(DAVA::Entity* entity)
{
    auto visibilityComponent = entity->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT);
    auto renderComponent = DAVA::GetRenderComponent(entity);
    if ((renderComponent != nullptr) || (visibilityComponent != nullptr))
    {
        AddEntity(entity);
    }
}

void VisibilityCheckSystem::UnregisterEntity(DAVA::Entity* entity)
{
    auto visibilityComponent = entity->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT);
    auto renderComponent = DAVA::GetRenderComponent(entity);
    if ((renderComponent != nullptr) || (visibilityComponent != nullptr))
    {
        RemoveEntity(entity);
    }
}

void VisibilityCheckSystem::AddEntity(DAVA::Entity* entity)
{
    auto requiredComponent = entity->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT);
    if (requiredComponent != nullptr)
    {
        entitiesWithVisibilityComponent.insert({ entity, DAVA::Vector<DAVA::Vector3>() });
        shouldPrerender = true;
        forceRebuildPoints = true;
    }

    auto renderComponent = DAVA::GetRenderComponent(entity);
    if (renderComponent != nullptr)
    {
        auto ro = renderComponent->GetRenderObject();
        if (ro->GetType() == DAVA::RenderObject::TYPE_LANDSCAPE)
        {
            landscape = static_cast<DAVA::Landscape*>(ro);
        }
        renderObjectToEntity[ro] = entity;
    }
}

void VisibilityCheckSystem::RemoveEntity(DAVA::Entity* entity)
{
    auto i = std::find_if(entitiesWithVisibilityComponent.begin(), entitiesWithVisibilityComponent.end(), [entity](const EntityMap::value_type& item) {
        return item.first == entity;
    });

    if (i != entitiesWithVisibilityComponent.end())
    {
        entitiesWithVisibilityComponent.erase(i);
        shouldPrerender = true;
        forceRebuildPoints = true;
    }

    auto renderComponent = DAVA::GetRenderComponent(entity);
    if (renderComponent != nullptr)
    {
        auto ro = renderComponent->GetRenderObject();
        if (ro->GetType() == DAVA::RenderObject::TYPE_LANDSCAPE)
        {
            landscape = nullptr;
        }
        renderObjectToEntity.erase(ro);
    }
}

void VisibilityCheckSystem::Process(DAVA::float32 timeElapsed)
{
    if (!DAVA::Renderer::GetOptions()->IsOptionEnabled(DAVA::RenderOptions::DEBUG_ENABLE_VISIBILITY_SYSTEM))
    {
        return;
    }

    bool shouldRebuildIndices = false;

    for (auto& mapItem : entitiesWithVisibilityComponent)
    {
        auto visibilityComponent = static_cast<DAVA::VisibilityCheckComponent*>(mapItem.first->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT));
        if (!visibilityComponent->IsValid())
        {
            if (visibilityComponent->ShouldRebuildPoints())
            {
                BuildPointSetForEntity(mapItem);
                shouldRebuildIndices = true;
            }
            shouldPrerender = true;
            visibilityComponent->SetValid();
        }
    }

    if (shouldRebuildIndices || forceRebuildPoints)
    {
        BuildIndexSet();
    }
    UpdatePointSet();
}

void VisibilityCheckSystem::Draw()
{
    if (!DAVA::Renderer::GetOptions()->IsOptionEnabled(DAVA::RenderOptions::DEBUG_ENABLE_VISIBILITY_SYSTEM))
    {
        return;
    }

    bool shouldRenderOverlay = false;
    auto rs = GetScene()->GetRenderSystem();
    auto dbg = rs->GetDebugDrawer();
    for (const auto& mapItem : entitiesWithVisibilityComponent)
    {
        auto visibilityComponent = static_cast<DAVA::VisibilityCheckComponent*>(mapItem.first->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT));
        if (visibilityComponent->IsEnabled())
        {
            auto worldTransform = mapItem.first->GetWorldTransform();
            DAVA::Vector3 position = worldTransform.GetTranslationVector();
            DAVA::Vector3 direction = MultiplyVectorMat3x3(DAVA::Vector3(0.0f, 0.0f, 1.0f), worldTransform);
            dbg->DrawCircle(position, direction, visibilityComponent->GetRadius(), 36, DAVA::Color::White, DAVA::RenderHelper::DRAW_WIRE_DEPTH);
            shouldRenderOverlay = true;
        }
    }

    if (!CacheIsValid())
    {
        BuildCache();
        shouldPrerender = true;
    }

    if (shouldPrerender)
    {
        Prerender();
    }

    for (const auto& point : controlPoints)
    {
        dbg->DrawIcosahedron(point.point, 0.1f, DAVA::Color(1.0f, 1.0f, 0.5f, 1.0f), DAVA::RenderHelper::DRAW_WIRE_DEPTH);
        dbg->DrawLine(point.point, point.point + 2.0f * point.normal, DAVA::Color(0.25f, 1.0f, 0.25f, 1.0f));
    }

    auto fromCamera = GetScene()->GetCurrentCamera();

    for (DAVA::uint32 cm = 0; (cm < CUBEMAPS_COUNT) && (currentPointIndex < controlPoints.size()); ++cm, ++currentPointIndex)
    {
        DAVA::uint32 pointIndex = controlPointIndices[currentPointIndex];
        const auto& point = controlPoints[pointIndex];
        renderer.RenderToCubemapFromPoint(rs, fromCamera, point.point, cubemapTarget[cm]);
        renderer.RenderVisibilityToTexture(rs, fromCamera, cubemapTarget[cm], point);
    }

    if (shouldRenderOverlay)
    {
        if ((currentPointIndex == controlPoints.size()) || renderer.FrameFixed())
        {
            renderer.RenderCurrentOverlayTexture(fromCamera);
        }

        if (currentPointIndex < controlPoints.size())
        {
            renderer.RenderProgress(static_cast<float>(currentPointIndex) / static_cast<float>(controlPoints.size()));
        }
    }
}

void VisibilityCheckSystem::InvalidateMaterials()
{
    renderer.InvalidateMaterials();
}

void VisibilityCheckSystem::UpdatePointSet()
{
    controlPoints.clear();

    for (const auto& mapItem : entitiesWithVisibilityComponent)
    {
        auto entity = mapItem.first;
        auto visibilityComponent = static_cast<DAVA::VisibilityCheckComponent*>(entity->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT));
        if (visibilityComponent->IsEnabled())
        {
            auto worldTransform = entity->GetWorldTransform();
            DAVA::Vector3 position = worldTransform.GetTranslationVector();
            DAVA::Vector3 normal = MultiplyVectorMat3x3(DAVA::Vector3(0.0f, 0.0f, 1.0f), worldTransform);
            normal.Normalize();
            float upAngle = std::cos(DAVA::DegToRad(90.0f - visibilityComponent->GetUpAngle()));
            float dnAngle = -std::cos(DAVA::DegToRad(90.0f - visibilityComponent->GetDownAngle()));
            float maxDist = visibilityComponent->GetMaximumDistance();

            bool shouldSnap = (landscape != nullptr) && visibilityComponent->ShouldPlaceOnLandscape();
            float snapHeight = visibilityComponent->GetHeightAboveLandscape();

            for (const auto& pt : mapItem.second)
            {
                DAVA::Vector3 transformedPoint = position + MultiplyVectorMat3x3(pt, worldTransform);
                if (shouldSnap && (landscape->PlacePoint(transformedPoint, transformedPoint)))
                {
                    DAVA::Vector3 dxPoint = transformedPoint + DAVA::Vector3(1.0f, 0.0f, 0.0f);
                    DAVA::Vector3 dyPoint = transformedPoint + DAVA::Vector3(0.0f, 1.0f, 0.0f);
                    landscape->PlacePoint(dxPoint, dxPoint);
                    landscape->PlacePoint(dyPoint, dyPoint);

                    normal = (dxPoint - transformedPoint).CrossProduct(dyPoint - transformedPoint);
                    normal.Normalize();

                    transformedPoint.z += snapHeight;
                }

                controlPoints.emplace_back(transformedPoint, normal, GetNormalizedColorForEntity(mapItem), upAngle, dnAngle, maxDist);
            }
        }
    }
}

void VisibilityCheckSystem::Prerender()
{
    renderer.CreateOrUpdateRenderTarget(stateCache.viewportSize);
    renderer.PreRenderScene(GetScene()->GetRenderSystem(), GetScene()->GetCurrentCamera());

    currentPointIndex = 0;
    shouldPrerender = false;
}

bool VisibilityCheckSystem::CacheIsValid()
{
    DAVA::Size2i vpSize(DAVA::Renderer::GetFramebufferWidth(), DAVA::Renderer::GetFramebufferHeight());
    if (vpSize != stateCache.viewportSize)
        return false;

    auto cam = GetScene()->GetCurrentCamera();
    if (cam != stateCache.camera)
        return false;

    auto currentMatrix = cam->GetViewProjMatrix();
    if (Memcmp(currentMatrix.data, stateCache.viewprojMatrix.data, sizeof(DAVA::Matrix4)) != 0)
        return false;

    return true;
}

void VisibilityCheckSystem::BuildCache()
{
    stateCache.camera = GetScene()->GetCurrentCamera();
    DVASSERT(stateCache.camera != nullptr)

    stateCache.viewportSize = DAVA::Size2i(DAVA::Renderer::GetFramebufferWidth(), DAVA::Renderer::GetFramebufferHeight());
    stateCache.viewprojMatrix = stateCache.camera->GetViewProjMatrix();
}

bool VisibilityCheckSystem::ShouldDrawRenderObject(DAVA::RenderObject* object)
{
    auto type = object->GetType();

    if (type == DAVA::RenderObject::TYPE_LANDSCAPE)
        return true;

    if ((type == DAVA::RenderObject::TYPE_SPEED_TREE) || (type == DAVA::RenderObject::TYPE_SPRITE) ||
        (type == DAVA::RenderObject::TYPE_VEGETATION) || (type == DAVA::RenderObject::TYPE_PARTICLE_EMTITTER))
    {
        return false;
    }

    auto entityIterator = renderObjectToEntity.find(object);
    if (entityIterator == renderObjectToEntity.end())
        return false;

    DAVA::KeyedArchive* customProps = GetCustomPropertiesArchieve(entityIterator->second);
    if (customProps)
    {
        DAVA::String collisionTypeString = "CollisionType";
        if ((object->GetMaxSwitchIndex() > 0) && (object->GetSwitchIndex() > 0))
        {
            collisionTypeString = "CollisionTypeCrashed";
        }

        const DAVA::int32 collisiontype = customProps->GetInt32(collisionTypeString, 0);

        if ((ResourceEditor::ESOT_NO_COLISION == collisiontype) ||
            (ResourceEditor::ESOT_TREE == collisiontype) ||
            (ResourceEditor::ESOT_BUSH == collisiontype) ||
            (ResourceEditor::ESOT_FALLING == collisiontype) ||
            (ResourceEditor::ESOT_FRAGILE_PROJ_INV == collisiontype) ||
            (ResourceEditor::ESOT_SPEED_TREE == collisiontype))
        {
            return false;
        }
    }

    return true;
}

namespace VCSLocal
{
inline bool DistanceFromPointToAnyPointFromSetGreaterThan(const DAVA::Vector3& pt,
                                                          const DAVA::Vector<DAVA::Vector3>& points, DAVA::float32 distanceSquared, DAVA::float32 radiusSquared)
{
    DAVA::float32 distanceFromCenter = pt.x * pt.x + pt.y * pt.y;
    if (distanceFromCenter > radiusSquared)
        return false;

    for (const auto& e : points)
    {
        DAVA::float32 dx = e.x - pt.x;
        DAVA::float32 dy = e.y - pt.y;
        if (dx * dx + dy * dy < distanceSquared)
            return false;
    }

    return true;
}

bool TryToGenerateAroundPoint(const DAVA::Vector3& src, DAVA::float32 distanceBetweenPoints, DAVA::float32 radius, DAVA::Vector<DAVA::Vector3>& points)
{
    const DAVA::uint32 maxAttempts = 36;

    DAVA::float32 distanceSquared = distanceBetweenPoints * distanceBetweenPoints;
    DAVA::float32 radiusSquared = radius * radius;
    DAVA::float32 angle = DAVA::Random::Instance()->RandFloat32InBounds(-DAVA::PI, DAVA::PI);
    DAVA::float32 da = 2.0f * DAVA::PI / static_cast<float>(maxAttempts);
    DAVA::uint32 attempts = 0;
    DAVA::Vector3 newPoint;
    bool canInclude = false;
    do
    {
        newPoint = src + DAVA::Polar(angle, 2.0f * distanceBetweenPoints);
        if (DistanceFromPointToAnyPointFromSetGreaterThan(newPoint, points, distanceSquared, radiusSquared))
        {
            points.push_back(newPoint);
            return true;
        }
        angle += da;
    } while ((++attempts < maxAttempts) && !canInclude);

    return false;
};
}

void VisibilityCheckSystem::BuildPointSetForEntity(EntityMap::value_type& item)
{
    auto component = static_cast<DAVA::VisibilityCheckComponent*>(item.first->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT));

    DAVA::float32 radius = component->GetRadius();
    DAVA::float32 radiusSquared = radius * radius;
    DAVA::float32 distanceBetweenPoints = component->GetDistanceBetweenPoints();
    DAVA::float32 distanceSquared = distanceBetweenPoints * distanceBetweenPoints;

    DAVA::uint32 pointsToGenerate = 2 * static_cast<DAVA::uint32>(radiusSquared / distanceSquared);
    item.second.clear();
    item.second.reserve(pointsToGenerate);

    if (pointsToGenerate < 2)
    {
        item.second.emplace_back(0.0f, 0.0f, 0.0f);
    }
    else
    {
        item.second.push_back(DAVA::Polar(DAVA::Random::Instance()->RandFloat32InBounds(-DAVA::PI, +DAVA::PI), radius - distanceBetweenPoints));

        bool canGenerate = true;
        while (canGenerate)
        {
            canGenerate = false;
            for (DAVA::int32 i = static_cast<DAVA::int32>(item.second.size()) - 1; i >= 0; --i)
            {
                if (VCSLocal::TryToGenerateAroundPoint(item.second.at(i), distanceBetweenPoints, radius, item.second))
                {
                    canGenerate = true;
                    break;
                }
            }
        }
    }

    DAVA::float32 verticalVariance = component->GetVerticalVariance();
    if (verticalVariance > 0.0f)
    {
        for (auto& p : item.second)
        {
            p.z = DAVA::Random::Instance()->RandFloat32InBounds(-verticalVariance, verticalVariance);
        }
    }
}

void VisibilityCheckSystem::BuildIndexSet()
{
    size_t totalPoints = 0;
    for (const auto& item : entitiesWithVisibilityComponent)
    {
        auto visibilityComponent = static_cast<DAVA::VisibilityCheckComponent*>(item.first->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT));
        if (visibilityComponent->IsEnabled())
        {
            totalPoints += item.second.size();
        }
    }
    controlPointIndices.resize(totalPoints);
    for (size_t i = 0; i < totalPoints; ++i)
    {
        controlPointIndices[i] = i;
    }
    std::random_shuffle(controlPointIndices.begin() + 1, controlPointIndices.end());
    forceRebuildPoints = false;
}

DAVA::Color VisibilityCheckSystem::GetNormalizedColorForEntity(const EntityMap::value_type& item) const
{
    auto component = static_cast<DAVA::VisibilityCheckComponent*>(item.first->GetComponent(DAVA::Component::VISIBILITY_CHECK_COMPONENT));
    DAVA::Color normalizedColor = component->GetColor();
    if (component->ShouldNormalizeColor())
    {
        DAVA::float32 fpoints = static_cast<float>(item.second.size());
        normalizedColor.r = (normalizedColor.r > 0.0f) ? std::max(1.0f / 255.0f, normalizedColor.r / fpoints) : 0.0f;
        normalizedColor.g = (normalizedColor.g > 0.0f) ? std::max(1.0f / 255.0f, normalizedColor.g / fpoints) : 0.0f;
        normalizedColor.b = (normalizedColor.b > 0.0f) ? std::max(1.0f / 255.0f, normalizedColor.b / fpoints) : 0.0f;
    }
    return normalizedColor;
}

void VisibilityCheckSystem::FixCurrentFrame()
{
    renderer.FixFrame();
}

void VisibilityCheckSystem::ReleaseFixedFrame()
{
    renderer.ReleaseFrame();
}
