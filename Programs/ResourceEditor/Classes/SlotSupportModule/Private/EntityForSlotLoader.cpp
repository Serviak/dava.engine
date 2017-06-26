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

void EntityForSlotLoader::Load(DAVA::RefPtr<DAVA::Entity> rootEntity, const DAVA::FilePath& path, const DAVA::Function<void(DAVA::String&&)>& finishCallback)
{
    DVASSERT(scene != nullptr);
    SceneEditor2* editorScene = DAVA::DynamicTypeCheck<SceneEditor2*>(scene);

    DAVA::Entity* loadedEntity = editorScene->structureSystem->Load(path);
    CallbackInfo callbackInfo;
    callbackInfo.callback = finishCallback;

    if (loadedEntity != nullptr)
    {
        loadedEntity->SetNotRemovable(true);
        DAVA::CustomPropertiesComponent* propertiesComponent = DAVA::GetOrCreateCustomProperties(loadedEntity);
        propertiesComponent->GetArchive()->SetBool(ResourceEditor::EDITOR_IS_LOCKED, true);
        rootEntity->AddNode(loadedEntity);
        rootEntity->SetNotRemovable(true);
    }
    else
    {
        callbackInfo.message = DAVA::String("Can't load item");
    }

    callbacks.push_back(callbackInfo);
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

void EntityForSlotLoader::Process(DAVA::float32 delta)
{
    for (auto& callbackNode : callbacks)
    {
        callbackNode.callback(std::move(callbackNode.message));
    }

    callbacks.clear();
}

void EntityForSlotLoader::Reset()
{
    callbacks.clear();
}
