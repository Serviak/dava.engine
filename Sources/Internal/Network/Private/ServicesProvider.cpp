#include "Logger/Logger.h"
#include "Network/ServicesProvider.h"
#include "Network/NetService.h"
#include "Network/PeerDesription.h"
#include "Network/NetCore.h"
#include "Network/NetConfig.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
namespace Net
{
namespace ServicesProviderDetails
{
uint16 FIRST_ALLOWED_TCP_PORT = 10000; // number of first TCP port allowed to be occupied for providing of specified network services
uint16 LAST_ALLOWED_TCP_PORT = 10010; // numert of last TCP port allowed
float32 WAITING_CONTROLLER_START_MS = 2000.f;
}

class ServicesProvider::ServicesProviderImpl
{
public:
    ServicesProviderImpl(Engine& engine, const String& appName);
    ~ServicesProviderImpl();
    void AddService(ServiceID serviceId, std::shared_ptr<NetService>& service);
    void Start();
    void Stop();

private:
    void StartServicesController();
    void StartAnnounceController();
    void StopServicesController();
    void StopAnnounceController();
    void TryUseNextPort();
    size_t AnnounceDataSupplier(size_t length, void* buffer);
    void OnUpdate(float32);

private:
    struct ServiceContext
    {
        ServiceContext(std::shared_ptr<NetService>& service)
            : netService(service)
        {
        }

        IChannelListener* ServiceCreatorFn(ServiceID serviceId, void*);
        void ServiceDeleterFn(IChannelListener* obj, void*);

        std::shared_ptr<NetService> netService;
        bool serviceInUse = false;
    };

private:
    Engine& engine;
    String applicationName;

    std::unique_ptr<Net::NetConfig> config;
    PeerDescription peerDescr;

    NetCore::TrackId id_anno = Net::NetCore::INVALID_TRACK_ID;
    NetCore::TrackId id_net = Net::NetCore::INVALID_TRACK_ID;

    uint16 servicesPort = 0;

    Map<ServiceID, ServiceContext> services;

    SigConnectionID updateSignalId = 0;
    float32 elapsedSinceStartMs = 0.f;

