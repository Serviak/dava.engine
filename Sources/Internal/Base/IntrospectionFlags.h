#ifndef __DAVAENGINE_INTROSPECTION_FLAGS_H__
#define __DAVAENGINE_INTROSPECTION_FLAGS_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

enum eIntrospectionFlags
{
    INTROSPECTION_FLAG_NONE = 0x00,
    INTROSPECTION_FLAG_SERIALIZABLE = 0x01,
    INTROSPECTION_FLAG_EDITOR = 0x02,
    INTROSPECTION_FLAG_EDITOR_READONLY = 0x04,

};
    
};

#endif // __DAVAENGINE_INTROSPECTION_FLAGS_H__
