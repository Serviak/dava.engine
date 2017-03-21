#include "Input/ActionSystem.h"

#include "Engine/Engine.h"
#include "Input/InputSystem.h"
#include "DeviceManager/DeviceManager.h"

namespace DAVA
{
ActionSystem::ActionSystem()
{
    GetEngineContext()->inputSystem->AddInputEventHandler(MakeFunction(this, &ActionSystem::OnInputEvent));
}

ActionSystem::~ActionSystem()
{
    // GetEngineContext()->inputSystem->RemoveHandler(inputSystemHandlerToken);
}

uint32 ActionSystem::BindDigitalAction(uint32 actionId, DeviceDigitalControlState state1)
{
    DigitalActionBinding binding;
    binding.actionId = actionId;
    binding.states[0] = state1;

    digitalBindings.push_back(binding);

    return 0;
}

uint32 ActionSystem::BindDigitalAction(uint32 actionId, DeviceDigitalControlState state1, DeviceDigitalControlState state2)
{
    DigitalActionBinding binding;
    binding.actionId = actionId;
    binding.states[0] = state1;
    binding.states[1] = state2;

    digitalBindings.push_back(binding);

    return 0;
}

uint32 ActionSystem::BindDigitalAction(uint32 actionId, DeviceDigitalControlState state1, DeviceDigitalControlState state2, DeviceDigitalControlState state3)
{
    DigitalActionBinding binding;
    binding.actionId = actionId;
    binding.states[0] = state1;
    binding.states[1] = state2;
    binding.states[2] = state3;

    digitalBindings.push_back(binding);

    return 0;
}

bool ActionSystem::OnInputEvent(const InputEvent& event)
{
    for (const DigitalActionBinding& binding : digitalBindings)
    {
        bool triggered = true;

        for (const DeviceDigitalControlState& requiredState : binding.states)
        {
            if (requiredState.deviceId == -1)
            {
                break;
            }

            const InputDevice* device = GetEngineContext()->deviceManager->GetInputDevice(requiredState.deviceId);
            eDigitalControlState state = device->GetDigitalControlState(requiredState.controlId);

            if ((state & requiredState.stateMask) != requiredState.stateMask)
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