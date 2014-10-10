#include "PropertiesTreeItemDelegate.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QPainter>
#include <QStylePainter>
#include <QApplication>
#include <QToolButton>
#include "QtControls/lineeditext.h"

#include "DAVAEngine.h"
#include "QtControls/Vector2DEdit.h"
#include "UIControls/BaseProperty.h"
#include "Utils/QtDavaConvertion.h"
#include "Vector2PropertyDelegate.h"
#include "EnumPropertyDelegate.h"
#include "PropertiesTreeModel.h"
#include "StringPropertyDelegate.h"
#include "FilePathPropertyDelegate.h"
#include "ColorPropertyDelegate.h"
#include "IntegerPropertyDelegate.h"
#include "FloatPropertyDelegate.h"

PropertiesTreeItemDelegate::PropertiesTreeItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    propertyItemDelegates[BaseProperty::TYPE_ENUM] = new EnumPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_VECTOR2] = new Vector2PropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_STRING] = new StringPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_WIDE_STRING] = new StringPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_FILEPATH] = new FilePathPropertyDelegate();
    variantTypeItemDelegates[DAVA::VariantType::TYPE_COLOR] = new ColorPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_INT32] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_INT64] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_UINT32] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_UINT64] = new IntegerPropertyDelegate(this);
    variantTypeItemDelegates[DAVA::VariantType::TYPE_FLOAT] = new FloatPropertyDelegate(this);
}

PropertiesTreeItemDelegate::~PropertiesTreeItemDelegate()
{
}

void PropertiesTreeItemDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

QSize PropertiesTreeItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *PropertiesTreeItemDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    AbstractPropertyDelegate *currentDelegate = GetCustomItemDelegateForIndex(index);
    if (currentDelegate)
    {
        QWidget *editorWidget = new QWidget(parent);
        editorWidget->setObjectName(QString::fromUtf8("editorWidget"));
        QWidget *editor = currentDelegate->createEditor(editorWidget, option, index);

        if (!editor)
        {
            DAVA::SafeDelete(editorWidget);
        }
        else
        {
            QHBoxLayout *horizontalLayout = new QHBoxLayout(editorWidget);
            horizontalLayout->setSpacing(1);
            horizontalLayout->setContentsMargins(0, 0, 0, 0);
            horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
            editorWidget->setLayout(horizontalLayout);

            editorWidget->setAutoFillBackground(true);
            editorWidget->setFocusProxy(editor);

            editorWidget->layout()->addWidget(editor);

            QList<QAction *> actions;
            currentDelegate->enumEditorActions(editorWidget, index, actions);

            foreach (QAction *action, actions)
            {
                QToolButton *toolButton = new QToolButton(editorWidget);
                toolButton->setDefaultAction(action);
                toolButton->setIconSize(QSize(15, 15));
                editorWidget->layout()->addWidget(toolButton);
            }
        }

        return editorWidget;
    }

    if (index.data(Qt::EditRole).type() == QVariant::Bool)
        return NULL;

    return QStyledItemDelegate::createEditor(parent, option, index);
}

void PropertiesTreeItemDelegate::setEditorData( QWidget *editor, const QModelIndex & index ) const
{
    AbstractPropertyDelegate *currentDelegate = GetCustomItemDelegateForIndex(index);
    if (currentDelegate)
    {
        return currentDelegate->setEditorData(editor, index);
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

void PropertiesTreeItemDelegate::setModelData(QWidget * editor, QAbstractItemModel *model, const QModelIndex & index) const
{
    AbstractPropertyDelegate *currentDelegate = GetCustomItemDelegateForIndex(index);
    if (currentDelegate)
    {
        currentDelegate->setModelData(editor, model, index);
        return;
    }

    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    if (lineEdit && !lineEdit->isModified())
        return;

    QStyledItemDelegate::setModelData(editor, model, index);
}

void PropertiesTreeItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

bool PropertiesTreeItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

AbstractPropertyDelegate * PropertiesTreeItemDelegate::GetCustomItemDelegateForIndex( const QModelIndex & index ) const
{
    BaseProperty *property = static_cast<BaseProperty *>(index.internalPointer());
    if (property)
    {
        QMap<BaseProperty::ePropertyType, AbstractPropertyDelegate *>::const_iterator prop_iter = propertyItemDelegates.find(property->GetType());
        if (prop_iter != propertyItemDelegates.end())
            return prop_iter.value();
    }

    QVariant editValue = index.data(Qt::EditRole);
    if (editValue.userType() == QMetaTypeId<DAVA::VariantType>::qt_metatype_id())
    {
        DAVA::VariantType variantType = editValue.value<DAVA::VariantType>();
        QMap<DAVA::VariantType::eVariantType, AbstractPropertyDelegate *>::const_iterator var_iter = variantTypeItemDelegates.find(variantType.GetType());
        if (var_iter != variantTypeItemDelegates.end())
            return var_iter.value();
    }
    else
    {
        QMap<QVariant::Type, AbstractPropertyDelegate *>::const_iterator iter = qvariantItemDelegates.find(editValue.type());
        if (iter != qvariantItemDelegates.end())
            return iter.value();
    }

    return NULL;
}

void PropertiesTreeItemDelegate::emitCommitData(QWidget * editor)
{
    emit commitData(editor);
}

void PropertiesTreeItemDelegate::emitCloseEditor(QWidget * editor, QAbstractItemDelegate::EndEditHint hint)
{
    emit closeEditor(editor, hint);
}