    bool isStarted = false;
};

ServicesProvider::ServicesProviderImpl::ServicesProviderImpl(Engine& engine, const String& appName)
    : engine(engine)
    , applicationName(appName)
{
}

ServicesProvider::ServicesProviderImpl::~ServicesProviderImpl()
{
    Stop();
    for (auto& serviceEntry : services)
    {
        NetCore::Instance()->UnregisterService(serviceEntry.first);
    }
}

IChannelListener* ServicesProvider::ServicesProviderImpl::ServiceContext::ServiceCreatorFn(ServiceID serviceId, void*)
{
    if (!serviceInUse)
    {
        serviceInUse = true;
        return netService.get();
    }
    return nullptr;
};

void ServicesProvider::ServicesProviderImpl::ServiceContext::ServiceDeleterFn(IChannelListener* obj, void*)
{
    serviceInUse = false;
}

void ServicesProvider::ServicesProviderImpl::AddService(ServiceID serviceId, std::shared_ptr<NetService>& service)
{
    DVASSERT(isStarted == false);
    DVASSERT(service);

    auto result = services.emplace(serviceId, ServiceContext(service));

    if (result.second == false)
    {
        DVASSERT(false, Format("Service %u is already added", serviceId).c_str());
        return;
    }

    ServiceContext& serviceContext = result.first->second;
    ServiceCreator serviceCreator = MakeFunction(&serviceContext, &ServiceContext::ServiceCreatorFn);
    ServiceDeleter serviceDeleter = MakeFunction(&serviceContext, &ServiceContext::ServiceDeleterFn);
    NetCore::Instance()->RegisterService(serviceId, serviceCreator, serviceDeleter);
}

void ServicesProvider::ServicesProviderImpl::Start()
{
    DVASSERT(services.empty() == false);
    DVASSERT(isStarted == false);
    servicesPort = ServicesProviderDetails::FIRST_ALLOWED_TCP_PORT;
    updateSignalId = engine.update.Connect(this, &ServicesProviderImpl::OnUpdate);
    StartServicesController();
}

void ServicesProvider::ServicesProviderImpl::StartServicesController()
{
    eNetworkRole role = SERVER_ROLE;
    Net::Endpoint endpoint = Net::Endpoint(servicesPort);

#ifdef __DAVAENGINE_WIN_UAP__
    role = UAPNetworkHelper::GetCurrentNetworkRole();
    endpoint = UAPNetworkHelper::GetCurrentEndPoint();
#endif

    config.reset(new NetConfig(role));
    config->AddTransport(TRANSPORT_TCP, endpoint);
    for (auto serviceEntry : services)
    {
        config->AddService(serviceEntry.first);
    }

    elapsedSinceStartMs = 0.f;
    id_net = NetCore::Instance()->CreateController(*config, nullptr);
}

void ServicesProvider::ServicesProviderImpl::StartAnnounceController()
{
    engine.update.Disconnect(updateSignalId);
    peerDescr = PeerDescription(applicationName, *config);
    Net::Endpoint annoUdpEndpoint(NetCore::defaultAnnounceMulticastGroup, NetCore::DEFAULT_UDP_ANNOUNCE_PORT);
    Net::Endpoint annoTcpEndpoint(NetCore::DEFAULT_TCP_ANNOUNCE_PORT);
    id_anno = NetCore::Instance()->CreateAnnouncer(annoUdpEndpoint, DEFAULT_ANNOUNCE_TIME_PERIOD, MakeFunction(this, &ServicesProviderImpl::AnnounceDataSupplier), annoTcpEndpoint);
}

void ServicesProvider::ServicesProviderImpl::StopServicesController()
{
    if (id_net != Net::NetCore::INVALID_TRACK_ID)
    {
        NetCore::Instance()->DestroyControllerBlocked(id_net);
        id_net = Net::NetCore::INVALID_TRACK_ID;
    }
}

void ServicesProvider::ServicesProviderImpl::StopAnnounceController()
{
    if (id_anno != Net::NetCore::INVALID_TRACK_ID)
    {
        NetCore::Instance()->DestroyControllerBlocked(id_anno);
        id_anno = Net::NetCore::INVALID_TRACK_ID;
    }
}

void ServicesProvider::ServicesProviderImpl::Stop()
{
    isStarted = false;
    StopAnnounceController();
    StopServicesController();
}

void ServicesProvider::ServicesProviderImpl::OnUpdate(float32 elapsedMs)
{
    IController::Status status = NetCore::Instance()->GetControllerStatus(id_net);
    switch (status)
    {
    case IController::STARTED:
    {
        StartAnnounceController();
        break;
    }
    case IController::START_FAILED:
    {
        TryUseNextPort();
        break;
    }
    default:
    {
        elapsedSinceStartMs += elapsedMs;
        if (elapsedSinceStartMs >= ServicesProviderDetails::WAITING_CONTROLLER_START_MS)
        {
            TryUseNextPort();
        }
        break;
    }
    }
}

void ServicesProvider::ServicesProviderImpl::TryUseNextPort()
{
    StopServicesController();
    if (++servicesPort <= ServicesProviderDetails::LAST_ALLOWED_TCP_PORT)
    {
        StartServicesController();
    }
    else
    {
        Logger::Error("Can't start net services controller: no more allowed ports");
    }
}

size_t ServicesProvider::ServicesProviderImpl::AnnounceDataSupplier(size_t length, void* buffer)
{
    if (true == peerDescr.NetworkInterfaces().empty())
    {
        peerDescr.SetNetworkInterfaces(NetCore::Instance()->InstalledInterfaces());
    }
    return peerDescr.Serialize(buffer, length);
}

//////////////////////////////////////////////////////////////////////////

ServicesProvider::ServicesProvider(Engine& engine, const String& appName)
{
    impl = std::make_unique<ServicesProviderImpl>(engine, appName);
}

ServicesProvider::~ServicesProvider()
{
}

void ServicesProvider::AddService(ServiceID serviceId, std::shared_ptr<NetService>& service)
{
    impl->AddService(serviceId, service);
}

void ServicesProvider::Start()
{
    impl->Start();
}

void ServicesProvider::Stop()
{
    impl->Stop();
}

} // NetCore
} // DAVA
