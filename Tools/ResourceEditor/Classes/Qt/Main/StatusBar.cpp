#include "StatusBar.h"

#include "Platform/SystemTimer.h"

#include "Main/mainwindow.h"
#include "Scene/SceneEditor2.h"
#include "Scene/System/SelectionSystem.h"
#include "Commands2/Base/RECommand.h"

#include <QLabel>
#include <QLayout>
#include <QPalette>

StatusBar::StatusBar(QWidget* parent)
    : QStatusBar(parent)
{
    sceneGeometry = new QLabel(this);
    sceneGeometry->setToolTip("Resolution");
    sceneGeometry->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    addPermanentWidget(sceneGeometry);

    fpsCounter = new QLabel(this);
    fpsCounter->setToolTip("Current FPS for active scene");
    fpsCounter->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    addPermanentWidget(fpsCounter);

    distanceToCamera = new QLabel(this);
    distanceToCamera->setToolTip("Distance from camera to center of the selection");
    distanceToCamera->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    addPermanentWidget(distanceToCamera);

    selectionBoxSize = new QLabel(this);
    selectionBoxSize->setToolTip("Selection box size");
    selectionBoxSize->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    addPermanentWidget(selectionBoxSize);

    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setMargin(0);
    layout()->setSpacing(1);
    setStyleSheet("QStatusBar::item {border: none;}");
}

void StatusBar::SetDistanceToCamera(DAVA::float32 distance)
{
    distanceToCamera->setText(QString::fromStdString(DAVA::Format("%0.6f", distance)));
}

void StatusBar::ResetDistanceToCamera()
{
    distanceToCamera->setText(QString::fromStdString("No selection"));
}

void StatusBar::UpdateDistanceToCamera()
{
    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    if (!scene)
    {
        ResetDistanceToCamera();
        return;
    }

    if (scene->selectionSystem->GetSelectionCount() > 0)
    {
        DAVA::float32 distanceToCamera = scene->cameraSystem->GetDistanceToCamera();
        SetDistanceToCamera(distanceToCamera);
    }
    else
    {
        ResetDistanceToCamera();
    }
}

void StatusBar::SceneActivated(SceneEditor2* scene)
{
    DVASSERT(scene != nullptr);
    scene->selectionSystem->AddDelegate(this);

    UpdateDistanceToCamera();
    UpdateSelectionBoxSize(scene);
}

void StatusBar::SceneDeactivated(SceneEditor2* scene)
{
    DVASSERT(scene != nullptr);
    scene->selectionSystem->RemoveDelegate(this);
}

void StatusBar::SceneSelectionChanged(SceneEditor2* scene, const SelectableGroup* selected, const SelectableGroup* deselected)
{
    UpdateDistanceToCamera();
    UpdateSelectionBoxSize(scene);
}

void StatusBar::StructureChanged(SceneEditor2* scene, DAVA::Entity* parent)
{
    UpdateSelectionBoxSize(scene);
}

void StatusBar::UpdateByTimer()
{
    UpdateDistanceToCamera();
    UpdateFPS();
}

void StatusBar::OnSceneGeometryChaged(int width, int height)
{
    sceneGeometry->setText(QString::fromStdString(DAVA::Format("%d x %d", width, height)));
}

void StatusBar::UpdateSelectionBoxSize(SceneEditor2* scene)
{
    if (scene == nullptr)
    {
        return;
    }

    const SelectableGroup& selection = scene->selectionSystem->GetSelection();
    if (selection.IsEmpty())
    {
        selectionBoxSize->setText(QString());
        selectionBoxSize->setVisible(false);
    }
    else
    {
        OnSelectionBoxChanged(selection.GetIntegralBoundingBox());
        selectionBoxSize->setVisible(true);
    }
}

void StatusBar::UpdateFPS()
{
    SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
    DAVA::uint32 frames = 0;
    if (scene != nullptr)
    {
        frames = scene->GetFramesCount();
        scene->ResetFramesCount();
    }

    DAVA::uint64 currentTimeMS = DAVA::SystemTimer::Instance()->AbsoluteMS();

    if (frames > 0 && lastTimeMS != 0 && lastTimeMS != currentTimeMS)
    {
        DAVA::uint64 deltaTime = currentTimeMS - lastTimeMS;
        fpsCounter->setText(QString::fromStdString(DAVA::Format("FPS: %lld", frames * 1000 / deltaTime)));
    }
    else
    {
        fpsCounter->setText(QString::fromStdString("FPS: unknown"));
    }

    lastTimeMS = currentTimeMS;
}

void StatusBar::OnSelectionBoxChanged(const DAVA::AABBox3& newBox)
{
    if (newBox.IsEmpty())
    {
        selectionBoxSize->setText("Empty box");
    }
    else
    {
        DAVA::Vector3 size = newBox.GetSize();
        selectionBoxSize->setText(QString::fromStdString(DAVA::Format("x:%0.2f, y: %0.2f, z: %0.2f", size.x, size.y, size.z)));
    }
}
