#pragma once

#include "Base/StaticSingleton.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
namespace TArc
{
class ModuleCollection : public StaticSingleton<ModuleCollection>
{
public:
    using TypeCreateFn = DAVA::Function<const ReflectedType*()>;
    void AddGuiModule(const TypeCreateFn& type);
    void AddConsoleModule(const TypeCreateFn& type, const String& command);

    Vector<const ReflectedType*> GetGuiModules() const;
    Vector<std::pair<const ReflectedType*, String>> GetConsoleModules() const;

private:
    Vector<TypeCreateFn> guiModules;
    Vector<std::pair<TypeCreateFn, String>> consoleModules;
};

template <typename T>
struct ModuleInitializer
{
    ModuleInitializer()
    {
        ModuleCollection::Instance()->AddGuiModule(ModuleCollection::TypeCreateFn(&ModuleInitializer<typename T>::GetType));
    }

    ModuleInitializer(const String& command)
    {
        ModuleCollection::Instance()->AddConsoleModule(ModuleCollection::TypeCreateFn(&ModuleInitializer<typename T>::GetType), command);
    }

    static const ReflectedType* GetType()
    {
        return ReflectedTypeDB::Get<T>();
    }
};
} // namespace TArc
} // namespace DAVA


#define DECL_GUI_MODULE(name) ::DAVA::TArc::ModuleInitializer<name> initializer_##name
#define DECL_CONSOLE_MODULE(name, command) ::DAVA::TArc::ModuleInitializer<name> initializer_##name(command)
