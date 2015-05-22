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



#include "AssetCache/CacheDB.h"

#include "AssetCache/CacheItemKey.h"
#include "AssetCache/CachedFiles.h"
#include "AssetCache/ServerCacheEntry.h"

#include "FileSystem/File.h"
#include "FileSystem/KeyedArchive.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
    
namespace AssetCache
{
    
const String CacheDB::DB_FILE_NAME = "cache.dat";
    
CacheDB::CacheDB(const FilePath &folderPath, uint64 size, uint32 _itemsInMemory)
    : cacheRootFolder(folderPath)
    , storageSize(size)
    , itemsInMemory(_itemsInMemory)
{
    cacheRootFolder.MakeDirectoryPathname();
    cacheSettings = cacheRootFolder + DB_FILE_NAME;
    
    fastCache.reserve(itemsInMemory);
    
    Load();
}

CacheDB::~CacheDB()
{
    Save();
    
    fastCache.clear();
    
    for( auto & entry: fullCache)
    {
        entry.second.files.UnloadFiles();
    }
    fullCache.clear();
}
 
    
void CacheDB::Save() const
{
    ScopedPtr<File> file(File::Create(cacheSettings, File::CREATE | File::WRITE));
    if(static_cast<File*>(file) == nullptr)
    {
        Logger::Error("[CacheDB::%s] Cannot create file %s", __FUNCTION__, cacheSettings.GetStringValue().c_str());
        return;
    }

    ScopedPtr<KeyedArchive> header(new KeyedArchive());
    header->SetString("signature", "cache");
    header->SetInt32("version", 1);
    header->SetUInt64("usedSize", usedSize);
    header->SetUInt64("itemsCount", fullCache.size());
    header->Save(file);
    
    
    ScopedPtr<KeyedArchive> cache(new KeyedArchive());
    uint64 index = 0;
    for (auto & item: fullCache)
    {
        ScopedPtr<KeyedArchive> itemArchieve(new KeyedArchive());
        item.first.Serialize(itemArchieve);
        item.second.GetFiles().Serialize(itemArchieve, false);
        
        cache->SetArchive(Format("item_%d", index++), itemArchieve);
    }
    cache->Save(file);
}

void CacheDB::Load()
{
    DVASSERT(fastCache.size() == 0);
    DVASSERT(fullCache.size() == 0);
    
    ScopedPtr<File> file(File::Create(cacheSettings, File::OPEN | File::READ));
    if(static_cast<File*>(file) == nullptr)
    {
        Logger::Error("[CacheDB::%s] Cannot create file %s", __FUNCTION__, cacheSettings.GetStringValue().c_str());
        return;
    }
    
    ScopedPtr<KeyedArchive> header(new KeyedArchive());
    header->Load(file);
    
    if(header->GetString("signature") != "cache")
    {
        Logger::Error("[CacheDB::%s] Wrong signature %s", __FUNCTION__, header->GetString("signature").c_str());
        return;
    }

    if(header->GetInt32("version") != 1)
    {
        Logger::Error("[CacheDB::%s] Wrong version %d", __FUNCTION__, header->GetInt32("version"));
        return;
    }
    
    usedSize = header->GetUInt64("usedSize");
    auto cacheSize = header->GetUInt64("itemsCount");
    fullCache.reserve(cacheSize);

    ScopedPtr<KeyedArchive> cache(new KeyedArchive());
    cache->Load(file);
    
    for(uint64 index = 0; index < cacheSize; ++index)
    {
        KeyedArchive *itemArchieve = cache->GetArchive(Format("item_%d", index));
        DVASSERT(nullptr != itemArchieve);
        
        CacheItemKey key;
        key.Deserialize(itemArchieve);

        ServerCacheEntry entry;
        entry.files.Deserialize(itemArchieve);
        
        fullCache[key] = entry;
    }
}
    
ServerCacheEntry * CacheDB::Get(const CacheItemKey &key)
{
    ServerCacheEntry * entry = nullptr;

    {   //find in fast cache
        auto found = fastCache.find(key);
        if(found != fastCache.end())
        {
            entry = found->second;
        }
    }
    
    if(nullptr == entry)
    {   // find in full cache
        auto found = fullCache.find(key);
        if(found != fullCache.end())
        {
            entry = &found->second;
        }
    }
    
    if(nullptr != entry)
    {   // update access token
        entry->InvalidateAccesToken(nextItemID++);
    }
    
    return entry;
}

void CacheDB::Insert(const CacheItemKey &key, const CachedFiles &files)
{
    ServerCacheEntry entry(files);
    Insert(key, entry);
}
    
void CacheDB::Insert(const CacheItemKey &key, const ServerCacheEntry &entry)
{
    ServerCacheEntry *entryForFastCache = nullptr;
    
    auto found = fullCache.find(key);
    if(found != fullCache.end())
    {
        IncreaseUsedSize(found->second.GetFiles());
        
        found->second = entry;
        entryForFastCache = &found->second;
    }
    else
    {
        fullCache[key] = entry;
        entryForFastCache = &fullCache[key];
    }
    
    entryForFastCache->InvalidateAccesToken(nextItemID++);

    //TODO: insert in fast cache
    
    
    usedSize += entryForFastCache->GetFiles().GetFilesSize();

    auto savedPath = CreateFolderPath(key);
    entry.GetFiles().Save(savedPath);
}

void CacheDB::Remove(const CacheItemKey &key)
{
    fastCache.erase(key); //remove from fast cache

    auto found = fullCache.find(key);   //remove from full cache
    if(found != fullCache.end())
    {
        IncreaseUsedSize(found->second.GetFiles());
        fullCache.erase(found);
    }
    else
    {
        Logger::Error("[CacheDB::%s] Cannot find item in cache");
    }
}
    
void CacheDB::IncreaseUsedSize(const CachedFiles &files)
{
    auto fileSize = files.GetFilesSize();
    DVASSERT(fileSize <= usedSize);
    
    usedSize -= fileSize;
}

FilePath CacheDB::CreateFolderPath(const CacheItemKey &key) const
{
    constexpr auto HASH_STRING_SIZE = MD5::DIGEST_SIZE * 2 + 1;
    
    std::array<char8, HASH_STRING_SIZE> primaryHashBuffer;
    std::array<char8, HASH_STRING_SIZE> secondaryHashBuffer;
    
    MD5::HashToChar(key.keyData.hash.primary, primaryHashBuffer.data(), HASH_STRING_SIZE);
    MD5::HashToChar(key.keyData.hash.secondary, secondaryHashBuffer.data(), HASH_STRING_SIZE);
    
    auto folder = String(primaryHashBuffer.data()) + String(secondaryHashBuffer.data());
    return (cacheRootFolder + (folder + "/"));
}
    
const FilePath & CacheDB::GetPath() const
{
    return cacheRootFolder;
}

const uint64 CacheDB::GetStorageSize() const
{
    return storageSize;
}

const uint64 CacheDB::GetAvailableSize() const
{
    DVASSERT(GetStorageSize() > GetUsedSize());
    return (GetStorageSize() - GetUsedSize());
}

const uint64 CacheDB::GetUsedSize() const
{
    return usedSize;
}
    
    
void CacheDB::Dump()
{
    Logger::FrameworkDebug("======= [CacheDB::%s] =======", __FUNCTION__);

    Logger::FrameworkDebug(" == General Info ==");
    Logger::FrameworkDebug("\tstorageSize = %lld", GetStorageSize());
    Logger::FrameworkDebug("\tusedSize = %lld", GetUsedSize());
    Logger::FrameworkDebug("\tavailableSize = %lld", GetAvailableSize());
    
    Logger::FrameworkDebug(" == Files Info ==");
    Logger::FrameworkDebug(" FAST:");

    Logger::FrameworkDebug("\tentries count = %d", fastCache.size());
    size_t index = 0;
    for(auto & entry: fastCache)
    {
        auto & files = entry.second->GetFiles();
        auto & fileDescriptors = files.GetFiles();
        
        Logger::FrameworkDebug("\tentry[%d]:", index);
        Logger::FrameworkDebug("\t\taccessID = %d", entry.second->GetAccesID());
        Logger::FrameworkDebug("\t\tnames count = %d", fileDescriptors.size());
        
        for(auto &f: fileDescriptors)
        {
            Logger::FrameworkDebug("\t\tname: %s", f.first.GetStringValue().c_str());
        }
        
        ++index;
    }
    
    Logger::FrameworkDebug(" FULL:");
    Logger::FrameworkDebug("\tentries count = %d", fullCache.size());
    index = 0;
    for(auto & entry: fullCache)
    {
        auto & files = entry.second.GetFiles();
        auto & fileDescriptors = files.GetFiles();

        Logger::FrameworkDebug("\tentry[%d]:", index);
        Logger::FrameworkDebug("\t\taccessID = %d", entry.second.GetAccesID());
        Logger::FrameworkDebug("\t\tnames count = %d", fileDescriptors.size());
        
        for(auto &f: fileDescriptors)
        {
            Logger::FrameworkDebug("\t\tname: %s", f.first.GetStringValue().c_str());
        }

        ++index;
    }
    
    Logger::FrameworkDebug("======= [CacheDB::%s] =======", __FUNCTION__);
}
    
    
    
    
}; // end of namespace AssetCache
}; // end of namespace DAVA

