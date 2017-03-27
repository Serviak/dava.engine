#include "Input/ActionSystem.h"
#include "Input/Private/ActionSystemImpl.h"

namespace DAVA
{
ActionSystem::ActionSystem() : impl(new Private::ActionSystemImpl(this))
{

}

ActionSystem::~ActionSystem()
{
    if (impl != nullptr)
    {
        delete impl;
        impl = nullptr;
    }
}

void ActionSystem::BindSet(const ActionSet& set)
{
    impl->BindSet(set, Array<uint32, MAX_DEVICES_COUNT>{});
}

void ActionSystem::BindSet(const ActionSet& set, uint32 deviceId)
{
    Array<uint32, MAX_DEVICES_COUNT> devices{};
    devices[0] = deviceId;
    impl->BindSet(set, devices);
}

void ActionSystem::BindSet(const ActionSet& set, uint32 deviceId1, uint32 deviceId2)
{
    Array<uint32, MAX_DEVICES_COUNT> devices;
    devices[0] = deviceId1;
    devices[1] = deviceId2;
    impl->BindSet(set, devices);
}
}