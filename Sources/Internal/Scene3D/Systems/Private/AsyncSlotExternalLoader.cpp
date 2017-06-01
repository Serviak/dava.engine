#include "Scene3D/Systems/Private/AsyncSlotExternalLoader.h"

#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Concurrency/LockGuard.h"
#include "Job/JobManager.h"
#include "Logger/Logger.h"
#include "Base/FastName.h"

namespace DAVA
{
Entity* AsyncSlotExternalLoader::Load(const FilePath& path)
{
    RefPtr<Entity> rootEntity;
    rootEntity.ConstructInplace();
    rootEntity->SetName(path.GetBasename().c_str());

    {
        LockGuard<Mutex> guard(jobsMutex);
        jobsMap.emplace(rootEntity, RefPtr<Scene>());
    }

    {
        LockGuard<Mutex> guard(queueMutes);
        loadingQueue.push_back(LoadTask());
        LoadTask& task = loadingQueue.back();
        task.filePath = path;
        task.rootEntity = rootEntity;
    }

    ApplyNextJob();
    return rootEntity.Get();
}

void AsyncSlotExternalLoader::AddEntity(Entity* parent, Entity* child)
{
    parent->AddNode(child);
}

void AsyncSlotExternalLoader::Process(float32 delta)
{
    LockGuard<Mutex> guard(jobsMutex);
    auto currentIter = jobsMap.begin();
    auto endIter = jobsMap.end();
    while (currentIter != endIter)
    {
        RefPtr<Entity> rootEntity = currentIter->first;
        RefPtr<Scene> scene = currentIter->second;
        if (scene.Get() != nullptr)
        {
            for (int32 childIndex = 0; childIndex < scene->GetChildrenCount(); ++childIndex)
            {
                rootEntity->AddNode(scene->GetChild(childIndex));
            }

            currentIter = jobsMap.erase(currentIter);
        }
        else
        {
            ++currentIter;
        }
    }
}

void AsyncSlotExternalLoader::LoadImpl(RefPtr<Entity> rootEntity, const FilePath& path)
{
#if defined(__DAVAENGINE_DEBUG__)
    {
        LockGuard<Mutex> guard(jobsMutex);
        DVASSERT(jobsMap.count(rootEntity) > 0);
    }
#endif // DEBUG

    RefPtr<Scene> scene;
    scene.ConstructInplace();
    SceneFileV2::eError result = scene->LoadScene(path);
    if (result != SceneFileV2::ERROR_NO_ERROR)
    {
        Logger::Error("[AsyncSlotExternalLoader] Couldn't load scene %s", path.GetAbsolutePathname().c_str());
        LockGuard<Mutex> guard(jobsMutex);
        jobsMap.erase(rootEntity);
        return;
    }

    {
        LockGuard<Mutex> guard(jobsMutex);
        jobsMap[rootEntity] = scene;
    }

    {
        LockGuard<Mutex> loadingGuard(queueMutes);
        isLoadingActive = false;
    }

    ApplyNextJob();
}

void AsyncSlotExternalLoader::ApplyNextJob()
{
    {
        LockGuard<Mutex> loadingGuard(queueMutes);
        if (isLoadingActive == true)
        {
            return;
        }

        if (loadingQueue.empty() == true)
        {
            return;
        }

        isLoadingActive = true;
    }

    JobManager* jobMng = GetEngineContext()->jobManager;
    std::shared_ptr<AsyncSlotExternalLoader> loaderRef = shared_from_this();
    LoadTask task = loadingQueue.front();
    loadingQueue.pop_front();
    jobMng->CreateWorkerJob([loaderRef, task]()
                            {
                                loaderRef->LoadImpl(task.rootEntity, task.filePath);
                            });
}

} // namespace DAVA