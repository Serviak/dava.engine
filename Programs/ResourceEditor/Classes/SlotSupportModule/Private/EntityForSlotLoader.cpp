#include "Classes/SlotSupportModule/Private/EntityForSlotLoader.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

#include <Base/TemplateHelpers.h>

DAVA::Entity* EntityForSlotLoader::Load(const DAVA::FilePath& path)
{
    DVASSERT(scene != nullptr);
    SceneEditor2* editorScene = DAVA::DynamicTypeCheck<SceneEditor2*>(scene);

    return nullptr;
}

void EntityForSlotLoader::AddEntity(DAVA::Entity* parent, DAVA::Entity* child)
{
}
