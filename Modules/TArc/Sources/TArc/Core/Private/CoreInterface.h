#pragma once

#include "TArc/Core/ContextManager.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/Core/OperationInvoker.h"

namespace DAVA
{
class AnyFn;
namespace TArc
{
class ClientModule;
class CoreInterface : public ContextAccessor, public ContextManager, public OperationInvoker
{
public:
    virtual void RegisterOperation(int operationID, AnyFn&& fn) = 0;
    virtual void RegisterInterface(ClientModule* module, const Type* lookupType, Any interface) = 0;
    virtual Any QueryInterface(const Type* lookupType) const = 0;
};
} // namespace TArc
} // namespace DAVA
