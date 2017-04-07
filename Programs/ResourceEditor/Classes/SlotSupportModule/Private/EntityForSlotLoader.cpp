#include "Classes/SlotSupportModule/Private/EntityForSlotLoader.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Deprecated/SceneValidator.h"

#include <TArc/Core/ContextAccessor.h>
#include <Base/TemplateHelpers.h>

EntityForSlotLoader::EntityForSlotLoader(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
{
}

DAVA::Entity* EntityForSlotLoader::Load(const DAVA::FilePath& path)
{
    DVASSERT(scene != nullptr);
    SceneEditor2* editorScene = DAVA::DynamicTypeCheck<SceneEditor2*>(scene);

    return editorScene->structureSystem->Load(path, true);
}

void EntityForSlotLoader::AddEntity(DAVA::Entity* parent, DAVA::Entity* child)
{
    parent->AddNode(child);

    SceneValidator validator;
    ProjectManagerData* data = accessor->GetGlobalContext()->GetData<ProjectManagerData>();
    if (data)
    {
        validator.SetPathForChecking(data->GetProjectPath());
    }

    DVASSERT(scene != nullptr);
    SceneEditor2* editorScene = DAVA::DynamicTypeCheck<SceneEditor2*>(scene);
    validator.ValidateScene(editorScene, editorScene->GetScenePath());
}
