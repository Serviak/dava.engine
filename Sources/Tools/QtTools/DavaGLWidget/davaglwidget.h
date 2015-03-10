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



#ifndef DAVAGLWIDGET_H
#define DAVAGLWIDGET_H

#include <QOpenGLWidget>
#include <QTimer>
#include <QMimeData>
#include <QWindow>
#include <QPointer>


#include "UI/UIEvent.h"
#include "Platform/Qt5/QtLayer.h"

class QOpenGLContext;
class QOpenGLPaintDevice;
class QExposeEvent;
class DavaGLWidget;


class OpenGLWindow
    : public QWindow
{
    friend class DavaGLWidget;

    Q_OBJECT
    
signals:
    void mousePressed();
    
public:
    OpenGLWindow();
    ~OpenGLWindow();
    
    void render();
    void renderNow();

signals:
    void Exposed();
    
protected:
    bool event(QEvent *event) override;
    void exposeEvent(QExposeEvent *event) override;
    
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    
    void mouseMoveEvent(QMouseEvent * event) override;
    void mousePressEvent(QMouseEvent * event) override;
    void mouseReleaseEvent(QMouseEvent * event) override;
    
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *) override;
    void handleDragMoveEvent(QDragMoveEvent * event);
    
    DAVA::UIEvent MapMouseEventToDAVA(const QMouseEvent *event) const;
    DAVA::UIEvent::eButtonID MapQtButtonToDAVA(const Qt::MouseButton button) const;
    
private:
    QOpenGLPaintDevice *paintDevice;
};


class FocusTracker
    : public QObject
{
    Q_OBJECT

public:
    explicit FocusTracker( DavaGLWidget *glWidget );
    ~FocusTracker();

    void OnClick();
    void OnEnter();
    void OnLeave();
    void OnFocusIn();
    void OnFocusOut();

private:
    QPointer< DavaGLWidget > glWidget;
    QPointer< QWindow > glWindow;
    QPointer< QWidget > prevWidget;
    QPointer< QWindow > prevWindow;

    bool isFocused;
    bool needToRestoreFocus;

private:
    static bool isEditor( QWidget *w );
};


class DavaGLWidget
    : public QWidget
{
    Q_OBJECT
    
public:
    explicit DavaGLWidget(QWidget *parent = nullptr);
    ~DavaGLWidget();

    OpenGLWindow *GetGLWindow();
    
signals:
    void Initialized();
    void Resized(int width, int height, int dpr);
	void OnDrop(const QMimeData *mimeData);

private slots:
    void OnWindowExposed();
    
private:
    void resizeEvent(QResizeEvent *) override;
    bool eventFilter( QObject * watched, QEvent * event ) override;

    void PerformSizeChange();
    
    bool isInitialized;
    int currentDPR;
    int currentWidth;
    int currentHeight;

    QPointer< OpenGLWindow > openGlWindow;
    QPointer< QWidget > container;
    QPointer< FocusTracker > focusTracker;
};



#endif // DAVAGLWIDGET_H
