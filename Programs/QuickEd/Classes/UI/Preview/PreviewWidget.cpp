#include "PreviewWidget.h"

#include "Application/QEGlobal.h"

#include "EditorSystems/EditorSystemsManager.h"

#include "EditorSystems/EditorCanvas.h"
#include "EditorSystems/CursorSystem.h"

#include "UI/Preview/Ruler/RulerWidget.h"
#include "UI/Preview/Ruler/RulerController.h"
#include "UI/Preview/Guides/GuidesController.h"

#include "UI/Find/Widgets/FindInDocumentWidget.h"
#include "UI/Package/PackageMimeData.h"
#include "UI/CommandExecutor.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/PackageBaseNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"

#include "Modules/DocumentsModule/DocumentData.h"
#include "UI/Preview/Data/CentralWidgetData.h"
#include "UI/Preview/Data/CanvasData.h"

#include "Controls/ScaleComboBox.h"

#include <TArc/Controls/SceneTabbar.h>
#include <TArc/Controls/ScrollBar.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/OperationInvoker.h>
#include <TArc/DataProcessing/DataContext.h>

#include <QtTools/Updaters/ContinuousUpdater.h>

#include <UI/UIControl.h>
#include <UI/Text/UITextComponent.h>
#include <UI/UIControlSystem.h>
#include <Engine/Engine.h>
#include <Reflection/ReflectedTypeDB.h>

#include <QLineEdit>
#include <QScreen>
#include <QMenu>
#include <QShortcut>
#include <QFileInfo>
#include <QInputDialog>
#include <QComboBox>
#include <QScrollBar>
#include <QGridLayout>

#include <QApplication>
#include <QTimer>

using namespace DAVA;

PreviewWidget::PreviewWidget(DAVA::TArc::ContextAccessor* accessor_, DAVA::TArc::OperationInvoker* invoker_, DAVA::TArc::UI* ui_, DAVA::RenderWidget* renderWidget, EditorSystemsManager* systemsManager)
    : QFrame(nullptr)
    , accessor(accessor_)
    , invoker(invoker_)
    , ui(ui_)
    , rulerController(new RulerController(accessor, this))
    , scaleComboBoxData(accessor)
    , hScrollBarData(Vector2::AXIS_X, accessor)
    , vScrollBarData(Vector2::AXIS_Y, accessor)
    , canvasDataAdapter(accessor)
{
    InjectRenderWidget(renderWidget);

    InitUI();

    InitFromSystemsManager(systemsManager);

    centralWidgetDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<CentralWidgetData>());
}

PreviewWidget::~PreviewWidget() = default;

FindInDocumentWidget* PreviewWidget::GetFindInDocumentWidget()
{
    return findInDocumentWidget;
}

void PreviewWidget::CreateActions()
{
    QAction* importPackageAction = new QAction(tr("Import package"), this);
    importPackageAction->setShortcut(QKeySequence::New);
    importPackageAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(importPackageAction, &QAction::triggered, this, &PreviewWidget::ImportRequested);
    renderWidget->addAction(importPackageAction);

    QAction* cutAction = new QAction(tr("Cut"), this);
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(cutAction, &QAction::triggered, this, &PreviewWidget::CutRequested);
    renderWidget->addAction(cutAction);

    QAction* copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(copyAction, &QAction::triggered, this, &PreviewWidget::CopyRequested);
    renderWidget->addAction(copyAction);

    QAction* pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(pasteAction, &QAction::triggered, this, &PreviewWidget::PasteRequested);
    renderWidget->addAction(pasteAction);

    QAction* duplicateAction = new QAction(tr("Duplicate"), this);
    duplicateAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    duplicateAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(duplicateAction, &QAction::triggered, this, &PreviewWidget::DuplicateRequested);
    addAction(duplicateAction);

    QAction* deleteAction = new QAction(tr("Delete"), this);
#if defined Q_OS_WIN
    deleteAction->setShortcut(QKeySequence(QKeySequence::Delete));
#elif defined Q_OS_MAC
    deleteAction->setShortcuts({ QKeySequence::Delete, QKeySequence(Qt::Key_Backspace) });
#endif // platform

    deleteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(deleteAction, &QAction::triggered, this, &PreviewWidget::DeleteRequested);
    renderWidget->addAction(deleteAction);

    selectAllAction = new QAction(tr("Select all"), this);
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    selectAllAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    renderWidget->addAction(selectAllAction);

    focusNextChildAction = new QAction(tr("Focus next child"), this);
    focusNextChildAction->setShortcut(Qt::Key_Tab);
    focusNextChildAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    renderWidget->addAction(focusNextChildAction);

    focusPreviousChildAction = new QAction(tr("Focus previous child"), this);
    focusPreviousChildAction->setShortcut(static_cast<int>(Qt::ShiftModifier | Qt::Key_Tab));
    focusPreviousChildAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    renderWidget->addAction(focusPreviousChildAction);
}

