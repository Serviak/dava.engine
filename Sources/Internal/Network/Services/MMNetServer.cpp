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

#include "Debug/DVAssert.h"
#include "FileSystem/Logger.h"

#include <Platform/SystemTimer.h>
#include "MemoryManager/MemoryManager.h"

#include "MMNetServer.h"

#if defined(DAVA_MEMORY_PROFILING_ENABLE)

#include <cstdlib>
#include <ctime>

namespace DAVA
{
namespace Net
{

MMNetServer::MMNetServer()
    : NetService()
    , sessionId(0)
    , commInited(false)
    , timerBegin(0)
    , statPeriod(250)
    , periodCounter(0)
{
    srand(static_cast<unsigned int>(time(nullptr)));
    sessionId = rand();
}

MMNetServer::~MMNetServer()
{

}

void MMNetServer::Update(float32 timeElapsed)
{
    if (!commInited) return;

    periodCounter += static_cast<size_t>(timeElapsed * 1000.0f);
    if (periodCounter >= statPeriod)
    {
        SendMemoryStat();
        periodCounter = 0;
    }
}

void MMNetServer::ChannelOpen()
{
    timerBegin = SystemTimer::Instance()->AbsoluteMS();
}

void MMNetServer::ChannelClosed(const char8* message)
{
    commInited = false;

    for (auto x : parcels)
        DestroyParcel(x);
    parcels.clear();
}

void MMNetServer::PacketReceived(const void* packet, size_t length)
{
    DVASSERT(length >= sizeof(MMProtoHeader));

    const MMProtoHeader* hdr = static_cast<const MMProtoHeader*>(packet);
    const eMMProtoCmd cmd = static_cast<eMMProtoCmd>(hdr->cmd);
    switch (cmd)
    {
    case eMMProtoCmd::INIT_COMM:
        ProcessInitCommunication(hdr, static_cast<const uint8*>(packet) + sizeof(MMProtoHeader), length - sizeof(MMProtoHeader));
        break;
    case eMMProtoCmd::DUMP:
        ProcessDump(hdr, static_cast<const uint8*>(packet)+sizeof(MMProtoHeader), length - sizeof(MMProtoHeader));
        break;
    default:
        break;
    }
}

void MMNetServer::PacketDelivered()
{
    DVASSERT(!parcels.empty());

    if (!commInited)
    {
        // As reply to eMMProtoCmd::INIT_COMM is always first delivered packet after connection
        // so we can simply set
        commInited = true;
    }

    Parcel& parcel = parcels.front();
    parcel.nsent += parcel.chunk;
    if (parcel.nsent >= parcel.size)
    {
        parcels.pop_front();
        MMProtoHeader* hdr = static_cast<MMProtoHeader*>(parcel.buffer);
        if (hdr->cmd == static_cast<uint32>(eMMProtoCmd::DUMP))
            MemoryManager::FreeDump(parcel.buffer);
        else
            DestroyParcel(parcel);

        if (!parcels.empty())
        {
            Parcel& next = parcels.front();
            size_t n = std::min(next.chunk, next.size);
            Send(next.buffer, n);
        }
    }
    else
    {
        size_t n = std::min(parcel.chunk, parcel.size - parcel.nsent);
        Send((uint8*)parcel.buffer + parcel.nsent, n);
    }
}

void MMNetServer::ProcessInitCommunication(const MMProtoHeader* hdr, const void* packet, size_t length)
{
    size_t dataSize = 0;
    if (hdr->sessionId != sessionId)
    {
        dataSize = MemoryManager::CalcStatConfigSize();
    }
    
    Parcel parcel = CreateParcel(sizeof(MMProtoHeader) + dataSize);
    if (dataSize > 0)
    {
        MMStatConfig* config = reinterpret_cast<MMStatConfig*>(static_cast<uint8*>(parcel.buffer) + sizeof(MMProtoHeader));
        MemoryManager::GetStatConfig(config);

        /*Logger::Debug("MMNetServer::ProcessInitCommunication: new session %u", sessionId);
        Logger::Debug("   dataSize=%u", (uint32)dataSize);
        Logger::Debug("   maxTags=%u, ntags=%u", config->maxTagCount, config->tagCount);
        for (uint32 i = 0;i < config->tagCount;++i)
            Logger::Debug("      %d, %s", i, config->names[i].name);
        Logger::Debug("   maxPools=%u, npools=%u", config->maxAllocPoolCount, config->allocPoolCount);
        for (uint32 i = 0;i < config->allocPoolCount;++i)
            Logger::Debug("      %d, %s", i, config->names[i + config->tagCount].name);*/
    }
    
    MMProtoHeader* outHdr = static_cast<MMProtoHeader*>(parcel.buffer);
    outHdr->sessionId = sessionId;
    outHdr->cmd = static_cast<uint32>(eMMProtoCmd::INIT_COMM);
    outHdr->status = static_cast<uint32>(eMMProtoStatus::ACK);
    outHdr->length = static_cast<uint32>(dataSize);
    
    EnqueueAndSend(parcel);
}

void MMNetServer::ProcessDump(const MMProtoHeader* hdr, const void* packet, size_t length)
{
    void* buf = nullptr;
    uint64 timerStart = SystemTimer::Instance()->AbsoluteMS();
    size_t dataSize = MemoryManager::GetDump(sizeof(MMProtoHeader), &buf, 0, uint32(-1));

    Parcel parcel = CreateParcel(dataSize, buf);

    MMProtoHeader* outHdr = static_cast<MMProtoHeader*>(parcel.buffer);
    MMDump* dump = reinterpret_cast<MMDump*>(outHdr + 1);
    dump->timestampBegin = timerStart - timerBegin;
    dump->timestampEnd = SystemTimer::Instance()->AbsoluteMS() - timerBegin;

    outHdr->sessionId = sessionId;
    outHdr->cmd = static_cast<uint32>(eMMProtoCmd::DUMP);
    outHdr->status = static_cast<uint32>(eMMProtoStatus::ACK);
    outHdr->length = static_cast<uint32>(dataSize - sizeof(MMProtoHeader));

    EnqueueAndSend(parcel);
}

void MMNetServer::SendMemoryStat()
{
    size_t dataSize = MemoryManager::CalcStatSize();
    Parcel parcel = CreateParcel(sizeof(MMProtoHeader) + dataSize);

    MMProtoHeader* hdr = static_cast<MMProtoHeader*>(parcel.buffer);
    MMStat* stat = reinterpret_cast<MMStat*>(hdr + 1);

    MemoryManager::GetStat(stat);
    stat->timestamp = SystemTimer::Instance()->AbsoluteMS() - timerBegin;

    hdr->sessionId = sessionId;
    hdr->cmd = static_cast<uint32>(eMMProtoCmd::CUR_STAT);
    hdr->status = static_cast<uint32>(eMMProtoStatus::ACK);
    hdr->length = static_cast<uint32>(dataSize);

    EnqueueAndSend(parcel);
}

MMNetServer::Parcel MMNetServer::CreateParcel(size_t parcelSize)
{
    Parcel parcel = {
        parcelSize,
        0,
        CHUNK_SIZE,
        new uint8[parcelSize]
    };
    return parcel;
}

MMNetServer::Parcel MMNetServer::CreateParcel(size_t parcelSize, void* buf)
{
    Parcel parcel = {
        parcelSize,
        0,
        CHUNK_SIZE,
        buf
    };
    return parcel;
}

void MMNetServer::DestroyParcel(Parcel parcel)
{
    delete [] static_cast<uint8*>(parcel.buffer);
}

void MMNetServer::EnqueueAndSend(Parcel parcel)
{
    bool wasEmpty = parcels.empty();
    parcels.push_back(parcel);
    if (wasEmpty)
    {
        size_t n = std::min(parcel.chunk, parcel.size);
        Send(parcel.buffer, n);
    }
}

}   // namespace Net
}   // namespace DAVA

#endif  // defined(DAVA_MEMORY_PROFILING_ENABLE)
