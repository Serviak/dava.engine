#pragma once

#include "Base/RefPtr.h"

#include "Engine/EngineTypes.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{
namespace TArc
{
class Core;
class BaseApplication
{
public:
    BaseApplication() = default;

    int Run();

protected:
    struct EngineInitInfo
    {
        RefPtr<KeyedArchive> options;
        eEngineRunMode runMode;
        Vector<String> modules;
    };

    virtual EngineInitInfo GetInitInfo() const = 0;
    virtual void CreateModules(Core* tarcCore) const = 0;
    virtual void Cleanup()
    {
    }
};

} // namespace TArc
} // namespace DAVA