void PreviewWidget::OnEmulationModeChanged(bool emulationMode)
{
    systemsManager->SetEmulationMode(emulationMode);

    if (emulationMode)
    {
        focusNextChildAction->setShortcut(0);
        focusNextChildAction->setEnabled(false);

        focusPreviousChildAction->setShortcut(0);
        focusPreviousChildAction->setEnabled(false);
    }
    else
    {
        focusNextChildAction->setShortcut(Qt::Key_Tab);
        focusNextChildAction->setEnabled(true);

        focusPreviousChildAction->setShortcut(static_cast<int>(Qt::ShiftModifier | Qt::Key_Tab));
        focusPreviousChildAction->setEnabled(true);
    }
}

void PreviewWidget::OnIncrementScale()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    CanvasData* canvasData = activeContext->GetData<CanvasData>();

    float32 nextScale = canvasData->GetNextScale(1);
    canvasDataAdapter.SetScale(nextScale);
}

void PreviewWidget::OnDecrementScale()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    CanvasData* canvasData = activeContext->GetData<CanvasData>();

    float32 nextScale = canvasData->GetPreviousScale(-1);
    canvasDataAdapter.SetScale(nextScale);
}

void PreviewWidget::SetActualScale()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    canvasDataAdapter.SetScale(1.0f);
}

void PreviewWidget::OnResized(DAVA::uint32 width, DAVA::uint32 height)
{
    const EngineContext* engineContext = GetEngineContext();
    VirtualCoordinatesSystem* vcs = engineContext->uiControlSystem->vcs;
    vcs->UnregisterAllAvailableResourceSizes();
    vcs->SetVirtualScreenSize(width, height);
    vcs->RegisterAvailableResourceSize(width, height, "Gfx");
    vcs->RegisterAvailableResourceSize(width, height, "Gfx2");
}

void PreviewWidget::InitFromSystemsManager(EditorSystemsManager* systemsManager_)
{
    DVASSERT(nullptr == systemsManager);
    systemsManager = systemsManager_;

    connect(focusNextChildAction, &QAction::triggered, std::bind(&EditorSystemsManager::FocusNextChild, systemsManager));
    connect(focusPreviousChildAction, &QAction::triggered, std::bind(&EditorSystemsManager::FocusPreviousChild, systemsManager));
    connect(selectAllAction, &QAction::triggered, std::bind(&EditorSystemsManager::SelectAll, systemsManager));

    editorCanvas = new EditorCanvas(systemsManager, accessor);

    systemsManager->AddEditorSystem(editorCanvas);

    CursorSystem* cursorSystem = new CursorSystem(renderWidget, systemsManager, accessor);
    systemsManager->AddEditorSystem(cursorSystem);
}

void PreviewWidget::InjectRenderWidget(DAVA::RenderWidget* renderWidget_)
{
    DVASSERT(renderWidget_ != nullptr);
    renderWidget = renderWidget_;
    CreateActions();

    renderWidget->resized.Connect(this, &PreviewWidget::OnResized);

    renderWidget->SetClientDelegate(this);
}

