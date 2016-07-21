#include "DAVAEngine.h"
#include "Platform/Qt5/QtLayer.h"

#include "DavaRenderer.h"
#include "Davaglwidget.h"

#include "ControlMapper.h"

#include <QKeyEvent>
#include <QScreen>
#include <QTimer>
#include <QBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QAction>
#include <QVariant>
#include <QOpenGLContext>

namespace
{
const QSize cMinSize = QSize(180, 180);
}

DavaGLView::DavaGLView()
    : QQuickWindow()
    , controlMapper(new ControlMapper(this))
{
    setSurfaceType(QWindow::OpenGLSurface);

    setKeyboardGrabEnabled(true);
    setMouseGrabEnabled(true);

    setMinimumSize(cMinSize);
}

bool DavaGLView::event(QEvent* event)
{
    switch (event->type())
    {
    // Drag-n-drop
    case QEvent::DragEnter:
    {
        auto e = static_cast<QDragEnterEvent*>(event);
        e->setDropAction(Qt::LinkAction);
        e->accept();
    }
    break;
    case QEvent::DragMove:
    {
        auto e = static_cast<QDragMoveEvent*>(event);
        handleDragMoveEvent(e);
        e->setDropAction(Qt::LinkAction);
        e->accept();
        return QWindow::event(event);
    }
    break;
    case QEvent::DragLeave:
        break;
    case QEvent::Drop:
    {
        auto e = static_cast<QDropEvent*>(event);
        emit OnDrop(e->mimeData());
        e->setDropAction(Qt::LinkAction);
        e->accept();
    }
    break;

    // Focus
    case QEvent::FocusOut:
        controlMapper->releaseKeyboard();
        break;

    default:
        break;
    }

    return QQuickWindow::event(event);
}

void DavaGLView::keyPressEvent(QKeyEvent* e)
{
    controlMapper->keyPressEvent(e);
}

void DavaGLView::keyReleaseEvent(QKeyEvent* e)
{
    controlMapper->keyReleaseEvent(e);
}

void DavaGLView::mouseMoveEvent(QMouseEvent* e)
{
    controlMapper->mouseMoveEvent(e);
}

void DavaGLView::mousePressEvent(QMouseEvent* e)
{
    requestActivate();
    controlMapper->mousePressEvent(e);
}

void DavaGLView::mouseReleaseEvent(QMouseEvent* e)
{
    controlMapper->mouseReleaseEvent(e);
}

void DavaGLView::mouseDoubleClickEvent(QMouseEvent* e)
{
    controlMapper->mouseDoubleClickEvent(e);
}

void DavaGLView::wheelEvent(QWheelEvent* e)
{
    if (e->phase() != Qt::ScrollUpdate)
    {
        return;
    }

    controlMapper->wheelEvent(e);
    if (e->orientation() == Qt::Vertical)
    {
        emit mouseScrolled(e->angleDelta().y());
    }
}

void DavaGLView::handleDragMoveEvent(QDragMoveEvent* e)
{
    controlMapper->dragMoveEvent(e);
}

///=======================
DavaGLWidget::DavaGLWidget(QWidget* parent)
    : QWidget(parent)
{
//configure Qt Scene Graph to single thread mode
#ifdef Q_OS_WIN
    _putenv_s("QSG_RENDER_LOOP", "basic");
#else
    setenv("QSG_RENDER_LOOP", "basic", 1);
#endif

    setAcceptDrops(true);
    setMouseTracking(true);

    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMinimumSize(cMinSize);

    davaGLView = new DavaGLView();

    connect(qApp, &QApplication::focusWindowChanged, [this](QWindow* now) //fix bug with actions focus scope
            {
                bool isActive = (now == davaGLView);
                for (auto& action : actions())
                {
                    action->setEnabled(isActive);
                }
            });

    davaGLView->setClearBeforeRendering(false);
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, davaGLView, &DavaGLView::update);
    timer->start(16); //62.5 fps :)

    connect(davaGLView, &QWindow::screenChanged, this, &DavaGLWidget::OnResize);
    connect(davaGLView, &QWindow::screenChanged, this, &DavaGLWidget::ScreenChanged);
    connect(davaGLView, &QQuickWindow::beforeRendering, this, &DavaGLWidget::OnPaint, Qt::DirectConnection);
    connect(davaGLView, &QQuickWindow::sceneGraphInvalidated, this, &DavaGLWidget::OnCleanup);
    connect(davaGLView, &DavaGLView::mouseScrolled, this, &DavaGLWidget::mouseScrolled);
    connect(davaGLView, &DavaGLView::OnDrop, this, &DavaGLWidget::OnDrop);
    auto layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    layout->setMargin(0);
    setLayout(layout);

    QWidget* container = createWindowContainer(davaGLView);
    container->setAcceptDrops(true);
    container->setMouseTracking(true);
    container->setFocusPolicy(Qt::NoFocus);

    layout->addWidget(container);

#if defined(Q_OS_MAC)
    DAVA::Core::Instance()->SetNativeView(reinterpret_cast<void*>(davaGLView->winId()));
#endif //Q_OS_MAC
}

void DavaGLWidget::MakeInvisible()
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::Tool); // Remove border
    setAttribute(Qt::WA_TransparentForMouseEvents); // Rethrow mouse events
    setAttribute(Qt::WA_ShowWithoutActivating); // Do not get focus
    setWindowOpacity(0.0);
    setFixedSize(1, 1);
    setEnabled(false);
    move(0, 0);
}

QQuickWindow* DavaGLWidget::GetGLView()
{
    return davaGLView;
}

QCursor DavaGLWidget::GetCursor() const
{
    return davaGLView->cursor();
}

void DavaGLWidget::SetCursor(const QCursor& cursor)
{
    davaGLView->setCursor(cursor);
}

void DavaGLWidget::UnsetCursor()
{
    davaGLView->unsetCursor();
}

void DavaGLWidget::OnResize()
{
    if (nullptr != renderer)
    {
        auto dpr = davaGLView->devicePixelRatio();
        DAVA::QtLayer::Instance()->Resize(width(), height(), dpr);
        emit Resized(width(), height());
    }
}

namespace DAVAGLWidget_namespace
{
//there is a bug in Qt: https://bugreports.qt.io/browse/QTBUG-50465
void Kostil_ForceUpdateCurrentScreen(DavaGLWidget* davaGLWidget)
{
    auto desktop = qApp->desktop();
    int screenNumber = desktop->screenNumber(davaGLWidget);
    DVASSERT(screenNumber >= 0 && screenNumber < qApp->screens().size());

    QWindow* parent = davaGLWidget->GetGLView();
    while (parent->parent() != nullptr)
    {
        parent = parent->parent();
    }
    parent->setScreen(qApp->screens().at(screenNumber));
}
} //unnamed namespace

void DavaGLWidget::OnPaint()
{
    QVariant nativeHandle = davaGLView->openglContext()->nativeHandle();
    if (!nativeHandle.isValid())
    {
        DAVA::Logger::Error("GL context is not valid!");
        qApp->quit();
    }
    else
    {
        if (renderer == nullptr)
        {
            DAVAGLWidget_namespace::Kostil_ForceUpdateCurrentScreen(this);

            renderer = new DavaRenderer(davaGLView, davaGLView->openglContext());
            emit Initialized();
            OnResize();
        }

        renderer->paint();
        davaGLView->resetOpenGLState();
    }
}

void DavaGLWidget::resizeEvent(QResizeEvent*)
{
    if (nullptr != renderer)
    {
        OnResize();
    }
}

void DavaGLWidget::OnCleanup()
{
    delete renderer;
}
