#pragma once

#include <QObject>
#include <QPointer>

#include <Tools/NetworkHelpers/ChannelListenerAsync.h>

#include <Network/PeerDesription.h>
#include <Network/NetService.h>
#include <Network/IChannel.h>

class LogWidget;

class DeviceLogController : public QObject,
                            public DAVA::Net::NetService,
                            public std::enable_shared_from_this<DAVA::Net::NetService>
{
    Q_OBJECT

public:
    explicit DeviceLogController(const DAVA::Net::PeerDescription& peerDescr, QWidget* parentWidget, QObject* parent = NULL);
    ~DeviceLogController();
    void Init();

    void ShowView();

    virtual void ChannelOpen();
    virtual void ChannelClosed(const DAVA::char8* message);
    virtual void PacketReceived(const void* packet, size_t length);

    DAVA::Net::IChannelListener* GetAsyncChannelListener()
    {
        return channelListenerAsync.get();
    }

private:
    void Output(const DAVA::String& msg);

private:
    QPointer<LogWidget> view;
    QPointer<QWidget> parentWidget;
    DAVA::Net::PeerDescription peer;
    std::unique_ptr<DAVA::Net::ChannelListenerAsync> channelListenerAsync;
};
