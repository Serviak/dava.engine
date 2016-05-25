#pragma once

#include "PackManager/Private/PacksDB.h"
#include "PackManager/Private/RequestManager.h"

namespace DAVA
{
struct PackPriorityComparator
{
    bool operator()(const PackManager::Pack* lhs, const PackManager::Pack* rhs) const
    {
        return lhs->priority < rhs->priority;
    }
};

class PackManagerImpl
{
public:
    PackManagerImpl() = default;

    void Initialize(const FilePath& dbFile_,
                    const FilePath& localPacksDir_,
                    const String& packUrlCommon,
                    const String& packUrlGpu,
                    Signal<const PackManager::Pack&, PackManager::Pack::Change>& signal);

    bool IsProcessingEnabled() const
    {
        return isProcessingEnabled;
    }

    void EnableProcessing()
    {
        if (!isProcessingEnabled)
        {
            isProcessingEnabled = true;
            requestManager->Start();
        }
    }

    void DisableProcessing()
    {
        if (isProcessingEnabled)
        {
            isProcessingEnabled = false;
            requestManager->Stop();
        }
    }

    void Update()
    {
        if (isProcessingEnabled)
        {
            requestManager->Update();
        }
    }

    const String& FindPack(const FilePath& relativePathInPack) const
    {
        return db->FindPack(relativePathInPack);
    }

    const PackManager::Pack& RequestPack(const String& packName, float32 priority);

    uint32 GetPackIndex(const String& packName)
    {
        auto it = packsIndex.find(packName);
        if (it != end(packsIndex))
        {
            return it->second;
        }
        throw std::runtime_error("can't find pack with name: " + packName);
    }

    PackManager::Pack& GetPack(const String& packName)
    {
        uint32 index = GetPackIndex(packName);
        return packs[index];
    }

    void MountDownloadedPacks();

    void DeletePack(const String& packName);

    const Vector<PackManager::Pack>& GetAllState() const
    {
        return packs;
    }

    const FilePath& GetLocalPacksDir() const
    {
        return localPacksDir;
    }

    const String& GetRemotePacksURL() const
    {
        return packsUrlCommon;
    }

    Signal<const PackManager::Pack&, PackManager::Pack::Change>* onPackChange;

private:
    FilePath dbFile;
    FilePath localPacksDir;
    String packsUrlCommon;
    bool isProcessingEnabled = false;
    PackManager* packManager = nullptr;
    UnorderedMap<String, uint32> packsIndex;
    Vector<PackManager::Pack> packs;
    std::unique_ptr<RequestManager> requestManager;
    std::unique_ptr<PacksDB> db;
};

} // end namespace DAVA