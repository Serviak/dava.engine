#include "DAVAEngine.h"

#include "davaglwidget.h"
#include "ui_davaglwidget.h"

#include <QApplication>
#include <QResizeEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <QMoveEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>

#if defined (__DAVAENGINE_MACOS__)
	#include "Platform/Qt/MacOS/QtLayerMacOS.h"
#elif defined (__DAVAENGINE_WIN32__)
	#include "Platform/Qt/Win32/QtLayerWin32.h"
	#include "Platform/Qt/Win32/CorePlatformWin32.h"
#endif //#if defined (__DAVAENGINE_MACOS__)

DavaGLWidget::DavaGLWidget(QWidget *parent)
	: QWidget(parent)
    , ui(new Ui::DavaGLWidget)
	, maxFPS(60)
	, minFrameTimeMs(0)
{
	ui->setupUi(this);

	// Widget will try to expand to maximum available size
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	// Init OS-specific layer
	{

#if defined (__DAVAENGINE_MACOS__)
		setMouseTracking(true);
		DAVA::QtLayerMacOS *qtLayer = new DAVA::QtLayerMacOS();
		qtLayer->InitializeGlWindow((void *)this->winId(), this->size().width(), this->size().height());
#elif defined (__DAVAENGINE_WIN32__)
		DAVA::QtLayerWin32 *qtLayer = new DAVA::QtLayerWin32();
		HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
		qtLayer->SetWindow(hInstance, this->winId(), this->size().width(), this->size().height());
		qtLayer->OnResume();
#else
		DVASSERT(false && "Wrong platform");
#endif //#if defined (__DAVAENGINE_MACOS__)

		DAVA::QtLayer::Instance()->SetDelegate(this);
		DAVA::QtLayer::Instance()->Resize(size().width(), size().height());

	}

	// Disable Widget blinking
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_PaintOnScreen, true);

	// Setup FPS
	SetMaxFPS(maxFPS);

	// start render in 1 ms
	QTimer::singleShot(1, this, SLOT(Render()));
}

DavaGLWidget::~DavaGLWidget()
{
    DAVA::QtLayer::Instance()->Release();

    delete ui;
}

QPaintEngine *DavaGLWidget::paintEngine() const
{
	return NULL;
}

void DavaGLWidget::paintEvent(QPaintEvent *)
{
	// We have custom rendering (by timer), so here nothing to do
	return;
}

void DavaGLWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

	QPoint newPosition = mapTo(parentWidget(), QPoint(0, 0));

	DAVA::QtLayer::Instance()->Resize(e->size().width(), e->size().height());
	DAVA::QtLayer::Instance()->Move(newPosition.x(), newPosition.y());
}

void DavaGLWidget::moveEvent(QMoveEvent *e)
{
	QWidget::moveEvent(e);

    QPoint newPosition = mapTo(parentWidget(), QPoint(0, 0));

	DAVA::QtLayer::Instance()->Move(newPosition.x(), newPosition.y());
}

void DavaGLWidget::showEvent(QShowEvent *e)
{
	QWidget::showEvent(e);

	DAVA::QtLayer::Instance()->OnResume();
}

void DavaGLWidget::hideEvent(QHideEvent *e)
{
	QWidget::hideEvent(e);

	DAVA::QtLayer::Instance()->OnSuspend();
}

void DavaGLWidget::focusInEvent(QFocusEvent *e)
{
	QWidget::focusInEvent(e);

	DAVA::QtLayer::Instance()->LockKeyboardInput(true);
}

void DavaGLWidget::focusOutEvent(QFocusEvent *e)
{
	QWidget::focusOutEvent(e);

	DAVA::QtLayer::Instance()->LockKeyboardInput(false);
}

#if defined(Q_WS_WIN)
bool DavaGLWidget::winEvent(MSG *message, long *result)
{
	DAVA::CoreWin32Platform *core = dynamic_cast<DAVA::CoreWin32Platform *>(DAVA::CoreWin32Platform::Instance());
	if (NULL != core)
	{
		return core->WinEvent(message, result);
	}

	return false;
}
#endif //#if defined(Q_WS_WIN)

#if defined (Q_WS_MAC)
void DavaGLWidget::mouseMoveEvent(QMouseEvent *e)
{
    DAVA::QtLayerMacOS *qtLayer = dynamic_cast<DAVA::QtLayerMacOS *>(DAVA::QtLayer::Instance());
    if(qtLayer)
    {
        const QRect geometry = this->geometry();
        qtLayer->MouseMoved(e->x() + geometry.x(), -e->y() - geometry.y());
    }

    QWidget::mouseMoveEvent(e);
}
#endif //#if defined (Q_WS_MAC)

void DavaGLWidget::SetMaxFPS(int fps)
{
	maxFPS = fps;

	if(0 != fps)
	{
		minFrameTimeMs = 1000 / fps;
	}

	DAVA::RenderManager::Instance()->SetFPS(maxFPS);
}

void DavaGLWidget::Render()
{
	QElapsedTimer frameTimer;
	frameTimer.start();

	DAVA::QtLayer::Instance()->ProcessFrame();

	qint64 waitUntilNextFrameMs = (qint64) minFrameTimeMs - frameTimer.elapsed();

	if(waitUntilNextFrameMs <= 0)
	{
		// our render is too slow to reach maxFPS,
		// so we can wait a minimum time
		waitUntilNextFrameMs = 1;
	}

	QTimer::singleShot(waitUntilNextFrameMs, this, SLOT(Render()));
}

void DavaGLWidget::Quit()
{
    DAVA::Logger::Info("[QUIT]");
    exit(0);
}