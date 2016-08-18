#include "GLView.h"
#include "QtTools/DavaGLWidget/DavaGLWidget.h"

GLView::GLView()
    : glWidget(new DavaGLWidget())
    , layoutHint("default")
{
    glWidget = new DavaGLWidget();
}

GLView::~GLView()
{
    delete glWidget;
}

const char* GLView::id() const
{
    return "glWidget";
}

const char* GLView::title() const
{
    return "title";
}

const char* GLView::windowId() const
{
    return "";
}

const wgt::LayoutHint& GLView::hint() const
{
    return layoutHint;
};

void GLView::update()
{
    //we update this widget 60 times per second manually
}

void GLView::focusInEvent()
{
}

void GLView::focusOutEvent()
{
}

void GLView::registerListener(wgt::IViewEventListener* listener)
{
}

void GLView::deregisterListener(wgt::IViewEventListener* listener)
{
}

QWidget* GLView::releaseView()
{
    return glWidget;
};

void GLView::retainView()
{
}

QWidget* GLView::view() const
{
    return glWidget;
};