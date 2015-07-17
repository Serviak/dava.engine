/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_ASSET_CACHE_SERVER_H__
#define __DAVAENGINE_ASSET_CACHE_SERVER_H__

#include "Base/BaseTypes.h"

#include "AssetCache/TCPConnection/TCPConnection.h"

namespace DAVA
{

class TCPServer;
namespace AssetCache
{
 
class CachedFiles;
class CacheItemKey;

    
class ServerDelegate
{
public:
    
    virtual ~ServerDelegate() = default;
    
    virtual void OnAddToCache(DAVA::TCPChannel *tcpChannel, const CacheItemKey &key, const CachedFiles &files) = 0;
    virtual void OnRequestedFromCache(DAVA::TCPChannel *tcpChannel, const CacheItemKey &key) = 0;
    virtual void OnWarmingUp(DAVA::TCPChannel *tcpChannel, const CacheItemKey &key) = 0;

    virtual void OnChannelClosed(TCPChannel *tcpChannel, const char8* message) { } ;
};
    
    
class Server: public DAVA::TCPChannelDelegate
{
public:
    
    Server() = default;
    virtual ~Server();
    
    void SetDelegate(ServerDelegate * delegate);

    bool Listen(uint16 port);
    
    bool IsConnected() const;
    void Disconnect();
    
    uint16 GetListenPort() const;
    
    //TCPChannelDelegate
    void PacketReceived(DAVA::TCPChannel *tcpChannel, const void* packet, size_t length) override;
    void ChannelClosed(TCPChannel *tcpChannel, const char8* message) override;
    //END of TCPChannelDelegate
    
    bool FilesAddedToCache(DAVA::TCPChannel *tcpChannel, const CacheItemKey &key, bool added);
    bool SendFiles(DAVA::TCPChannel *tcpChannel, const CacheItemKey &key, const CachedFiles &files);
    
private:
    

    void OnAddToCache(DAVA::TCPChannel *tcpChannel, KeyedArchive * archieve);
    void OnGetFromCache(DAVA::TCPChannel *tcpChannel, KeyedArchive * archieve);
    void OnWarmingUp(DAVA::TCPChannel *tcpChannel, KeyedArchive * archieve);
    
private:
    
    uint16 listenPort = 0;
    std::unique_ptr<TCPConnection> netServer;
    ServerDelegate *delegate = nullptr;
};

inline uint16 Server::GetListenPort() const
{
    return listenPort;
}

inline void Server::SetDelegate(ServerDelegate * _delegate)
{
    delegate = _delegate;
}
    
    
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_SERVER_H__

