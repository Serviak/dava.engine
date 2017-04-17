#include "Classes/SlotSupportModule/Private/EditorSlotSystem.h"
#include "Classes/Commands2/SlotCommands.h"
#include "Classes/Commands2/Base/RECommandBatch.h"
#include "Classes/Commands2/SetFieldValueCommand.h"
#include "Classes/Commands2/RemoveComponentCommand.h"
#include "Classes/Commands2/Base/RECommandNotificationObject.h"
#include "Classes/Qt/Scene/SceneSignals.h"

#include <Scene3D/Systems/SlotSystem.h>
#include <Scene3D/Components/SlotComponent.h>
#include <Scene3D/Scene.h>
#include <Base/BaseTypes.h>

#include <Utils/Utils.h>
#include <QObject>

EditorSlotSystem::EditorSlotSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
}

void EditorSlotSystem::ImmediateEvent(DAVA::Component* component, DAVA::uint32 event)
{
    if (event == DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED)
    {
        DAVA::Entity* entity = component->GetEntity();
        DAVA::SlotComponent* slotComponent = GetScene()->slotSystem->LookUpSlot(entity);
        if (slotComponent != nullptr)
        {
            slotComponent->SetAttachmentTransform(entity->GetLocalTransform());
        }
    }
}

void EditorSlotSystem::RegisterEntity(DAVA::Entity* entity)
{
    if (entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) > 0)
    {
        entities.push_back(entity);
        pendingOnInitialize.insert(entity);
    }
}

void EditorSlotSystem::UnregisterEntity(DAVA::Entity* entity)
{
    if (entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) > 0)
    {
        FindAndRemoveExchangingWithLast(entities, entity);
        pendingOnInitialize.erase(entity);
    }
}

void EditorSlotSystem::RegisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    if (component->GetType() == DAVA::Component::SLOT_COMPONENT)
    {
        pendingOnInitialize.insert(entity);
        if (entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) == 1)
        {
            entities.push_back(entity);
        }
    }
}

void EditorSlotSystem::UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    if (component->GetType() == DAVA::Component::SLOT_COMPONENT)
    {
        if (entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT) == 1)
        {
            FindAndRemoveExchangingWithLast(entities, entity);
            pendingOnInitialize.erase(entity);
        }
    }
}

void EditorSlotSystem::Process(DAVA::float32 timeElapsed)
{
    using namespace DAVA;

    for (Entity* entity : pendingOnInitialize)
    {
        Set<FastName> names;
        Set<SlotComponent*> uninitializedSlots;
        for (uint32 i = 0; i < entity->GetComponentCount(Component::SLOT_COMPONENT); ++i)
        {
            SlotComponent* slotComponent = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, i));
            FastName slotName = slotComponent->GetSlotName();
            if (slotName.IsValid())
            {
                names.insert(slotName);
            }
            else
            {
                uninitializedSlots.insert(slotComponent);
            }
        }

        uint32 slotIndex = 1;
        for (SlotComponent* component : uninitializedSlots)
        {
            FastName newSlotName(Format("Slot_%u", slotIndex++));
            while (names.count(newSlotName) > 0)
            {
                newSlotName = FastName(Format("Slot_%u", slotIndex++));
            }

            component->SetSlotName(newSlotName);
        }
    }

    SlotSystem* slotSystem = GetScene()->slotSystem;
    for (Entity* entity : pendingOnInitialize)
    {
        uint32 slotCount = entity->GetComponentCount(Component::SLOT_COMPONENT);
        for (uint32 slotIndex = 0; slotIndex < slotCount; ++slotIndex)
        {
            SlotComponent* component = static_cast<SlotComponent*>(entity->GetComponent(Component::SLOT_COMPONENT, slotIndex));
            Entity* loadedEntity = slotSystem->LookUpLoadedEntity(component);
            if (loadedEntity == nullptr)
            {
                DAVA::RefPtr<Entity> newEntity(new Entity());
                slotSystem->AttachEntityToSlot(component, newEntity.Get());
            }
        }
    }

    pendingOnInitialize.clear();
}

void EditorSlotSystem::DetachEntity(DAVA::SlotComponent* component, DAVA::Entity* entity)
{
    DAVA::Entity* slotEntity = component->GetEntity();
    DAVA::Entity* loadedEntity = GetScene()->slotSystem->LookUpLoadedEntity(component);
    DVASSERT(loadedEntity == entity);
    DVASSERT(slotEntity == entity->GetParent());

    slotEntity->RemoveNode(entity);
}

void EditorSlotSystem::AttachEntity(DAVA::SlotComponent* component, DAVA::Entity* entity)
{
    DAVA::SlotSystem* slotSystem = GetScene()->slotSystem;
    slotSystem->AttachEntityToSlot(component, entity);
}

DAVA::Entity* EditorSlotSystem::AttachEntity(DAVA::SlotComponent* component, const DAVA::FastName& itemName)
{
    DAVA::SlotSystem* slotSystem = GetScene()->slotSystem;
    return slotSystem->AttachItemToSlot(component, itemName);
}

