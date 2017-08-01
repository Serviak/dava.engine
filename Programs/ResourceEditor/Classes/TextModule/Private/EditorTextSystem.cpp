#include "Classes/TextModule/Private/EditorTextSystem.h"

#include "Classes/Qt/Scene/System/TextDrawSystem.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

#include <Debug/DVAssert.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TextComponent.h>

EditorTextSystem::EditorTextSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
}

void EditorTextSystem::AddEntity(DAVA::Entity* entity)
{
    DVASSERT(entity->GetComponentCount(DAVA::Component::TEXT_COMPONENT) > 0);
    textEntities.push_back(entity);
}

void EditorTextSystem::RemoveEntity(DAVA::Entity* entity)
{
    bool removed = DAVA::FindAndRemoveExchangingWithLast(textEntities, entity);
    DVASSERT(removed);
}

void EditorTextSystem::Draw()
{
    using namespace DAVA;

    if (systemIsEnabled == false)
        return;

    SceneEditor2* sc = static_cast<SceneEditor2*>(GetScene());
    TextDrawSystem* drawSystem = sc->textDrawSystem;

    for (Entity* entity : textEntities)
    {
        uint32 count = entity->GetComponentCount(Component::TEXT_COMPONENT);

        float32 textHeight = 0.f;
        for (uint32 i = 0; i < count; ++i)
        {
            TextComponent* tc = static_cast<TextComponent*>(entity->GetComponent(Component::TEXT_COMPONENT, i));
            if (tc->IsVisible() && tc->GetText().empty() == false)
            {
                textHeight += tc->GetSize();
            }
        }

        Vector3 position = entity->GetWorldTransform().GetTranslationVector();
        Vector2 position2D = drawSystem->ToPos2d(position);
        position2D.y += textHeight / 2.f;

        float32 offset = 0.f;
        for (uint32 i = 0; i < count; ++i)
        {
            TextComponent* tc = static_cast<TextComponent*>(entity->GetComponent(Component::TEXT_COMPONENT, i));
            if (tc->IsVisible() && tc->GetText().empty() == false)
            {
                offset += tc->GetSize();
                drawSystem->DrawText(position2D - Vector2(0.f, offset), tc->GetText(), tc->GetColor(), tc->GetSize(), TextDrawSystem::Align::TopCenter);
            }
        }
    }
}
