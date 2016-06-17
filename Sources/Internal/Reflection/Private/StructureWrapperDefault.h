#pragma once
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionWrappers.h"

#if !defined(__DAVAENGINE_ANDROID__)

namespace DAVA
{
template <typename T>
class StructureWrapperDefault : public StructureWrapper
{
public:
    bool IsDynamic() const override
    {
        return false;
    }

    bool CanAdd() const override
    {
        return false;
    }
    bool CanInsert() const override
    {
        return false;
    }
    bool CanRemove() const override
    {
        return false;
    }

    Ref::Field GetField(const ReflectedObject& object, const Any& key) const override
    {
        return Ref::Field();
    }
    Ref::FieldsList GetFields(const ReflectedObject& object) const override
    {
        return Ref::FieldsList();
    }

    bool AddField(const ReflectedObject& object, const Any& key, const Any& value) const override
    {
        return false;
    }
    bool InsertField(const ReflectedObject& object, const Any& key, const Any& beforeKey, const Any& value) const override
    {
        return false;
    }
    bool RemoveField(const ReflectedObject& object, const Any& key) const override
    {
        return false;
    }
};

template <typename T>
struct StructureWrapperCreator
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperDefault<T>();
    }
};

} // namespace DAVA

#endif