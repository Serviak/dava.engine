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


#include "ServerCore.h"
#include <QTimer>

ServerCore::ServerCore()
	: state(State::STOPPED)
    , remoteState(RemoteState::STOPPED)
{
    settings = new ApplicationSettings();
    QObject::connect(settings, &ApplicationSettings::SettingsUpdated, this, &ServerCore::OnSettingsUpdated);
    
    server.SetDelegate(&serverLogics);
	client.SetDelegate(&serverLogics);
    serverLogics.Init(&server, &client, &dataBase);

    updateTimer = new QTimer(this);
    QObject::connect(updateTimer, &QTimer::timeout, this, &ServerCore::UpdateByTimer);

    settings->Load();
}

ServerCore::~ServerCore()
{
    Stop();
    delete settings;
}

void ServerCore::Start()
{
    if (state != State::STARTED)
    {
        bool started = StartListening();
        if (started)
        {
            const auto remoteServer = settings->GetCurrentServer();
            ConnectRemote(remoteServer);

            updateTimer->start(UPDATE_INTERVAL_MS);

            emit ServerStateChanged(this);
        }
    }
}

void ServerCore::Stop()
{
    if (state != State::STOPPED)
    {
        updateTimer->stop();

        StopListening();
        DisconnectRemote();

        emit ServerStateChanged(this);
    }
}

bool ServerCore::StartListening()
{
    bool established = server.Listen(settings->GetPort());

    if (established)
    {
        state = State::STARTED;
        return true;
    }
    else
    {
        state = State::STOPPED;
        return false;
    }
}

void ServerCore::StopListening()
{
    state = State::STOPPED;
    server.Disconnect();
}

bool ServerCore::ConnectRemote(const ServerData& remote)
{
    if (!remote.ip.empty())
    {
        bool created = client.Connect(remote.ip, remote.port);
        if (created)
        {
            client.SetDelegate(this);
            remoteState = RemoteState::STARTING;
            return true;
        }
        else
        {
            client.SetDelegate(nullptr);
            remoteState = RemoteState::STOPPED;
            return false;
        }
    }

    return false;
}

void ServerCore::DisconnectRemote()
{
    client.SetDelegate(nullptr);
    remoteState = RemoteState::STOPPED;
    client.Disconnect();
}

void ServerCore::Update()
{
    serverLogics.Update();
    
    auto netSystem = DAVA::Net::NetCore::Instance();
    if(netSystem)
    {
        netSystem->Poll();
    }
}

void ServerCore::UpdateByTimer()
{
    Update();
}

void ServerCore::OnAssetClientStateChanged()
{
    DVASSERT(remoteState != RemoteState::STOPPED);

    remoteState = client.IsConnected() ? RemoteState::STARTED : RemoteState::STARTING;
    emit ServerStateChanged(this);
}

void ServerCore::OnSettingsUpdated(const ApplicationSettings *_settings)
{
    if(settings == _settings)
    {
        auto & folder = settings->GetFolder();
        auto sizeGb = settings->GetCacheSizeGb();
        auto count = settings->GetFilesCount();
        auto autoSaveTimeoutMin = settings->GetAutoSaveTimeoutMin();
        const auto remoteServer = settings->GetCurrentServer();

        bool needServerRestart = false; //TODO: why we need different places for disconnect and listen???
        bool needClientRestart = false;

        if(state == State::STARTED)
        {   // disconnect network if settings changed
            if (server.GetListenPort() != settings->GetPort())
            {
                needServerRestart = true;
                StopListening();
            }
            
            auto clientConnection = client.GetConnection();
            if ((remoteState != RemoteState::STOPPED && clientConnection && remoteServer != clientConnection->GetEndpoint()) || 
                (remoteState == RemoteState::STOPPED && !remoteServer.ip.empty()))
            {
                needClientRestart = true;
                DisconnectRemote();
            }
        }
        
        {   //updated DB settings
            if(sizeGb && !folder.IsEmpty())
            {
                dataBase.UpdateSettings(folder, sizeGb * 1024 * 1024 * 1024, count, autoSaveTimeoutMin * 60 * 1000);
            }
            else
            {
                DAVA::Logger::Warning("[ServerCore::%s] Empty settings", __FUNCTION__);
            }
        }

        {   // restart network connections after changing of settings
            if (needServerRestart)
            {
                StartListening();
            }
            
            if(state == State::STARTED && needClientRestart)
            {
                ConnectRemote(remoteServer);
            }
        }

        if (needServerRestart || needClientRestart)
        {
            emit ServerStateChanged(this);
            return;
        }
    }
    else
    {
        DAVA::Logger::Error("[ServerCore::%s] Wrong settings", __FUNCTION__);
    }
}


ApplicationSettings * ServerCore::GetSettings() const
{
    return settings;
}

