#include "Input/ActionSystem.h"

#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "DeviceManager/DeviceManager.h"

namespace DAVA
{

static uint32 bindingId = 1;

ActionSystem::ActionSystem()
{
    GetEngineContext()->inputSystem->AddInputEventHandler(MakeFunction(this, &ActionSystem::OnInputEvent));
}

ActionSystem::~ActionSystem()
{
    GetEngineContext()->inputSystem->RemoveHandler(inputSystemHandlerToken);
}

uint32 ActionSystem::BindDigitalAction(uint32 actionId, DeviceDigitalControlState state1)
{

}

uint32 ActionSystem::BindDigitalAction(uint32 actionId, DeviceDigitalControlState state1, DeviceDigitalControlState state2)
{

}

uint32 ActionSystem::BindDigitalAction(uint32 actionId, DeviceDigitalControlState state1, DeviceDigitalControlState state2, DeviceDigitalControlState state3)
{

}

bool ActionSystem::OnInputEvent(const InputEvent& event)
{
    for (const DigitalActionBinding& binding : digitalBindings)
    {
        bool triggered = true;

        for (const DeviceDigitalControlState& requiredState : binding.requiredStates)
        {
            const InputDevice* device = GetEngineContext()->deviceManager->GetInputDevice(requiredState.deviceId);
            DigitalControlState state = device->GetDigitalControlState(requiredState.controlId);

            if (state != requiredState.state)
            {
                triggered = false;
                break;
            }
        }

        if (triggered)
        {
            Action action;
            action.actionId = binding.actionId;
            action.triggeredDeviceType = event.deviceType;
            action.triggeredDeviceId = event.deviceId;

            ActionTriggered.Emit(action);
        }
    }

    return false;
}
}