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

#ifndef __DAVAENGINE_NETCORE_H__
#define __DAVAENGINE_NETCORE_H__

#include <Base/BaseTypes.h>
#include <Base/Function.h>
#include <Base/Singleton.h>

#include <Network/Base/IOLoop.h>
#include <Network/Base/IfAddress.h>
#include <Network/Base/Endpoint.h>
#include <Network/ServiceRegistrar.h>
#include <Network/IController.h>

namespace DAVA
{
namespace Net
{

class NetConfig;
class NetCore : public Singleton<NetCore>
{
public:
    typedef uintptr_t TrackId;
    static const TrackId INVALID_TRACK_ID = 0;

public:
    NetCore();
    ~NetCore();

    IOLoop* Loop() { return &loop; }

    bool RegisterService(uint32 serviceId, ServiceCreator creator, ServiceDeleter deleter, const char8* serviceName = NULL);
    void UnregisterAllServices();
    const char8* ServiceName(uint32 serviceId) const;

    TrackId CreateController(const NetConfig& config, void* context = NULL);
    TrackId CreateAnnouncer(const Endpoint& endpoint, uint32 sendPeriod, Function<size_t (size_t, void*)> needDataCallback);
    TrackId CreateDiscoverer(const Endpoint& endpoint, Function<void (size_t, const void*, const Endpoint&)> dataReadyCallback);
    void DestroyController(TrackId id);
    void DestroyAllControllers(Function<void ()> callback);

    int32 Run();
    int32 Poll();
    void Finish(bool runOutLoop = false);

    const Vector<IfAddress>& InstalledInterfaces() const;

private:
    void DoStart(IController* ctrl);
    void DoDestroy(TrackId id);
    void DoDestroyAll();
    void AllDestroyed();
    IController* GetTrackedObject(TrackId id) const;
    void TrackedObjectStopped(IController* obj);

    TrackId ObjectToTrackId(const IController* obj) const;
    IController* TrackIdToObject(TrackId id) const;

private:
    IOLoop loop;                                    // Heart of NetCore and networ library - event loop
    Set<IController*> trackedObjects;               // Running objects
    Set<IController*> dyingObjects;
    ServiceRegistrar registrar;
    Vector<IfAddress> installedInterfaces;
    Function<void ()> controllersStoppedCallback;
    bool isFinishing;
};

//////////////////////////////////////////////////////////////////////////
inline bool NetCore::RegisterService(uint32 serviceId, ServiceCreator creator, ServiceDeleter deleter, const char8* serviceName)
{
    return registrar.Register(serviceId, creator, deleter, serviceName);
}

inline void NetCore::UnregisterAllServices()
{
    registrar.UnregisterAll();
}

inline const char8* NetCore::ServiceName(uint32 serviceId) const
{
    return registrar.Name(serviceId);
}

inline const Vector<IfAddress>& NetCore::InstalledInterfaces() const
{
    return installedInterfaces;
}

inline int32 NetCore::Run()
{
    return loop.Run(IOLoop::RUN_DEFAULT);
}

inline int32 NetCore::Poll()
{
    return loop.Run(IOLoop::RUN_NOWAIT);
}

inline NetCore::TrackId NetCore::ObjectToTrackId(const IController* obj) const
{
    return reinterpret_cast<TrackId>(obj);
}

inline IController* NetCore::TrackIdToObject(TrackId id) const
{
    return reinterpret_cast<IController*>(id);
}

}   // namespace Net
}   // namespace DAVA


#endif  // __DAVAENGINE_NETCORE_H__
