#include "LuaBridge.h"
#include "Scripting/LuaException.h"
#include "Reflection/Reflection.h"
#include "Logger/Logger.h"
#include "Utils/UTF8Utils.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
namespace LuaBridge
{
/******************************************************************************/

/**
 * \brief Lua global table name for Dava service functions
 */
static const char* DavaNamespace = "DV";

/**
 * \brief Check and pop string variable from stack and print it to log.
 *        Lua stack changes [-1, +0, v]
 */
int32 DV_Debug(lua_State* L)
{
    luaL_checktype(L, -1, LUA_TSTRING);
    const char* msg = lua_tostring(L, -1);
    lua_pop(L, 1);
    Logger::Debug(msg);
    return 0;
}

/**
 * \brief Check and pop string variable from stack and print it to log.
 *        Lua stack changes [-1, +0, v]
 */
int32 DV_Error(lua_State* L)
{
    luaL_checktype(L, -1, LUA_TSTRING);
    const char* msg = lua_tostring(L, -1);
    Logger::Error(msg);
    lua_pop(L, 1);
    return 0;
}

void RegisterDava(lua_State* L)
{
    static const struct luaL_reg davalib[] = {
        { "Debug", &DV_Debug },
        { "Error", &DV_Error },
        { nullptr, nullptr }
    };

    luaL_register(L, DavaNamespace, davalib);
    lua_pop(L, 1);
}

/******************************************************************************/

/**
 * \brief Any metatable type name
 */
static const char* AnyTName = "AnyT";

/**
 * \brief Get userdata from stack with index and return it as Any. 
 *        Return NULL if can't get.
 *        Lua stack changes [-0, +0, -]
 */
Any lua_todvany(lua_State* L, int32 index)
{
    Any* pAny = static_cast<Any*>(lua_touserdata(L, index));
    return *pAny;
}

/**
 * \brief Check and get userdata from stack with index and return it as Any.
 *        Throw lua_error on incorrect Lua type.
 *        Lua stack changes [-0, +0, v]
 */
Any lua_checkdvany(lua_State* L, int32 index)
{
    luaL_checktype(L, index, LUA_TUSERDATA);
    Any* pAny = static_cast<Any*>(luaL_checkudata(L, index, AnyTName));
    if (!pAny)
    {
        luaL_typerror(L, index, AnyTName);
    }
    return *pAny;
}

/**
 * \brief Push new userdata with Any metatable to top on the stack.
 *        Lua stack changes [-0, +1, -]
 */
void lua_pushdvany(lua_State* L, const Any& refl)
{
    Any* pAny = static_cast<Any*>(lua_newuserdata(L, sizeof(Any)));
    *pAny = refl;
    luaL_getmetatable(L, AnyTName);
    lua_setmetatable(L, -2);
}

/**
 * \brief Metamethod for presentation Any as string.
 *        Lua stack changes [-0, +1, -]
 */
int32 Any__tostring(lua_State* L)
{
    Any any = lua_checkdvany(L, 1);
    void* pAny = lua_touserdata(L, 1);
    lua_pushfstring(L, "Any: %s (%p)", any.GetType()->GetName(), pAny);
    return 1;
}

void RegisterAny(lua_State* L)
{
    static const luaL_reg Any_meta[] = {
        { "__tostring", &Any__tostring }, // Control string representation
        // { "__call",  }, // Treat a table like a function
        // { "__len",  }, // Control table length
        // { "__mode",  }, // Control weak references
        // { "__unm",  }, // Unary minus
        // { "__add",  }, // Addition
        // { "__sub",  }, // Subtraction
        // { "__mul",  }, // Multiplication
        // { "__div",  }, // Division
        // { "__mod",  }, // Modulo
        // { "__pow",  }, // Involution
        // { "__concat",  }, // Concatenation
        // { "__eq",  }, // Check for equality
        // { "__lt",  }, // Check for less-than
        // { "__le",  }, // Check for less-than-or-equal
        { nullptr, nullptr }
    };

    luaL_newmetatable(L, AnyTName);
    luaL_register(L, 0, Any_meta);
    lua_pop(L, 1);
}

/******************************************************************************/

/**
 * \brief Reflection metatable type name
 */
static const char* ReflectionTName = "ReflectionT";

/**
 * \brief Get userdata from stack with index and return it as Reflection. 
 *        Return NULL if can't get.
 *        Lua stack changes [-0, +0, -]
 */
Reflection lua_todvreflection(lua_State* L, int32 index)
{
    Reflection* pRef = static_cast<Reflection*>(lua_touserdata(L, index));
    return *pRef;
}

/**
 * \brief Check and get userdata from stack with index and return it as Reflection.
 *        Throw lua_error on incorrect Lua type.
 *        Lua stack changes [-0, +0, v]
 */
Reflection lua_checkdvreflection(lua_State* L, int32 index)
{
    luaL_checktype(L, index, LUA_TUSERDATA);
    Reflection* pRef = static_cast<Reflection*>(luaL_checkudata(L, index, ReflectionTName));
    if (!pRef)
    {
        luaL_typerror(L, index, ReflectionTName);
    }
    return *pRef;
}

/**
 * \brief Push new userdata with Reflection metatable to top on the stack.
 *        Lua stack changes [-0, +1, -]
 */
void lua_pushdvreflection(lua_State* L, const Reflection& refl)
{
    Reflection* pRef = static_cast<Reflection*>(lua_newuserdata(L, sizeof(Reflection)));
    *pRef = refl;
    luaL_getmetatable(L, ReflectionTName);
    lua_setmetatable(L, -2);
}

/**
 * \brief Metamethod for presentation Any as string.
 *        Lua stack changes [-0, +1, -]
 */
int32 Reflection__tostring(lua_State* L)
{
    Reflection refl = lua_checkdvreflection(L, 1);
    void* pRef = lua_touserdata(L, 1);
    lua_pushfstring(L, "Reflection: %s (%p)", refl.GetValueType()->GetName(), pRef);
    return 1;
}

/**
 * \brief Metamethod for getting element from Reflection userdata object.
 *        Lua stack changes [-0, +1, v]
 */
int32 Reflection__index(lua_State* L)
{
    Reflection self = lua_checkdvreflection(L, 1);

    Any name;
    int ltype = lua_type(L, 2);
    switch (ltype)
    {
    case LUA_TNUMBER:
        name.Set(size_t(lua_tointeger(L, 2)) - 1); // -1 because in Lua first item in array has index 1
        break;
    case LUA_TSTRING:
        name.Set(String(lua_tostring(L, 2)));
        break;
    default:
        return luaL_error(L, "Wrong key type \"%s\"!", lua_typename(L, ltype));
    }

    Reflection refl = self.GetField(name).ref;
    if (!refl.IsValid())
    {
        lua_pushnil(L);
        return 1;
    }

    if (refl.HasFields() || refl.HasMethods())
    {
        lua_pushdvreflection(L, refl);
        return 1;
    }

    anyToLua(L, refl.GetValue());
    return 1;
}

/**
 * \brief Metamethod for setting value to Reflection userdata object.
 *        Lua stack changes [-0, +0, v]
 */
int32 Reflection__newindex(lua_State* L)
{
    Reflection self = lua_checkdvreflection(L, 1);

    Any name;
    int ltype = lua_type(L, 2);
    switch (ltype)
    {
    case LUA_TNUMBER:
        name.Set(size_t(lua_tointeger(L, 2)) - 1); // -1 because in Lua first item in array has index 1
        break;
    case LUA_TSTRING:
        name.Set(String(lua_tostring(L, 2)));
        break;
    default:
        return luaL_error(L, "Wrong key type %d!", ltype);
    }

    Reflection refl = self.GetField(name).ref;
    if (refl.IsValid())
    {
        try
        {
            Any value = luaToAny(L, 3);

            // Cast-HACK
            if (value.GetType() == Type::Instance<float64>())
            {
                float64 rawValue = value.Get<float64>();
                if (refl.GetValueType() == Type::Instance<int32>())
                {
                    refl.SetValue(Any(static_cast<int32>(rawValue)));
                }
                else if (refl.GetValueType() == Type::Instance<int16>())
                {
                    refl.SetValue(Any(static_cast<int16>(rawValue)));
                }
                else if (refl.GetValueType() == Type::Instance<int8>())
                {
                    refl.SetValue(Any(static_cast<int8>(rawValue)));
                }
                else if (refl.GetValueType() == Type::Instance<float32>())
                {
                    refl.SetValue(Any(static_cast<float32>(rawValue)));
                }
            }
            else if (value.GetType() == Type::Instance<String>() &&
                     refl.GetValueType() == Type::Instance<WideString>())
            {
                const WideString& wstr = UTF8Utils::EncodeToWideString(value.Get<String>());
                refl.SetValue(Any(wstr));
            }
            else
            {
                refl.SetValue(value);
            }
        }
        catch (const LuaException& e)
        {
            return luaL_error(L, e.what());
        }

        return 0;
    }

    return 0;
}

void RegisterReflection(lua_State* L)
{
    static const luaL_reg Reflection_meta[] = {
        { "__tostring", &Reflection__tostring },
        { "__index", &Reflection__index },
        { "__newindex", &Reflection__newindex },
        { nullptr, nullptr }
    };

    luaL_newmetatable(L, ReflectionTName);
    luaL_register(L, 0, Reflection_meta);
    lua_pop(L, 1);
}

/******************************************************************************/

Any luaToAny(lua_State* L, int32 index)
{
    int ltype = lua_type(L, index);
    switch (ltype)
    {
    case LUA_TNIL:
        return Any();
    case LUA_TBOOLEAN:
        return Any(bool(lua_toboolean(L, index) != 0));
    case LUA_TNUMBER:
        return Any(float64(lua_tonumber(L, index)));
    case LUA_TSTRING:
        return Any(String(lua_tolstring(L, index, nullptr)));
    case LUA_TUSERDATA:

        if (void* ud = luaL_checkudata(L, index, AnyTName))
        {
            return Any(*static_cast<Any*>(ud));
        }
        else if (void* ud = luaL_checkudata(L, index, ReflectionTName))
        {
            return Any(*static_cast<Reflection*>(ud));
        }
        throw LuaException(ltype, "Unknown userdata type!");
    case LUA_TLIGHTUSERDATA:
    case LUA_TTABLE:
    case LUA_TFUNCTION:
    case LUA_TTHREAD:
    default:
        throw LuaException(ltype, Format("Unsupported Lua type \"%s\"!", lua_typename(L, ltype)));
    }
}

void anyToLua(lua_State* L, const Any& value)
{
    if (value.CanGet<int32>())
    {
        lua_pushinteger(L, value.Get<int32>());
    }
    else if (value.CanGet<int16>())
    {
        lua_pushinteger(L, value.Get<int16>());
    }
    else if (value.CanGet<int8>())
    {
        lua_pushinteger(L, value.Get<int8>());
    }
    else if (value.CanGet<float64>())
    {
        lua_pushnumber(L, value.Get<float64>());
    }
    else if (value.CanGet<float32>())
    {
        lua_pushnumber(L, value.Get<float32>());
    }
    else if (value.CanGet<const char*>())
    {
        const char* res = value.Get<const char*>();
        lua_pushlstring(L, res, strlen(res));
    }
    else if (value.CanGet<String>())
    {
        const String& res = value.Get<String>();
        lua_pushlstring(L, res.c_str(), res.length());
    }
    else if (value.CanGet<WideString>())
    {
        const WideString& res = value.Get<WideString>();
        const String& utf = UTF8Utils::EncodeToUTF8(res);
        lua_pushlstring(L, utf.c_str(), res.length());
    }
    else if (value.CanGet<bool>())
    {
        lua_pushboolean(L, value.Get<bool>());
    }
    else if (value.CanGet<Reflection>())
    {
        lua_pushdvreflection(L, value.Get<Reflection>());
    }
    else // unknown type, push as is
    {
        lua_pushdvany(L, value);
    }
}

String PopString(lua_State* L)
{
    String msg;
    if (lua_gettop(L) > 0)
    {
        msg = lua_tostring(L, -1);
        lua_pop(L, 1); // stack -1
    }
    return msg;
}
}
}