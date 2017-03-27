#pragma once

#include "Input/ActionSystem.h"
#include "Input/InputEvent.h"

namespace DAVA
{
namespace Private
{
#define MAX_DEVICES_COUNT 2

// Comparators for BindedActionSet's multisets
// Since we need to keep them in order

struct DigitalBindingCompare final
{
    bool operator()(const DigitalBinding& first, const DigitalBinding& second) const;
};

struct AnalogBindingCompare final
{
    bool operator()(const AnalogBinding& first, const AnalogBinding& second) const;
};

// ActionSet + devices it's binded to
// Use multiset since we need to process bindings with bigger number of buttons first
// (CTRL + SPACE should be checked before SPACE, otherwise we might end up with SPACE action being triggered even though CTRL is also pressed)
struct BindedActionSet final
{
    Array<uint32, MAX_DEVICES_COUNT> devices;
    std::multiset<DigitalBinding, DigitalBindingCompare> digitalBindings;
    std::multiset<AnalogBinding, AnalogBindingCompare> analogBindings;
};

class ActionSystemImpl final
{
public:
    ActionSystemImpl(ActionSystem* actionSystem);
    ~ActionSystemImpl();

    void BindSet(const ActionSet& set, Array<uint32, MAX_DEVICES_COUNT> devices);

private:
   bool OnInputEvent(const InputEvent& event);
   bool CheckDigitalStates(const Array<DigitalControlState, MAX_DIGITAL_STATES_COUNT>& states, const Array<uint32, MAX_DEVICES_COUNT>& devices);

private:
    ActionSystem* actionSystem;
    Vector<BindedActionSet> bindedSets;
};
}
}