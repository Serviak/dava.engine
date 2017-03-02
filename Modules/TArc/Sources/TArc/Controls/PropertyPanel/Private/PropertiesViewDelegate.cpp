#include "TArc/Controls/PropertyPanel/Private/PropertiesViewDelegate.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include "Engine/PlatformApi.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QtEvents>
#include <QtGlobal>

namespace DAVA
{
namespace TArc
{
namespace PropertiesViewDelegateDetail
{
void FixFont(QFont& font)
{
    font.setFamily(font.family());
}

void InitStyleOptions(QStyleOptionViewItem& options)
{
    FixFont(options.font);
    options.fontMetrics = QFontMetrics(options.font);
}
}

PropertiesViewDelegate::PropertiesViewDelegate(QAbstractItemView* view_, ReflectedPropertyModel* model_, QObject* parent)
    : QAbstractItemDelegate(parent)
    , model(model_)
    , view(view_)
{
}

void PropertiesViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    PropertiesViewDelegateDetail::InitStyleOptions(opt);

    BaseComponentValue* valueComponent = GetComponentValue(index);
    DVASSERT(valueComponent != nullptr);
    if (index.column() == 0)
    {
        QStyle* style = option.widget->style();
        opt.text = valueComponent->GetPropertyName();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
    }
    else
    {
        valueComponent->Draw(view->viewport(), painter, opt);
    }
}

QSize PropertiesViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    QStyle* style = opt.widget->style();
    BaseComponentValue* valueComponent = GetComponentValue(index);
    DVASSERT(valueComponent != nullptr);

    QSize sizeHint(opt.rect.size());
    if (index.column() == 0)
    {
        opt.text = valueComponent->GetPropertyName();
        sizeHint = style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), opt.widget);
    }
    else
    {
        QWidget* viewport = view->viewport();
        if (valueComponent->HasHeightForWidth(viewport))
        {
            int height = valueComponent->GetHeightForWidth(viewport, opt.rect.width());
            heightForWidthItems[QPersistentModelIndex(index)] = height;
            sizeHint.setHeight(height);
        }
        else
        {
            sizeHint.setHeight(valueComponent->GetHeight(viewport));
        }
    }

    return sizeHint;
}

QWidget* PropertiesViewDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (index.isValid() == false)
    {
        return nullptr;
    }

    BaseComponentValue* valueComponent = GetComponentValue(index);
    return valueComponent->AcquireEditorWidget(parent, option);
}

void PropertiesViewDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
    if (index.isValid() == false)
    {
        return;
    }

    BaseComponentValue* valueComponent = GetComponentValue(index);
    return valueComponent->ReleaseEditorWidget(editor);
}

void PropertiesViewDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    model->SyncWrapper();
}

void PropertiesViewDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
}

void PropertiesViewDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (editor == nullptr)
    {
        return;
    }

    BaseComponentValue* valueComponent = GetComponentValue(index);
    valueComponent->UpdateGeometry(view->viewport(), option);
}

bool PropertiesViewDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (index.column() == 0)
    {
        return false;
    }

    BaseComponentValue* valueComponent = GetComponentValue(index);
    return valueComponent->EditorEvent(view->viewport(), event, option);
}

bool PropertiesViewDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    return false;
}

BaseComponentValue* PropertiesViewDelegate::GetComponentValue(const QModelIndex& index) const
{
    DVASSERT(index.isValid());
    return model->GetComponentValue(index);
}

void PropertiesViewDelegate::UpdateSizeHints(int section, int newWidth)
{
    {
        auto iter = heightForWidthItems.begin();
        while (iter != heightForWidthItems.end())
        {
            if (!iter.key().isValid())
            {
                iter = heightForWidthItems.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    QList<QModelIndex> sizeHintChangedIndexes;
    QWidget* viewportWidgte = view->viewport();
    for (auto iter = heightForWidthItems.begin(); iter != heightForWidthItems.end(); ++iter)
    {
        QPersistentModelIndex index = iter.key();
        BaseComponentValue* value = GetComponentValue(index);
        DVASSERT(value->HasHeightForWidth(viewportWidgte));
        int heightForWidth = value->GetHeightForWidth(viewportWidgte, newWidth);
        if (iter.value() != heightForWidth)
        {
            iter.value() = heightForWidth;
            sizeHintChangedIndexes.push_back(index);
        }
    }

    foreach (const QModelIndex& index, sizeHintChangedIndexes)
    {
        emit sizeHintChanged(index);
    }
}

} // namespace TArc
} // namespace DAVA
