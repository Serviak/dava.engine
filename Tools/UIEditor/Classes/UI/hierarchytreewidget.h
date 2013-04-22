#ifndef HIERARCHYTREEWIDGET_H
#define HIERARCHYTREEWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include "HierarchyTreeControlNode.h"
#include "HierarchyTreeController.h"


namespace Ui {
class HierarchyTreeWidget;
}

class HierarchyTreeWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit HierarchyTreeWidget(QWidget *parent = 0);
    ~HierarchyTreeWidget();
	
private:
	typedef Set<HierarchyTreeNode::HIERARCHYTREENODEID> EXPANDEDITEMS;
	void AddControlItem(QTreeWidgetItem* parent, const HierarchyTreeNode::HIERARCHYTREENODESLIST& items);

	// Restore the selected/expanded tree item state.
	void RestoreTreeItemSelectedStateRecursive(QTreeWidgetItem* parentItem, const EXPANDEDITEMS& selectedItems);
	void RestoreTreeItemExpandedStateRecursive(QTreeWidgetItem* parentItem, const EXPANDEDITEMS& expandedItems);
signals:
	void CreateNewScreen();
	void CreateNewAggregator();

	void ImportScreenOrAggregator();

protected slots:
	void OnTreeUpdated();
	
private slots:
    void on_treeWidget_itemSelectionChanged();
	void OnSelectedControlNodesChanged(const HierarchyTreeController::SELECTEDCONTROLNODES &);
	void OnShowCustomMenu(const QPoint& pos);
	void OnDeleteControlAction();
	void OnCreateScreenAction();
	void OnCreateAggregatorAction();
	void OnCopyAction();
	void OnPasteAction();
	void OnImportScreenOrAggregatorAction();

private:
	typedef Map<int, QTreeWidgetItem*> TREEITEMS;
	TREEITEMS GetAllItems();
	void GetChildItems(const QTreeWidgetItem* parent, Map<int, QTreeWidgetItem*> &items);
	void ResetSelection();

	// Apply the icon, font color etc to the tree item.
	void Decorate(QTreeWidgetItem* item, UIControl* uiControl);
	bool IsDeleteNodeAllowed(HierarchyTreeControlNode* selectedControlNode);

private:
	bool internalSelectionChanged;
    Ui::HierarchyTreeWidget *ui;
};

#endif // HIERARCHYTREEWIDGET_H