void PreviewWidget::InitUI()
{
    using namespace DAVA::TArc;

    GuidesController* hGuidesController = new GuidesController(Vector2::AXIS_X, accessor, this);
    GuidesController* vGuidesController = new GuidesController(Vector2::AXIS_Y, accessor, this);

    QVBoxLayout* vLayout = new QVBoxLayout(this);

    DAVA::TArc::DataContext* ctx = accessor->GetGlobalContext();
    DAVA::TArc::SceneTabbar* tabBar = new DAVA::TArc::SceneTabbar(accessor, DAVA::Reflection::Create(&accessor), this);
    tabBar->closeTab.Connect(&requestCloseTab, &DAVA::Signal<DAVA::uint64>::Emit);
    tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar, &QWidget::customContextMenuRequested, this, &PreviewWidget::OnTabBarContextMenuRequested);

    tabBar->setElideMode(Qt::ElideNone);
    tabBar->setTabsClosable(true);
    tabBar->setUsesScrollButtons(true);
    vLayout->addWidget(tabBar);

    findInDocumentWidget = new FindInDocumentWidget(this);

    vLayout->addWidget(findInDocumentWidget);

    QGridLayout* gridLayout = new QGridLayout();
    vLayout->addLayout(gridLayout);
    RulerWidget* horizontalRuler = new RulerWidget(accessor, hGuidesController, this);
    horizontalRuler->SetRulerOrientation(Qt::Horizontal);
    horizontalRuler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    connect(rulerController, &RulerController::HorisontalRulerSettingsChanged, horizontalRuler, &RulerWidget::OnRulerSettingsChanged);
    connect(rulerController, &RulerController::HorisontalRulerMarkPositionChanged, horizontalRuler, &RulerWidget::OnMarkerPositionChanged);
    gridLayout->addWidget(horizontalRuler, 0, 1);

    RulerWidget* verticalRuler = new RulerWidget(accessor, vGuidesController, this);
    verticalRuler->SetRulerOrientation(Qt::Vertical);
    verticalRuler->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    connect(rulerController, &RulerController::VerticalRulerSettingsChanged, verticalRuler, &RulerWidget::OnRulerSettingsChanged);
    connect(rulerController, &RulerController::VerticalRulerMarkPositionChanged, verticalRuler, &RulerWidget::OnMarkerPositionChanged);
    gridLayout->addWidget(verticalRuler, 1, 0);

    DAVA::TArc::DataContext* globalContext = accessor->GetGlobalContext();
    globalContext->CreateData(std::make_unique<CentralWidgetData>(renderWidget, horizontalRuler, verticalRuler));

    QSizePolicy expandingPolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    renderWidget->setSizePolicy(expandingPolicy);
    gridLayout->addWidget(renderWidget, 1, 1);
    {
        ScrollBar::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
        params.fields[ScrollBar::Fields::Value] = ScrollBarAdapter::positionPropertyName;
        params.fields[ScrollBar::Fields::Minimum] = ScrollBarAdapter::minPosPropertyName;
        params.fields[ScrollBar::Fields::Maximum] = ScrollBarAdapter::maxPosPropertyName;
        params.fields[ScrollBar::Fields::PageStep] = ScrollBarAdapter::pageStepPropertyName;
        params.fields[ScrollBar::Fields::Orientation] = ScrollBarAdapter::orientationPropertyName;
        params.fields[ScrollBar::Fields::Enabled] = ScrollBarAdapter::enabledPropertyName;
        params.fields[ScrollBar::Fields::Visible] = ScrollBarAdapter::visiblePropertyName;
        {
            ScrollBar* scrollBar = new ScrollBar(params, accessor, Reflection::Create(ReflectedObject(&vScrollBarData)));
            scrollBar->ForceUpdate();
            gridLayout->addWidget(scrollBar->ToWidgetCast(), 1, 2);
        }

        {
            ScrollBar* scrollBar = new ScrollBar(params, accessor, Reflection::Create(ReflectedObject(&hScrollBarData)));
            scrollBar->ForceUpdate();
            gridLayout->addWidget(scrollBar->ToWidgetCast(), 2, 1);
        }
    }

    gridLayout->setMargin(0.0f);
    gridLayout->setSpacing(1.0f);

    vLayout->setMargin(0.0f);
    vLayout->setSpacing(1.0f);

    {
        ScaleComboBox::Params params(accessor, ui, DAVA::TArc::mainWindowKey);
        params.fields[ScaleComboBox::Fields::Enumerator] = ScaleComboBoxAdapter::enumeratorPropertyName;
        params.fields[ScaleComboBox::Fields::Value] = ScaleComboBoxAdapter::scalePropertyName;
        params.fields[ScaleComboBox::Fields::Enabled] = ScaleComboBoxAdapter::enabledPropertyName;
        ScaleComboBox* scaleCombo = new ScaleComboBox(params, accessor, Reflection::Create(ReflectedObject(&scaleComboBoxData)));

        QString toolbarName = "Document toolBar";
        ActionPlacementInfo toolBarScalePlacement(CreateMenuPoint(QList<QString>() << "View"
                                                                                   << "Toolbars"));
        ui->DeclareToolbar(DAVA::TArc::mainWindowKey, toolBarScalePlacement, toolbarName);

        QAction* action = new QAction(nullptr);
        QWidget* container = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(container);
        layout->addWidget(new QLabel(tr("Scale")));
        layout->addWidget(scaleCombo->ToWidgetCast());
        layout->addWidget(new QLabel(tr("%")));

        AttachWidgetToAction(action, container);

        ActionPlacementInfo placementInfo(CreateToolbarPoint(toolbarName));
        ui->AddAction(DAVA::TArc::mainWindowKey, placementInfo, action);
    }
}

