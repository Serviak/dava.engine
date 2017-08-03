#pragma once

#include "Input/ActionSystem.h"
#include "Input/InputEvent.h"

namespace DAVA
{
namespace Private
{
// Comparators for BoundActionSet's multisets
// Since we need to keep them in order

struct DigitalBindingCompare final
{
    bool operator()(const DigitalBinding& first, const DigitalBinding& second) const;
};

struct AnalogBindingCompare final
{
    bool operator()(const AnalogBinding& first, const AnalogBinding& second) const;
};

// ActionSet + devices it's bound to
// Use multiset since we need to process bindings with bigger number of buttons first
// (CTRL + SPACE should be checked before SPACE, otherwise we might end up with SPACE action being triggered even though CTRL is also pressed)
struct BoundActionSet final
{
    String name;
    Vector<uint32> devices;
    std::multiset<DigitalBinding, DigitalBindingCompare> digitalBindings;
    std::multiset<AnalogBinding, AnalogBindingCompare> analogBindings;
};

class ActionSystemImpl final
{
public:
    ActionSystemImpl(ActionSystem* actionSystem);
    ~ActionSystemImpl();

    void BindSet(const ActionSet& set, Vector<uint32> devices);
    void UnbindAllSets();

private:
    bool OnInputEvent(const InputEvent& event);
    bool CheckDigitalStates(const Array<eInputElements, ActionSystem::MAX_DIGITAL_STATES_COUNT>& digitalElements, const Array<DigitalElementState, ActionSystem::MAX_DIGITAL_STATES_COUNT>& digitalStates, const Vector<uint32>& devices);
    bool CompareDigitalStates(const DigitalElementState& requiredState, const DigitalElementState& state);

private:
    ActionSystem* actionSystem;
    Vector<BoundActionSet> boundSets;
    uint32 inputHandlerToken = 0;
};
}
}
