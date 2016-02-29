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


#ifndef QTTOOLS_SPYWIDGETINFO_H
#define QTTOOLS_SPYWIDGETINFO_H

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QObject>
#include <QPointer>
#include <QModelIndex>
POP_QT_WARNING_SUPRESSOR

class QWidget;
class QTimer;

class SpyWidget;
class WidgetModel;
class WidgetHighlightModel;


class SpyWidgetInfo
    : public QObject
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

public:
    explicit SpyWidgetInfo( QObject *parent = nullptr );
    ~SpyWidgetInfo();

    void trackWidget( QWidget *w );

    bool eventFilter( QObject *obj, QEvent *e ) override;

public slots:
    void show();
    void updateInformation();

private slots:
    void onChangeWidget( const QModelIndex& index );
    void onSelectWidget();

private:
    void selectWidget( QWidget *w );

    QPointer< SpyWidget > view;
    QPointer< QWidget > widget;
    QPointer< QTimer > updateTimer;
    QPointer< WidgetModel > widgetModel;
    QPointer< WidgetHighlightModel > widgetHighlightModel;
};


#endif // QTTOOLS_SPYWIDGETINFO_H