void PreviewWidget::ShowMenu(const QMouseEvent* mouseEvent)
{
    QMenu menu;
    //separator must be added by the client code, which call AddSelectionMenuSection function
    QPoint localPos = mouseEvent->pos();
    if (AddSelectionMenuSection(&menu, localPos))
    {
        menu.addSeparator();
    }
    Vector2 davaPoint(localPos.x(), localPos.y());
    ControlNode* node = systemsManager->GetControlNodeAtPoint(davaPoint);
    if (CanChangeTextInControl(node))
    {
        QString name = QString::fromStdString(node->GetName());
        QAction* action = menu.addAction(tr("Change text in %1").arg(name));
        connect(action, &QAction::triggered, [this, node]() { requestChangeTextInNode.Emit(node); });
    }
    if (!menu.actions().isEmpty())
    {
        menu.exec(mouseEvent->globalPos());
    }
}

bool PreviewWidget::AddSelectionMenuSection(QMenu* menu, const QPoint& pos)
{
    using namespace DAVA;
    using namespace TArc;
    Vector<ControlNode*> nodesUnderPoint;
    Vector2 davaPos(pos.x(), pos.y());
    auto predicateForMenu = [davaPos](const ControlNode* node) -> bool
    {
        const UIControl* control = node->GetControl();
        DVASSERT(nullptr != control);
        const VisibleValueProperty* visibleProp = node->GetRootProperty()->GetVisibleProperty();
        return visibleProp->GetVisibleInEditor() && control->IsPointInside(davaPos);
    };
    auto stopPredicate = [](const ControlNode* node) -> bool {
        const auto visibleProp = node->GetRootProperty()->GetVisibleProperty();
        return !visibleProp->GetVisibleInEditor();
    };
    systemsManager->CollectControlNodes(std::back_inserter(nodesUnderPoint), predicateForMenu, stopPredicate);

    //create list of item to select
    for (auto it = nodesUnderPoint.rbegin(); it != nodesUnderPoint.rend(); ++it)
    {
        ControlNode* controlNode = *it;
        QString className = QString::fromStdString(controlNode->GetControl()->GetClassName());
        QAction* action = new QAction(QString::fromStdString(controlNode->GetName()), menu);
        action->setCheckable(true);
        menu->addAction(action);

        DataContext* activeContext = accessor->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        DocumentData* data = activeContext->GetData<DocumentData>();
        const SelectedNodes& selectedNodes = data->GetSelectedNodes();

        if (selectedNodes.find(controlNode) != selectedNodes.end())
        {
            action->setChecked(true);
        }
        connect(action, &QAction::toggled, [this, controlNode]() {
            systemsManager->SelectNode(controlNode);
        });
    }
    return !nodesUnderPoint.empty();
}

