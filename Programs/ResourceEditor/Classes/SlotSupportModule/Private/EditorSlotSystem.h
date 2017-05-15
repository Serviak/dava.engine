#pragma once

#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

#include <TArc/Utils/QtConnections.h>

#include <Entity/SceneSystem.h>
#include <Scene3D/Components/SlotComponent.h>
#include <Base/BaseTypes.h>

class EditorSlotSystem : public DAVA::SceneSystem,
                         public EditorSceneSystem
{
public:
    static const DAVA::FastName emptyItemName;

    EditorSlotSystem(DAVA::Scene* scene);

    void ImmediateEvent(DAVA::Component* component, DAVA::uint32 event) override;

    void RegisterEntity(DAVA::Entity* entity) override;
    void UnregisterEntity(DAVA::Entity* entity) override;

    void RegisterComponent(DAVA::Entity* entity, DAVA::Component* component) override;
    void UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component) override;

    void Process(DAVA::float32 timeElapsed) override;

protected:
    friend class AttachEntityToSlot;

    void SetScene(DAVA::Scene* scene) override;

    void DetachEntity(DAVA::SlotComponent* component, DAVA::Entity* entity);
    void AttachEntity(DAVA::SlotComponent* component, DAVA::Entity* entity, DAVA::FastName itemName);
    DAVA::Entity* AttachEntity(DAVA::SlotComponent* component, DAVA::FastName itemName);

    void AccumulateDependentCommands(REDependentCommandsHolder& holder) override;
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

protected:
    std::unique_ptr<DAVA::Command> PrepareForSave(bool saveForGame) override;

private:
    DAVA::Vector<DAVA::Entity*> entities;
    DAVA::Set<DAVA::Entity*> pendingOnInitialize;
    DAVA::TArc::QtConnections connections;
};
