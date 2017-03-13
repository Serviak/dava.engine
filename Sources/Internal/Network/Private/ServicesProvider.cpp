#include "Logger/Logger.h"
#include "Network/ServicesProvider.h"
#include "Network/NetService.h"
#include "Network/PeerDesription.h"
#include "Network/NetCore.h"
#include "Network/NetConfig.h"

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

class ServicesProviderImpl
{
public:
    ServicesProviderImpl(Engine& engine, const String& appName);
    ~ServicesProviderImpl();
    void AddService(ServiceID serviceId, NetService* service);
    void Start();
    void Stop();

private:
    void StartServicesController();
    void StartAnnounceController();
    void TryUseNextPort();
    size_t AnnounceDataSupplier(size_t length, void* buffer);
    void OnUpdate(float32);

private:
    Engine& engine;

    String applicationName;

    std::unique_ptr<Net::NetConfig> config;
    PeerDescription peerDescr;

    NetCore::TrackId id_anno = Net::NetCore::INVALID_TRACK_ID;
    NetCore::TrackId id_net = Net::NetCore::INVALID_TRACK_ID;

    uint16 servicesPort = 0;

    struct ServiceContext
    {
        ServiceContext(NetService* service)
            : netService(service)
        {
        }

        IChannelListener* ServiceCreatorFn(ServiceID serviceId, void*);
        void ServiceDeleterFn(IChannelListener* obj, void*);

        NetService* netService = nullptr;
        ServiceCreator serviceCreator;
        ServiceDeleter serviceDeleter;
        bool serviceInUse = false;
    };
    Map<ServiceID, ServiceContext> services;

    SigConnectionID updateSignalId = 0;
    float32 elapsedSinceStartMs = 0.f;

    bool wasStopped = false;
};

ServicesProviderImpl::ServicesProviderImpl(Engine& engine, const String& appName)
    : engine(engine)
    , applicationName(appName)
{
}

ServicesProviderImpl::~ServicesProviderImpl()
{
    Stop();
}

IChannelListener* ServicesProviderImpl::ServiceContext::ServiceCreatorFn(ServiceID serviceId, void*)
{
    if (!serviceInUse)
    {
        serviceInUse = true;
        return netService;
    }
    return nullptr;
};

void ServicesProviderImpl::ServiceContext::ServiceDeleterFn(IChannelListener* obj, void*)
{
    serviceInUse = false;
}

void ServicesProviderImpl::AddService(ServiceID serviceId, NetService* service)
{
    DVASSERT(service != nullptr);
    auto result = services.emplace(serviceId, ServiceContext(service));
    ServiceContext& serviceContext = result.first->second;

    NetCore::Instance()->RegisterService(serviceId, MakeFunction(&serviceContext, &ServiceContext::ServiceCreatorFn), MakeFunction(&serviceContext, &ServiceContext::ServiceDeleterFn));
}

void ServicesProviderImpl::Start()
{
    DVASSERT(services.empty() == false);
    servicesPort = ServicesProviderDetails::FIRST_ALLOWED_TCP_PORT;
    updateSignalId = engine.update.Connect(this, &ServicesProviderImpl::OnUpdate);
    StartServicesController();
}

void ServicesProviderImpl::StartServicesController()
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

void ServicesProviderImpl::StartAnnounceController()
{
    engine.update.Disconnect(updateSignalId);
    peerDescr = PeerDescription(applicationName, *config);
    Net::Endpoint annoUdpEndpoint(NetCore::defaultAnnounceMulticastGroup, NetCore::DEFAULT_UDP_ANNOUNCE_PORT);
    Net::Endpoint annoTcpEndpoint(NetCore::DEFAULT_TCP_ANNOUNCE_PORT);
    id_anno = NetCore::Instance()->CreateAnnouncer(annoUdpEndpoint, DEFAULT_ANNOUNCE_TIME_PERIOD, MakeFunction(this, &ServicesProviderImpl::AnnounceDataSupplier), annoTcpEndpoint);
}

void ServicesProviderImpl::Stop()
{
    if (!wasStopped)
    {
        wasStopped = true;
        NetCore::Instance()->DestroyControllerBlocked(id_anno);
        NetCore::Instance()->DestroyControllerBlocked(id_net);
    }
}

void ServicesProviderImpl::OnUpdate(float32 elapsedMs)
{
    IController::Status status;
    if (!NetCore::Instance()->GetControllerStatus(id_net, status) || status == IController::START_FAILED || status == IController::STARTED_SOME_FAILED)
    {
        TryUseNextPort();
    }
    else if (status == IController::STARTED)
    {
        StartAnnounceController();
    }
    else
    {
        elapsedSinceStartMs += elapsedMs;
        if (elapsedSinceStartMs >= ServicesProviderDetails::WAITING_CONTROLLER_START_MS)
        {
            TryUseNextPort();
        }
    }
}

void ServicesProviderImpl::TryUseNextPort()
{
    NetCore::Instance()->DestroyControllerBlocked(id_net);
    if (++servicesPort <= ServicesProviderDetails::LAST_ALLOWED_TCP_PORT)
    {
        StartServicesController();
    }
    else
    {
        Logger::Error("Can't start net services controller: no more allowed ports");
    }
}

size_t ServicesProviderImpl::AnnounceDataSupplier(size_t length, void* buffer)
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
    impl = new ServicesProviderImpl(engine, appName);
}

ServicesProvider::~ServicesProvider()
{
    SafeDelete(impl);
}

void ServicesProvider::AddService(ServiceID serviceId, NetService* service)
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
