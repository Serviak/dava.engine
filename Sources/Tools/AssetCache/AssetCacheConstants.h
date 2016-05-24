#ifndef __DAVAENGINE_ASSET_CACHE_CONSTANTS_H__
#define __DAVAENGINE_ASSET_CACHE_CONSTANTS_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
namespace AssetCache
{
static const uint32 NET_SERVICE_ID = 0xACCA;
static const uint16 ASSET_SERVER_PORT = 0xACCA;

extern const String LOCALHOST;

enum ePacketID : uint8
{
    PACKET_UNKNOWN = 0,
    PACKET_ADD_REQUEST,
    PACKET_ADD_RESPONSE,
    PACKET_GET_REQUEST,
    PACKET_GET_RESPONSE,
    PACKET_WARMING_UP_REQUEST,
    //    PACKET_WARMING_UP_RESPONSE, // We don't need send response right now. Left it in code for better reading
    PACKET_COUNT
};

enum class Error : int32
{
    NO_ERRORS = 0,
    CODE_NOT_INITIALIZED,
    WRONG_COMMAND_LINE,
    WRONG_IP,
    OPERATION_TIMEOUT,
    CANNOT_CONNECT,
    SERVER_ERROR,
    NOT_FOUND_ON_SERVER,
    READ_FILES,
    ADDRESS_RESOLVER_FAILED,
    CANNOT_SEND_REQUEST_ADD,
    CANNOT_SEND_REQUEST_GET,
    CORRUPTED_DATA,

    ERRORS_COUNT,
};

String ErrorToString(Error error);

} // end of namespace AssetCache
} // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_CONSTANTS_H__
