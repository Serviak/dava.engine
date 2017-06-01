#pragma once

#include "Scene3D/Systems/SlotSystem.h"

#include "Concurrency/Mutex.h"
#include "Base/RefPtr.h"
#include "Base/Hash.h"

namespace DAVA
{
class AsyncSlotExternalLoader final : public SlotSystem::ExternalEntityLoader, public std::enable_shared_from_this<AsyncSlotExternalLoader>
{
public:
    Entity* Load(const FilePath& path) override;
    void AddEntity(Entity* parent, Entity* child) override;
    void Process(float32 delta) override;

    void LoadImpl(RefPtr<Entity> rootEntity, const FilePath& path);

private:
    void ApplyNextJob();
    struct HashRefPtrEntity
    {
        size_t operator()(const RefPtr<Entity>& pointer) const;
    };

    UnorderedMap<RefPtr<Entity>, RefPtr<Scene>, HashRefPtrEntity> jobsMap;
    Mutex jobsMutex;

    Mutex queueMutes;
    struct LoadTask
    {
        RefPtr<Entity> rootEntity;
        FilePath filePath;
    };
    List<LoadTask> loadingQueue;
    bool isLoadingActive = false;
};

inline size_t AsyncSlotExternalLoader::HashRefPtrEntity::operator()(const RefPtr<Entity>& pointer) const
{
    return reinterpret_cast<size_t>(pointer.Get());
}

} // namespace DAVA
