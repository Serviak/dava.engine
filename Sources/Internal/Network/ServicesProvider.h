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

class ServicesProvider
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