bool PreviewWidget::CanChangeTextInControl(const ControlNode* node) const
{
    if (node == nullptr)
    {
        return false;
    }

    UIControl* control = node->GetControl();

    UITextComponent* textComponent = control->GetComponent<UITextComponent>();
    return textComponent != nullptr;
}

void PreviewWidget::OnMouseReleased(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
    {
        DVASSERT(nodeToChangeTextOnMouseRelease == nullptr);
        ShowMenu(event);
    }

    if (nodeToChangeTextOnMouseRelease)
    {
        QPoint point = event->pos();
        Vector2 davaPoint(point.x(), point.y());
        ControlNode* node = systemsManager->GetControlNodeAtPoint(davaPoint);
        if (node == nodeToChangeTextOnMouseRelease && CanChangeTextInControl(node))
        {
            requestChangeTextInNode.Emit(node);
        }
        nodeToChangeTextOnMouseRelease = nullptr;
    }
}

void PreviewWidget::OnMouseDBClick(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        QPoint point = event->pos();
        Vector2 davaPoint(point.x(), point.y());
        nodeToChangeTextOnMouseRelease = systemsManager->GetControlNodeAtPoint(davaPoint);
    }
}

void PreviewWidget::OnMouseMove(QMouseEvent* event)
{
    DVASSERT(nullptr != event);
    rulerController->UpdateRulerMarkers(event->pos());
}

void PreviewWidget::OnDragEntered(QDragEnterEvent* event)
{
    event->accept();
}

void PreviewWidget::OnDragMoved(QDragMoveEvent* event)
{
    DVASSERT(nullptr != event);
    ProcessDragMoveEvent(event) ? event->accept() : event->ignore();
}

bool PreviewWidget::ProcessDragMoveEvent(QDropEvent* event)
{
    DVASSERT(nullptr != event);
    auto mimeData = event->mimeData();
    if (mimeData->hasFormat("text/uri-list"))
    {
        QStringList strList = mimeData->text().split("\n");
        for (const auto& str : strList)
        {
            QUrl url(str);
            if (url.isLocalFile())
            {
                QString path = url.toLocalFile();
                QFileInfo fileInfo(path);
                return fileInfo.isFile() && fileInfo.suffix() == "yaml";
            }
        }
    }
    else if (mimeData->hasFormat("text/plain") || mimeData->hasFormat(PackageMimeData::MIME_TYPE))
    {
        QPoint pos = event->pos();
        DAVA::Vector2 davaPos(pos.x(), pos.y());
        ControlNode* node = systemsManager->GetControlNodeAtPoint(davaPos);
        systemsManager->HighlightNode(node);

        if (nullptr != node)
        {
            if (node->IsReadOnly())
            {
                return false;
            }
            else
            {
                if (mimeData->hasFormat(PackageMimeData::MIME_TYPE))
                {
                    const PackageMimeData* controlMimeData = DynamicTypeCheck<const PackageMimeData*>(mimeData);
                    const Vector<ControlNode*>& srcControls = controlMimeData->GetControls();
                    for (const auto& srcNode : srcControls)
                    {
                        if (srcNode == node)
                        {
                            return false;
                        }
                    }
                }
                return true;
            }
        }
        else
        {
            //root node will be added
            return true;
        }
    }
    return false;
}

void PreviewWidget::OnDragLeaved(QDragLeaveEvent*)
{
    systemsManager->ClearHighlight();
}

