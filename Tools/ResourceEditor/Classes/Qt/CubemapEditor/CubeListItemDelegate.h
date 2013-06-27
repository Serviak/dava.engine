#ifndef __CUBE_LIST_ITEM_DELEGATE_H__
#define __CUBE_LIST_ITEM_DELEGATE_H__

#include <QPainter>
#include <QAbstractItemDelegate>

#define CUBELIST_DELEGATE_ITEMFULLPATH (Qt::UserRole)
#define CUBELIST_DELEGATE_ITEMFILENAME (Qt::UserRole + 1)

class CubeListItemDelegate : public QAbstractItemDelegate
{
protected:
	
	int currentPage;
	
public:
	
	CubeListItemDelegate(QObject *parent = 0);
	virtual ~CubeListItemDelegate();
	
	void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
	QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
	
	void SetNeedsRepaint();
	void SetCurrentPage(int newPage);
};

#endif /* defined(__CUBE_LIST_ITEM_DELEGATE_H__) */
