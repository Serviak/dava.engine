#ifndef __UI_EDITOR_UI_PACKAGE_WIDGET__
#define __UI_EDITOR_UI_PACKAGE_WIDGET__

#include <QWidget>
#include <QDockWidget>
#include <QItemSelectionModel>

//!! TODO: remove include DAVAEngine.h
#include "DAVAEngine.h"

namespace Ui {
    class PackageWidget;
}

class ControlNode;
class QSortFilterProxyModel;
class WidgetContext;

class PackageWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit PackageWidget(QWidget *parent = 0);
    virtual ~PackageWidget();

public slots:
    void OnContextChanged(WidgetContext *context);
    void OnDataChanged(const QByteArray &role);

    void OnControlSelectedInEditor(ControlNode *node);
    void OnAllControlsDeselectedInEditor();

private:
    void UpdateModel();
    void UpdateSelection();
    void UpdateExpanded();
    void UpdateFilterString();

    void SaveSelection();
    void SaveExpanded();
    void SaveFilterString();

    void RefreshActions(const QModelIndexList &indexList);
    void RefreshAction(QAction *action, bool enabled, bool visible);
    void CollectSelectedNodes(DAVA::Vector<ControlNode*> &nodes);
    void CopyNodesToClipboard(const DAVA::Vector<ControlNode*> &nodes);
    void RemoveNodes(const DAVA::Vector<ControlNode*> &nodes);
    QList<QPersistentModelIndex> GetExpandedIndexes() const;
    
private slots:
    void OnSelectionChanged(const QItemSelection &proxySelected, const QItemSelection &proxyDeselected);
    void filterTextChanged(const QString &);
    void OnCopy();
    void OnPaste();
    void OnCut();
    void OnDelete();

signals:
    void SelectionRootControlChanged(const QList<ControlNode*> &activatedRootControls, const QList<ControlNode*> &deactivatedRootControls);
    void SelectionControlChanged(const QList<ControlNode*> &activatedControls, const QList<ControlNode*> &deactivatedControls);

private:
    Ui::PackageWidget *ui;
    WidgetContext *widgetContext;
    QAction *importPackageAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *cutAction;
    QAction *delAction;

    QSortFilterProxyModel *proxyModel;
};

#endif // __UI_EDITOR_UI_PACKAGE_WIDGET__
