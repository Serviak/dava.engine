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


#ifndef QTTOOLS_WIDGETITEM_H
#define QTTOOLS_WIDGETITEM_H

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QObject>
#include <QPointer>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QList>
#include <QWidgetList>
POP_QT_WARNING_SUPRESSOR

class QWidget;
class WidgetModel;

class WidgetItem
: public QObject
{
    friend class QSharedPointer<WidgetItem>;
    friend class WidgetModel;

    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

private:
    using ItemList = QList<QSharedPointer<WidgetItem>>;

public:
    ~WidgetItem();

    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    explicit WidgetItem(QWidget* w);
    void rebuildChildren();
    void onChildAdd(QWidget* w);
    void onChildRemove(QWidget* w);

    QPointer<QWidget> widget;
    QSharedPointer<WidgetItem> parentItem;
    QWeakPointer<WidgetItem> self;
    QPointer<WidgetModel> model;
    ItemList children;

public:
    static QSharedPointer<WidgetItem> create(QWidget* w);
};


#endif // QTTOOLS_WIDGETITEM_H
