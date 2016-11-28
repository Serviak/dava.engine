#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Utils/QtConnections.h"

#include "QtTools/Utils/QtDelayedExecutor.h"

class ProjectManagerData;
class ProjectManagerModule : public DAVA::TArc::ClientModule
{
protected:
    void OnContextCreated(DAVA::TArc::DataContext& context) override;
    void OnContextDeleted(DAVA::TArc::DataContext& context) override;
    void PostInit() override;

private:
    void CreateActions();
    void RegisterOperations();

    void OpenProject();
    void OpenProjectByPath(const DAVA::FilePath& incomePath);
    void OpenProjectImpl(const DAVA::FilePath& incomePath);
    void OpenLastProject();
    void CloseProject();
    void ReloadSprites();

private:
    ProjectManagerData* GetData();

    void AddRecentProjectActions();
    void AddRecentProject(const DAVA::FilePath& projectPath);
    void RemoveRecentProjects();
    DAVA::Vector<DAVA::String> GetRecentProjects();

private:
    DAVA::TArc::QtConnections connections;
    QtDelayedExecutor delayedExecutor;
    QPointer<QAction> closeAction;
    std::unique_ptr<ProjectResources> projectResources;
};
