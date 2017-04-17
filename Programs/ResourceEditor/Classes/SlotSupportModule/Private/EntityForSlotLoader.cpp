#include "Classes/SlotSupportModule/Private/EntityForSlotLoader.h"
#include "Classes/Selection/SelectionSystem.h"
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

    return editorScene->structureSystem->Load(path);
}

void EntityForSlotLoader::AddEntity(DAVA::Entity* parent, DAVA::Entity* child)
{
    DVASSERT(scene != nullptr);
    SceneEditor2* editorScene = DAVA::DynamicTypeCheck<SceneEditor2*>(scene);

    child->SetNotRemovable(true);
    DAVA::CustomPropertiesComponent* propertiesComponent = DAVA::GetOrCreateCustomProperties(child);
    propertiesComponent->GetArchive()->SetBool(ResourceEditor::EDITOR_CONST_REFERENCE, true);

    SelectionSystem* selectionSystem = editorScene->LookupEditorSystem<SelectionSystem>();
    DVASSERT(selectionSystem != nullptr);

    bool isLocked = selectionSystem->IsLocked();
    selectionSystem->SetLocked(true);
    parent->AddNode(child);
    selectionSystem->SetLocked(isLocked);

    SceneValidator validator;
    ProjectManagerData* data = accessor->GetGlobalContext()->GetData<ProjectManagerData>();
    if (data)
    {
        validator.SetPathForChecking(data->GetProjectPath());
    }
    validator.ValidateScene(editorScene, editorScene->GetScenePath());
}
