#include "PropertyPanel.h"
#include "Classes/Qt/Main/mainwindow.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "NgtTools/Reflection/ReflectionBridge.h"
#include "NgtTools/Common/GlobalContext.h"

#include "Scene3D/Entity.h"
#include "Debug/DVAssert.h"
#include "metainfo/PropertyPanel.mpp"

#include <core_command_system/i_command_manager.hpp>
#include <core_reflection/i_definition_manager.hpp>
#include <core_reflection/interfaces/i_reflection_controller.hpp>
#include <core_data_model/reflection/reflected_property_model.hpp>
#include <core_data_model/i_tree_model.hpp>
#include <core_qt_common/helpers/qt_helpers.hpp>

#include <QTimerEvent>

PropertyPanel::PropertyPanel()
{
    wgt::IComponentContext* context = NGTLayer::GetGlobalContext();
    DVASSERT(context != nullptr);
    model.reset(new wgt::ReflectedPropertyModel(*context));
    model->registerExtension(std::make_shared<EntityChildCreatorExtension>());
    model->registerExtension(std::make_shared<EntityMergeValueExtension>());
    model->registerExtension(std::make_shared<PropertyPanelGetExtension>());
    model->registerExtension(std::make_shared<EntityInjectDataExtension>(*this, *context));
}

PropertyPanel::~PropertyPanel()
{
}

void PropertyPanel::Initialize(wgt::IUIFramework& uiFramework, wgt::IUIApplication& uiApplication)
{
    wgt::IDefinitionManager* defMng = NGTLayer::queryInterface<wgt::IDefinitionManager>();
    DVASSERT(defMng != nullptr);
    defMng->registerDefinition(std::unique_ptr<wgt::IClassDefinitionDetails>(new wgt::TypeClassDefinition<PropertyPanel>()));

    view = uiFramework.createView("Views/PropertyPanel.qml", wgt::IUIFramework::ResourceType::Url, this);
    view->registerListener(this);
    uiApplication.addView(*view);
}

void PropertyPanel::Finalize(wgt::IUIApplication& uiApplication)
{
    killTimer(updateTimerId);
    selectedObjects.clear();
    SetObject(selectedObjects);
    view->deregisterListener(this);
    view.reset();
    model.reset();
}

wgt::ObjectHandle PropertyPanel::GetPropertyTree() const
{
    return wgt::ObjectHandleT<wgt::AbstractItemModel>(model.get());
}

void PropertyPanel::SetPropertyTree(const wgt::ObjectHandle& /*dummyTree*/)
{
}

void PropertyPanel::SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected)
{
    selectedObjects.clear();
    const SelectableGroup::CollectionType& selectedContent = selected->GetContent();
    selectedObjects.reserve(selectedContent.size());
    std::transform(selectedContent.begin(), selectedContent.end(),
                   std::back_inserter(selectedObjects),
                   std::bind(&Selectable::GetContainedObject, std::placeholders::_1));

    isSelectionDirty = false;
    if (visible || selected->IsEmpty())
    {
        SetObject(selectedObjects);
    }
    else
    {
        isSelectionDirty = true;
    }
}

void PropertyPanel::SetObject(const std::vector<DAVA::InspBase*>& davaObjects)
{
    DVASSERT(model != nullptr);

    wgt::IDefinitionManager* defMng = NGTLayer::queryInterface<wgt::IDefinitionManager>();
    DVASSERT(defMng != nullptr);

    std::vector<wgt::ObjectHandle> objects;
    for (DAVA::InspBase* object : davaObjects)
    {
        const DAVA::InspInfo* info = object->GetTypeInfo();
        NGTLayer::RegisterType(*defMng, info);

        wgt::IClassDefinition* definition = defMng->getDefinition(info->Type()->GetTypeName());
        objects.push_back(NGTLayer::CreateObjectHandle(*defMng, info, object));
    }

    model->setObjects(objects);
}

void PropertyPanel::timerEvent(QTimerEvent* e)
{
    if (e->timerId() == updateTimerId)
    {
        UpdateModel();
        e->accept();
    }
}

void PropertyPanel::onFocusIn(wgt::IView* view_)
{
    DVASSERT(view_ == view.get());
    visible = true;
    if (isSelectionDirty)
    {
        SetObject(selectedObjects);
        isSelectionDirty = false;
    }

    if (updateTimerId == -1)
    {
        updateTimerId = startTimer(1000);
    }
}

void PropertyPanel::onFocusOut(wgt::IView* view_)
{
    DVASSERT(view_ == view.get());
    visible = false;

    if (updateTimerId != -1)
    {
        killTimer(updateTimerId);
        updateTimerId = -1;
    }
}

void PropertyPanel::UpdateModel()
{
    model->update();
}

void PropertyPanel::BeginBatch(const DAVA::String& name, DAVA::uint32 commandCount)
{
    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    DVASSERT(scene != nullptr);

    scene->BeginBatch(name, commandCount);
}

void PropertyPanel::Exec(Command2::Pointer&& command)
{
    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    DVASSERT(scene != nullptr);

    scene->Exec(std::move(command));
}

void PropertyPanel::EndBatch()
{
    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    DVASSERT(scene != nullptr);

    scene->EndBatch();
}

void PropertyPanel::onLoaded(wgt::IView* view)
{
}
