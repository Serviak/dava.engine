#include "FloatPropertyDelegate.h"
#include "FileSystem/VariantType.h"
#include "PropertiesModel.h"
#include "PropertiesTreeItemDelegate.h"
#include "Utils/QtDavaConvertion.h"
#include <QLineEdit>
#include <QValidator>
#include <QLayout>


FloatPropertyDelegate::FloatPropertyDelegate( PropertiesTreeItemDelegate *delegate )
    : BasePropertyDelegate(delegate)
{

}

FloatPropertyDelegate::~FloatPropertyDelegate()
{

}

QWidget * FloatPropertyDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
    connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(OnValueChanged()));
    lineEdit->setValidator(new QRegExpValidator(QRegExp("\\s*-?\\d*[,\\.]?\\d*\\s*")));

    return lineEdit;
}

void FloatPropertyDelegate::setEditorData( QWidget * rawEditor, const QModelIndex & index ) const 
{
    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    DAVA::VariantType variant = index.data(Qt::EditRole).value<DAVA::VariantType>();
    editor->blockSignals(true);
    editor->setText(QString("%1").arg(variant.AsFloat()));
    editor->blockSignals(false);

    BasePropertyDelegate::SetValueModified(editor, false);
}

bool FloatPropertyDelegate::setModelData( QWidget * rawEditor, QAbstractItemModel * model, const QModelIndex & index ) const 
{
    if (BasePropertyDelegate::setModelData(rawEditor, model, index))
        return true;

    QLineEdit *editor = rawEditor->findChild<QLineEdit*>("lineEdit");

    QVariant variant;
    variant.setValue<DAVA::VariantType>(DAVA::VariantType(editor->text().toFloat()));

    return model->setData(index, variant, Qt::EditRole);
}

void FloatPropertyDelegate::OnValueChanged()
{
    QWidget *lineEdit = qobject_cast<QWidget *>(sender());
    if (!lineEdit)
        return;

    QWidget *editor = lineEdit->parentWidget();
    if (!editor)
        return;

    BasePropertyDelegate::SetValueModified(editor, true);
    itemDelegate->emitCommitData(editor);
}
