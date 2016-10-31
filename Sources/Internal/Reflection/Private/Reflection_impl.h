#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

#include "Reflection/Wrappers.h"
#include "Reflection/ReflectedType.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
inline Reflection::Reflection(const ReflectedObject& object_, const PropertieWrapper* vw_, const ReflectedType* rtype_, const ReflectedMeta* meta_)
    : object(object_)
    , vw(vw_)
    , meta(meta_)
    , objectType(rtype_)
{
    if (nullptr != rtype_)
    {
        sw = rtype_->structureWrapper.get();
    }

    if (nullptr != meta)
    {
        if (meta->HasMeta<StructureWrapper>())
        {
            sw = meta->GetMeta<StructureWrapper>();
        }
    }
}

inline bool Reflection::IsValid() const
{
    return (nullptr != vw && object.IsValid());
}

inline bool Reflection::IsReadonly() const
{
    return vw->IsReadonly();
}

inline const RttiType* Reflection::GetValueType() const
{
    return vw->GetType();
}

inline ReflectedObject Reflection::GetValueObject() const
{
    return vw->GetPropertieObject(object);
}

inline const DAVA::ReflectedType* Reflection::GetReflectedType() const
{
    return objectType;
}

inline Any Reflection::GetValue() const
{
    return vw->GetValue(object);
}

inline bool Reflection::SetValue(const Any& value) const
{
    return vw->SetValue(object, value);
}

inline bool Reflection::HasFields() const
{
    return sw->HasFields(object, vw);
}

inline Reflection Reflection::GetField(const Any& key) const
{
    return sw->GetField(object, vw, key);
}

inline Vector<Reflection::Field> Reflection::GetFields() const
{
    return sw->GetFields(object, vw);
}

inline bool Reflection::CanAddFields() const
{
    return sw->CanAdd(object, vw);
}

inline bool Reflection::CanInsertFields() const
{
    return sw->CanInsert(object, vw);
}

inline bool Reflection::CanRemoveFields() const
{
    return sw->CanRemove(object, vw);
}

inline bool Reflection::CanCreateFieldValue() const
{
    return sw->CanCreateValue(object, vw);
}

inline Any Reflection::CreateFieldValue() const
{
    return sw->CreateValue(object, vw);
}

inline bool Reflection::AddField(const Any& key, const Any& value) const
{
    return sw->AddField(object, vw, key, value);
}

inline bool Reflection::InsertField(const Any& beforeKey, const Any& key, const Any& value) const
{
    return sw->InsertField(object, vw, beforeKey, key, value);
}

inline bool Reflection::RemoveField(const Any& key) const
{
    return sw->RemoveField(object, vw, key);
}

inline bool Reflection::HasMethods() const
{
    return sw->HasMethods(object, vw);
}

inline AnyFn Reflection::GetMethod(const String& key) const
{
    return sw->GetMethod(object, vw, key);
}

inline Vector<Reflection::Method> Reflection::GetMethods() const
{
    return sw->GetMethods(object, vw);
}

template <typename Meta>
inline bool Reflection::HasMeta() const
{
    return (nullptr != meta) ? meta->HasMeta<Meta>() : false;
}

template <typename Meta>
inline const Meta* Reflection::GetMeta() const
{
    return (nullptr != meta) ? meta->GetMeta<Meta>() : nullptr;
}

template <typename T>
Reflection Reflection::Create(T* ptr, const ReflectedMeta* meta)
{
    static ValueWrapperDefault<T> vw;

    if (nullptr != ptr)
    {
        const ReflectedType* rtype = ReflectedType::GetByPointer(ptr);
        return Reflection(ReflectedObject(ptr), &vw, rtype, meta);
    }

    return Reflection();
}

template <typename T>
Reflection::Field Reflection::Field::Create(const Any& key, T* ptr, const ReflectedMeta* meta)
{
    return Reflection::Field{ key, Reflection::Create(ptr, meta) };
}

} // namespace DAVA