void PreviewWidget::OnDrop(QDropEvent* event)
{
    systemsManager->ClearHighlight();
    DVASSERT(nullptr != event);
    auto mimeData = event->mimeData();
    if (mimeData->hasFormat("text/plain") || mimeData->hasFormat(PackageMimeData::MIME_TYPE))
    {
        Vector2 pos(event->pos().x(), event->pos().y());
        PackageBaseNode* node = systemsManager->GetControlNodeAtPoint(pos);
        String string = mimeData->text().toStdString();
        auto action = event->dropAction();
        uint32 index = 0;
        if (node == nullptr)
        {
            DAVA::TArc::DataContext* active = accessor->GetActiveContext();
            DVASSERT(active != nullptr);
            const DocumentData* data = active->GetData<DocumentData>();
            DVASSERT(data != nullptr);
            const PackageNode* package = data->GetPackageNode();
            node = DynamicTypeCheck<PackageBaseNode*>(package->GetPackageControlsNode());
            index = systemsManager->GetIndexOfNearestRootControl(pos);
        }
        else
        {
            index = node->GetCount();
        }
        emit DropRequested(mimeData, action, node, index, &pos);
    }
    else if (mimeData->hasFormat("text/uri-list"))
    {
        QStringList list = mimeData->text().split("\n");
        Vector<FilePath> packages;
        for (const QString& str : list)
        {
            QUrl url(str);
            if (url.isLocalFile())
            {
                emit OpenPackageFile(url.toLocalFile());
            }
        }
    }
    renderWidget->setFocus();
}

void PreviewWidget::OnKeyPressed(QKeyEvent* event)
{
    using namespace DAVA;
    using namespace TArc;
    if (event->isAutoRepeat())
    {
        return;
    }
    int key = event->key();
    if (key == Qt::Key_Enter || key == Qt::Key_Return)
    {
        DataContext* active = accessor->GetActiveContext();
        if (active == nullptr)
        {
            return;
        }
        DocumentData* data = active->GetData<DocumentData>();
        const SelectedNodes& selectedNodes = data->GetSelectedNodes();
        if (selectedNodes.size() == 1)
        {
            ControlNode* node = dynamic_cast<ControlNode*>(*selectedNodes.begin());
            if (CanChangeTextInControl(node))
            {
                requestChangeTextInNode.Emit(node);
            }
        }
    }
}

void PreviewWidget::OnTabBarContextMenuRequested(const QPoint& pos)
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    Vector<uint64> allIDs;
    accessor->ForEachContext([&allIDs](const DataContext& context) {
        allIDs.push_back(context.GetID());
    });

    //no tabs at all, do nothing
    if (allIDs.empty())
    {
        return;
    }

    QTabBar* tabBar = findChild<QTabBar*>();
    DVASSERT(tabBar != nullptr);

    int index = tabBar->tabAt(pos);
    if (index == -1)
    {
        return;
    }

    QVariant data = tabBar->tabData(index);
    DVASSERT(data.canConvert<uint64>());
    uint64 currentId = data.value<uint64>();

    QMenu menu(this);
    QAction* closeTabAction = new QAction(tr("Close tab"), &menu);
    QAction* closeOtherTabsAction = new QAction(tr("Close other tabs"), &menu);
    QAction* closeAllTabsAction = new QAction(tr("Close all tabs"), &menu);
    QAction* selectInFileSystemAction = new QAction(tr("Select in File System"), &menu);

    closeOtherTabsAction->setEnabled(allIDs.size() > 1);

    menu.addAction(closeTabAction);
    menu.addAction(closeOtherTabsAction);
    menu.addAction(closeAllTabsAction);
    menu.addSeparator();
    menu.addAction(selectInFileSystemAction);

    connect(closeAllTabsAction, &QAction::triggered, [this, allIDs]()
            {
                for (uint64 id : allIDs)
                {
                    requestCloseTab.Emit(id);
                }
            });

    connect(closeTabAction, &QAction::triggered, std::bind(&Signal<uint64>::Emit, &requestCloseTab, currentId));

    connect(closeOtherTabsAction, &QAction::triggered, [this, allIDs, currentId]()
            {
                for (uint64 id : allIDs)
                {
                    if (id != currentId)
                    {
                        requestCloseTab.Emit(id);
                    }
                }
            });

    connect(selectInFileSystemAction, &QAction::triggered, [this, currentId]() {
        QString filePath = accessor->GetContext(currentId)->GetData<DocumentData>()->GetPackageAbsolutePath();
        invoker->Invoke(QEGlobal::SelectFile.ID, filePath);
    });

    menu.exec(tabBar->mapToGlobal(pos));
}