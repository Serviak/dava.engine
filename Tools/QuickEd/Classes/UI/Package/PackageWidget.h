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


#ifndef __UI_EDITOR_UI_PACKAGE_WIDGET__
#define __UI_EDITOR_UI_PACKAGE_WIDGET__

#include <QWidget>
#include <QDockWidget>
#include <QPointer>
#include <QItemSelectionModel>
#include "UI/Package/FilteredPackageModel.h"
#include "UI/Package/PackageModel.h"
#include "DAVAEngine.h"
#include "ui_PackageWidget.h"

namespace Ui {
    class PackageWidget;
}

class ControlNode;
class SharedData;

class PackageWidget : public QDockWidget, public Ui::PackageWidget
{
    Q_OBJECT
public:
    explicit PackageWidget(QWidget *parent = 0);
    ~PackageWidget() = default;

public slots:
    void OnDocumentChanged(SharedData *context);
    void OnDataChanged(const QByteArray &role);
private:
    void LoadContext();
    void SaveContext();
private:
    void OnControlSelectedInEditor(const QList<ControlNode *> &node);

    void RefreshActions(const QModelIndexList &indexList);
    void RefreshAction(QAction *action, bool enabled, bool visible);
    void CollectSelectedNodes(DAVA::Vector<ControlNode*> &nodes);
    void CopyNodesToClipboard(const DAVA::Vector<ControlNode*> &nodes);
    void RemoveNodes(const DAVA::Vector<ControlNode*> &nodes);
    QList<QPersistentModelIndex> GetExpandedIndexes() const;
    
private slots:
    void OnRowsInserted(const QModelIndex &parent, int first, int last);
    void OnRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void OnSelectionChanged(const QItemSelection &proxySelected, const QItemSelection &proxyDeselected);
    void filterTextChanged(const QString &);
    void OnImport();
    void OnCopy();
    void OnPaste();
    void OnCut();
    void OnDelete();

private:
    SharedData *sharedData;
    QAction *importPackageAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *cutAction;
    QAction *delAction;

    QPointer<FilteredPackageModel> filteredPackageModel;
    QPointer<PackageModel> packageModel;
};

#endif // __UI_EDITOR_UI_PACKAGE_WIDGET__
