/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "DAVAEngine.h"
#include "UI/UIEvent.h"
#include "UI/UIControlSystem.h"

#include "davaglwidget.h"

#include <QApplication>
#include <QMessageBox>
#include <QResizeEvent>
#include <QTimer>
#include <QElapsedTimer>
#include <QMoveEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QDateTime>
#include <QAbstractEventDispatcher>
#include <QDebug>

#if defined (__DAVAENGINE_MACOS__)
	#include "Platform/Qt/MacOS/QtLayerMacOS.h"
    #include "Platform/Qt/MacOS/CoreMacOSPlatformQt.h"
#elif defined (__DAVAENGINE_WIN32__)
	#include "Platform/Qt/Win32/QtLayerWin32.h"
	#include "Platform/Qt/Win32/CorePlatformWin32Qt.h"
#endif //#if defined (__DAVAENGINE_MACOS__)


DavaGLWidget::DavaGLWidget(QWidget *parent)
	: QWidget(parent)
	, maxFPS(60)
	, fps(0)
	, fpsCountTime(0)
	, fpsCount(0)
	, minFrameTimeMs(0)
{
	setAttribute(Qt::WA_OpaquePaintEvent, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_PaintOnScreen, true);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_NativeWindow, true);

#ifdef Q_OS_WIN
	//setWindowFlags(windowFlags() | Qt::MSWindowsOwnDC);	// TODO: investigate
#endif

    setFocusPolicy( Qt::StrongFocus );
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	// Init OS-specific layer
	{

#if defined (__DAVAENGINE_MACOS__)
		setMouseTracking(true);
		DAVA::QtLayerMacOS *qtLayer = (DAVA::QtLayerMacOS *) DAVA::QtLayer::Instance();
		qtLayer->InitializeGlWindow((void *)this->winId(), this->size().width(), this->size().height());
#elif defined (__DAVAENGINE_WIN32__)
		DAVA::QtLayerWin32 *qtLayer = (DAVA::QtLayerWin32 *) DAVA::QtLayer::Instance();
		HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
		qtLayer->SetWindow(hInstance, (HWND)this->winId(), this->size().width(), this->size().height());
		qtLayer->OnResume();
#else
		DVASSERT(false && "Wrong platform");
#endif //#if defined (__DAVAENGINE_MACOS__)

		DAVA::QtLayer::Instance()->SetDelegate(this);
		DAVA::QtLayer::Instance()->Resize(size().width(), size().height());

	}

	//EnableCustomPaintFlags(true);
	setAcceptDrops(true);

	// Setup FPS
	SetMaxFPS(maxFPS);

	QAbstractEventDispatcher::instance()->installNativeEventFilter(this);

	// start render
	QTimer::singleShot(0, this, SLOT(Render()));
}

DavaGLWidget::~DavaGLWidget()
{
	QAbstractEventDispatcher::instance()->removeNativeEventFilter(this);
}

QPaintEngine *DavaGLWidget::paintEngine() const
{
	return NULL;
}

bool DavaGLWidget::nativeEventFilter(const QByteArray& eventType, void* msg, long* result)
{
    Q_UNUSED(eventType);
    
#if defined(Q_OS_WIN)

	MSG *message = static_cast<MSG *>(msg);
	DAVA::CoreWin32PlatformQt *core = static_cast<DAVA::CoreWin32PlatformQt *>(DAVA::CoreWin32PlatformQt::Instance());
	DVASSERT(core);

	bool processMessage = false;

	if ( message->hwnd == reinterpret_cast<HWND>(winId()))
	{
		switch ( message->message )
		{
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
			core->SetFocused(true);
			break;

		default:
			break;
		}

		processMessage = true;
	}
	else
	{
		// Qt5 doesn't pass WM_CHAR, WM_KEY** messages to child QWidget::nativeEvent callback,
		// so handle this messages globaly
		switch ( message->message )
		{
		case WM_KEYUP:
		case WM_KEYDOWN:
		case WM_CHAR:
			processMessage = core->IsFocused();
			break;

		default:
			break;
		}
	}

	if (processMessage)
	{
		return core->WinEvent(message, result);
	}

#elif defined(Q_OS_MAC)
    Q_UNUSED(result);

    if (hasFocus())
    {
        DAVA::QtLayerMacOS *qtLayer = static_cast<DAVA::QtLayerMacOS *>(DAVA::QtLayer::Instance());
        qtLayer->HandleEvent(msg);
    }
#endif

	return false;
}

void DavaGLWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
}

void DavaGLWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
	DAVA::QtLayer::Instance()->Resize(e->size().width(), e->size().height());

	emit Resized(e->size().width(), e->size().height());
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

	DAVA::QtLayer::Instance()->LockKeyboardInput(true);
    qDebug() << "Focus In";
    QWidget::focusInEvent(e);
}

void DavaGLWidget::focusOutEvent(QFocusEvent *e)
{
	DAVA::InputSystem::Instance()->GetKeyboard()->ClearAllKeys();
	DAVA::QtLayer::Instance()->LockKeyboardInput(false);
    qDebug() << "Focus Out";
    QWidget::focusOutEvent(e);
}

void DavaGLWidget::dragEnterEvent(QDragEnterEvent *event)
{
	event->accept();
}

void DavaGLWidget::dragMoveEvent(QDragMoveEvent *event)
{
	DAVA::Vector<DAVA::UIEvent> touches;
	DAVA::Vector<DAVA::UIEvent> emptyTouches;

	DAVA::UIEvent newTouch;
	newTouch.tid = 1;
	newTouch.physPoint.x = event->pos().x();
	newTouch.physPoint.y = event->pos().y();
	newTouch.phase = DAVA::UIEvent::PHASE_MOVE;
	touches.push_back(newTouch);

	DAVA::UIControlSystem::Instance()->OnInput(DAVA::UIEvent::PHASE_MOVE, emptyTouches, touches);

	event->accept();
}

void DavaGLWidget::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();
	emit OnDrop(mimeData);
}

#if defined (Q_OS_MAC)
void DavaGLWidget::mouseMoveEvent(QMouseEvent *e)
{
    DAVA::QtLayerMacOS *qtLayer = static_cast<DAVA::QtLayerMacOS *>(DAVA::QtLayer::Instance());
    if(qtLayer)
    {
        //const QRect geometry = this->geometry();
        //qtLayer->MouseMoved(e->x() + geometry.x(), -e->y() - geometry.y());
        //fsdqtLayer->MouseMoved(e->x(), e->y());
    }

    QWidget::mouseMoveEvent(e);
}
#endif //#if defined (Q_OS_MAC)

void DavaGLWidget::SetMaxFPS(int fps)
{
	maxFPS = fps;

	if(0 != fps)
	{
		minFrameTimeMs = 1000 / fps;
	}

	DAVA::RenderManager::Instance()->SetFPS(maxFPS);
}

int DavaGLWidget::GetMaxFPS()
{
	return maxFPS;
}

int DavaGLWidget::GetFPS() const
{
	return fps;
}

void DavaGLWidget::Render()
{
	QElapsedTimer frameTimer;
	frameTimer.start();

	if(isEnabled() && DAVA::QtLayer::Instance()->IsDAVAEngineEnabled())
	{
		DAVA::QtLayer::Instance()->ProcessFrame();
	}

	if(QDateTime::currentMSecsSinceEpoch() >= fpsCountTime)
	{
		fps = fpsCount;
		fpsCount = 0;
		fpsCountTime = QDateTime::currentMSecsSinceEpoch() + 1000.0;
	}
	else
	{
		fpsCount++;
	}

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
//    DAVA::Logger::Info("[QUIT]");
    exit(0);
}

void DavaGLWidget::ShowAssertMessage(const char * message)
{
    QMessageBox::critical(this, "", message);
}
