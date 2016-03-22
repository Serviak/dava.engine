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


#ifndef QTTOOLS_WIDGETHIGHLIGHTMODEL_H
#define QTTOOLS_WIDGETHIGHLIGHTMODEL_H

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QIdentityProxyModel>
#include <QSet>
#include <QWidget>
POP_QT_WARNING_SUPRESSOR

class WidgetHighlightModel
: public QIdentityProxyModel
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR

public:
    explicit WidgetHighlightModel(QObject* parent = nullptr);
    ~WidgetHighlightModel();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setWidgetList(const QSet<QWidget*>& widgetsToHighlight = QSet<QWidget*>());

private slots:
    void onWidgetDestroyed();

private:
    void invalidate();

    QSet<QWidget*> widgets;
};


#endif // QTTOOLS_WIDGETHIGHLIGHTMODEL_H
