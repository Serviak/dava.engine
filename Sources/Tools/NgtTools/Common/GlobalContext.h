#ifndef __QTTOOLS_GLOBALCONTEXT_H__
#define __QTTOOLS_GLOBALCONTEXT_H__

#include "Debug/DVAssert.h"
#include "core_generic_plugin/interfaces/i_component_context.hpp"

namespace NGTLayer
{
void SetGlobalContext(wgt::IComponentContext* context);
wgt::IComponentContext* GetGlobalContext();

template <class T>
T* queryInterface()
{
    wgt::IComponentContext* context = GetGlobalContext();
    DVASSERT(context != nullptr);
    return context->queryInterface<T>();
}
} // namespace NGTLayer

#endif // __QTTOOLS_GLOBALCONTEXT_H__