#pragma once

#include <Base/BaseTypes.h>
#include <Network/ServiceRegistrar.h>
#include <Engine/Engine.h>

namespace DAVA
{
namespace Net
{
class NetService;
class ServicesProviderImpl;

/**
    ServicesProvider allows application to be detected by remote peers and to provide network services for that peers.

    Usage:
        1. create instance of ServicesProvider
        2. create instances of necessary network services. Only services from Modules/NetworkServices/ could be used.
        3. add these services to ServicesProvider instance
        4. call Start()

        Internally, on Start invocation, ServicesProvider will try to start all specified network services using first available TCP port.
        If that port is already occupied, ServicesProvider will try to use next available TCP port. First and last available TCP ports
        are specified in ServicesProvider.cpp
        
        If any application on the remote side wants to connnect to our ServicesProvider-application and its services, it should use PeerDescription and Discoverer classes.
        
    Example:
        ServiceProvider sp(engine, "MyAppName");
        NetLogger* netLoggerService = new NetLogger();
        sp->AddService(LOG_SERVICE_ID, netLoggerService);
        sp->Start();
*/

class ServicesProvider final
{
public:
    ServicesProvider(Engine& engine, const String& appName);
    ~ServicesProvider();

    void AddService(ServiceID serviceId, NetService* service);
    void Start();
    void Stop();

private:
    ServicesProviderImpl* impl = nullptr;
};
}
}
