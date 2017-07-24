#pragma once

#include "EditorSystems/EditorSystemsManager.h"

#include "UI/Preview/ScaleComboBoxAdapter.h"
#include "UI/Preview/ScrollBarAdapter.h"

#include <TArc/DataProcessing/DataWrapper.h>

#include <Engine/Qt/IClientDelegate.h>
#include <Engine/Qt/RenderWidget.h>

#include <QFrame>
#include <QCursor>
#include <QPointer>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
class OperationInvoker;
class UI;
class DataContext;
}
}
class EditorSystemsManager;
class WidgetsData;

class ControlNode;
class PackageBaseNode;
class CursorInterpreter;
class AbstractProperty;

class FindInDocumentWidget;
class RulerWidget;
class RulerController;
class EditorCanvas;
class GuidesController;

class QGridLayout;
class QComboBox;
class QScrollBar;
class QWheelEvent;
class QNativeGestureEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QMenu;

class PreviewWidget : public QFrame, private DAVA::IClientDelegate
{
    Q_OBJECT
public:
    explicit PreviewWidget(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::OperationInvoker* invoker, DAVA::TArc::UI* ui, DAVA::RenderWidget* renderWidget, EditorSystemsManager* systemsManager);
    ~PreviewWidget();

    FindInDocumentWidget* GetFindInDocumentWidget();

    DAVA::Signal<DAVA::uint64> requestCloseTab;
    DAVA::Signal<ControlNode*> requestChangeTextInNode;

signals:
    void DeleteRequested();
    void ImportRequested();
    void CutRequested();
    void CopyRequested();
    void PasteRequested();
    void DuplicateRequested();

    void OpenPackageFile(QString path);
    void DropRequested(const QMimeData* data, Qt::DropAction action, PackageBaseNode* targetNode, DAVA::uint32 destIndex, const DAVA::Vector2* pos);

public slots:
    void OnEmulationModeChanged(bool emulationMode);
    void OnIncrementScale();
    void OnDecrementScale();
    void SetActualScale();

private slots:
    void OnResized(DAVA::uint32 width, DAVA::uint32 height);

    void OnTabBarContextMenuRequested(const QPoint& pos);

private:
    void InitUI();
    void ShowMenu(const QMouseEvent* mouseEvent);
    bool AddSelectionMenuSection(QMenu* parentMenu, const QPoint& pos);
    bool CanChangeTextInControl(const ControlNode* node) const;

    void InitFromSystemsManager(EditorSystemsManager* systemsManager);

    void InjectRenderWidget(DAVA::RenderWidget* renderWidget);

    void CreateActions();
    void OnMouseReleased(QMouseEvent* event) override;
    void OnMouseMove(QMouseEvent* event) override;
    void OnMouseDBClick(QMouseEvent* event) override;
    void OnDragEntered(QDragEnterEvent* event) override;
    void OnDragMoved(QDragMoveEvent* event) override;
    bool ProcessDragMoveEvent(QDropEvent* event);
    void OnDragLeaved(QDragLeaveEvent* event) override;
    void OnDrop(QDropEvent* event) override;
    void OnKeyPressed(QKeyEvent* event) override;

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::OperationInvoker* invoker = nullptr;
    DAVA::TArc::UI* ui = nullptr;

    DAVA::RenderWidget* renderWidget = nullptr;

    RulerController* rulerController = nullptr;

    QAction* selectAllAction = nullptr;
    QAction* focusNextChildAction = nullptr;
    QAction* focusPreviousChildAction = nullptr;

    EditorSystemsManager* systemsManager = nullptr;
    EditorCanvas* editorCanvas = nullptr;
    CursorInterpreter* cursorInterpreter = nullptr;

    //we can show model dialogs only when mouse released, so remember node to change text when mouse will be released
    ControlNode* nodeToChangeTextOnMouseRelease = nullptr;

    FindInDocumentWidget* findInDocumentWidget = nullptr;

    DAVA::TArc::DataWrapper centralWidgetDataWrapper;

    ScaleComboBoxAdapter scaleComboBoxData;
    ScrollBarAdapter hScrollBarData;
    ScrollBarAdapter vScrollBarData;
    //adapter to change scale by actions increment/decrement/normalize scale
    CanvasDataAdapter canvasDataAdapter;
};