void EditorSlotSystem::AccumulateDependentCommands(REDependentCommandsHolder& holder)
{
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
    auto changeSlotvisitor = [&](const RECommand* command)
    {
        const SetFieldValueCommand* cmd = static_cast<const SetFieldValueCommand*>(command);
        DAVA::ReflectedObject object = cmd->GetTarget().GetParentValueObject();
        DAVA::FastName fieldName = cmd->GetFieldName().Cast<DAVA::FastName>(DAVA::FastName(""));
        const DAVA::ReflectedType* type = object.GetReflectedType();
        if (type == DAVA::ReflectedTypeDB::Get<DAVA::SlotComponent>())
        {
            DAVA::SlotComponent* component = object.GetPtr<DAVA::SlotComponent>();
            DAVA::Entity* entity = scene->slotSystem->LookUpLoadedEntity(component);
            if (fieldName == DAVA::SlotComponent::ConfigPathFieldName)
            {
                bool tankFound = false;
                for (DAVA::uint32 i = 0; i < component->GetFiltersCount(); ++i)
                {
                    if (component->GetFilter(i) == DAVA::FastName("Tank"))
                    {
                        tankFound = true;
                        break;
                    }
                }

                if (tankFound == false)
                {
                    component->AddFilter(DAVA::FastName("Tank"));
                }

                if (DAVA::GetRenderComponent(entity) == nullptr)
                {
                    holder.AddPostCommand(std::make_unique<AttachEntityToSlot>(scene, component, DAVA::FastName("Tank")));
                }
                else
                {
                    DAVA::RefPtr<DAVA::Entity> newEntity(new DAVA::Entity());
                    holder.AddPreCommand(std::make_unique<AttachEntityToSlot>(scene, component, newEntity.Get()));
                }
            }
        }
    };

    const RECommandNotificationObject& commandInfo = holder.GetMasterCommandInfo();
    commandInfo.ForEach(changeSlotvisitor, CMDID_REFLECTED_FIELD_MODIFY);

    auto removeSlotVisitor = [&](const RECommand* command)
    {
        const RemoveComponentCommand* cmd = static_cast<const RemoveComponentCommand*>(command);
        DAVA::Component* component = const_cast<DAVA::Component*>(cmd->GetComponent());
        if (component->GetType() == DAVA::Component::SLOT_COMPONENT)
        {
            DAVA::SlotComponent* slotComponent = static_cast<DAVA::SlotComponent*>(component);
            holder.AddPreCommand(std::make_unique<AttachEntityToSlot>(scene, slotComponent, nullptr));
        }
    };

    commandInfo.ForEach(removeSlotVisitor, CMDID_COMPONENT_REMOVE);
}

void EditorSlotSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
    auto visitor = [&](const RECommand* command)
    {
        const SetFieldValueCommand* cmd = static_cast<const SetFieldValueCommand*>(command);
        DAVA::ReflectedObject object = cmd->GetTarget().GetParentValueObject();
        DAVA::FastName fieldName = cmd->GetFieldName().Cast<DAVA::FastName>(DAVA::FastName(""));
        const DAVA::ReflectedType* type = object.GetReflectedType();
        if (type == DAVA::ReflectedTypeDB::Get<DAVA::SlotComponent>())
        {
            DAVA::SlotComponent* component = object.GetPtr<DAVA::SlotComponent>();
            if (fieldName == DAVA::SlotComponent::SlotNameFieldName)
            {
                DAVA::Entity* entity = scene->slotSystem->LookUpLoadedEntity(component);
                DVASSERT(entity != nullptr);
                entity->SetName(component->GetSlotName());
            }
        }

        if (type == DAVA::ReflectedTypeDB::Get<DAVA::Entity>() && fieldName == DAVA::Entity::EntityNameFieldName)
        {
            DAVA::Entity* entity = object.GetPtr<DAVA::Entity>();
            DAVA::SlotComponent* component = scene->slotSystem->LookUpSlot(entity);
            DVASSERT(component != nullptr);
            component->SetSlotName(entity->GetName());
        }
    };

    commandNotification.ForEach(visitor, CMDID_REFLECTED_FIELD_MODIFY);
}

void EditorSlotSystem::SetScene(DAVA::Scene* scene)
{
    {
        Scene* currentScene = GetScene();
        if (currentScene != nullptr)
        {
            currentScene->eventSystem->UnregisterSystemForEvent(this, DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED);
        }
    }

    SceneSystem::SetScene(scene);

    {
        Scene* currentScene = GetScene();
        if (currentScene != nullptr)
        {
            currentScene->eventSystem->RegisterSystemForEvent(this, DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED);
        }
    }
}

std::unique_ptr<DAVA::Command> EditorSlotSystem::PrepareForSave(bool /*saveForGame*/)
{
    if (entities.empty())
    {
        return nullptr;
    }

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    DAVA::SlotSystem* slotSystem = sceneEditor->slotSystem;

    RECommandBatch* batchCommand = new RECommandBatch("Prepare for save", static_cast<DAVA::uint32>(entities.size()));
    for (DAVA::Entity* entity : entities)
    {
        for (DAVA::uint32 i = 0; i < entity->GetComponentCount(DAVA::Component::SLOT_COMPONENT); ++i)
        {
            DAVA::SlotComponent* component = static_cast<DAVA::SlotComponent*>(entity->GetComponent(DAVA::Component::SLOT_COMPONENT, i));
            batchCommand->Add(std::make_unique<AttachEntityToSlot>(sceneEditor, component, nullptr));
        }
    }

    return std::unique_ptr<DAVA::Command>(batchCommand);
}
