#ifndef __PROPERTIESTREEITEMDELEGATE_H__
#define __PROPERTIESTREEITEMDELEGATE_H__

#include <QWidget>
#include <QVector2D>
#include <QLineEdit>
#include <QStyledItemDelegate>
#include "UIControls/BaseProperty.h"
#include "FileSystem/VariantType.h"
class AbstractPropertyDelegate;
class QToolButton;

class PropertiesTreeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PropertiesTreeItemDelegate(QObject *parent = NULL);
    ~PropertiesTreeItemDelegate();

    virtual void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    virtual void setEditorData ( QWidget * editor, const QModelIndex & index ) const override;
    virtual void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const override;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    AbstractPropertyDelegate * GetCustomItemDelegateForIndex(const QModelIndex & index) const;

    void emitCommitData(QWidget * editor);
    void emitCloseEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint);

private:
    QMap<QVariant::Type, AbstractPropertyDelegate *> qvariantItemDelegates;
    QMap<BaseProperty::ePropertyType, AbstractPropertyDelegate *> propertyItemDelegates;
    QMap<DAVA::VariantType::eVariantType, AbstractPropertyDelegate *> variantTypeItemDelegates;
};
#endif // __PROPERTIESTREEITEMDELEGATE_H__
