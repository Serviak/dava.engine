#include "TArc/Controls/ScrollBar.h"

namespace DAVA
{
namespace TArc
{
ScrollBar::ScrollBar(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QScrollBar>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

ScrollBar::ScrollBar(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QScrollBar>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl();
}

void ScrollBar::SetupControl()
{
    connections.AddConnection(this, &QScrollBar::valueChanged, MakeFunction(this, &ScrollBar::OnValueChanged));
}

void ScrollBar::UpdateControl(const ControlDescriptor& changedFields)
{
    bool minChanged = changedFields.IsChanged(Fields::Minimum);
    bool maxChanged = changedFields.IsChanged(Fields::Maximum);

    //setMinimum will call setValue inside QScrollBar so disable valueChanged signal on sync
    connections.RemoveConnection(this, &QScrollBar::valueChanged);

    if (changedFields.IsChanged(Fields::Enabled))
    {
        this->setEnabled(this->template GetFieldValue<bool>(Fields::Enabled, true));
    }

    if (minChanged)
    {
        this->setMinimum(this->template GetFieldValue<int>(Fields::Minimum, 0));
    }

    if (maxChanged)
    {
        this->setMaximum(this->template GetFieldValue<int>(Fields::Maximum, 0));
    }

    if (changedFields.IsChanged(Fields::PageStep))
    {
        this->setPageStep(this->template GetFieldValue<int>(Fields::PageStep, 0));
    }

    if (minChanged || maxChanged || changedFields.IsChanged(Fields::Value))
    {
        this->setValue(this->template GetFieldValue<int>(Fields::Value, 0));
    }

    if (changedFields.IsChanged(Fields::Orientation))
    {
        Qt::Orientation orientation = this->template GetFieldValue<Qt::Orientation>(Fields::Orientation, Qt::Horizontal);
        this->setSizePolicy(orientation == Qt::Horizontal ? QSizePolicy::Expanding : QSizePolicy::Maximum,
                            orientation == Qt::Horizontal ? QSizePolicy::Maximum : QSizePolicy::Expanding);
        this->setOrientation(orientation);
    }

    if (changedFields.IsChanged(Fields::Visible))
    {
        this->setVisible(this->template GetFieldValue<bool>(Fields::Visible, true));
    }

    connections.AddConnection(this, &QScrollBar::valueChanged, MakeFunction(this, &ScrollBar::OnValueChanged));
}

void ScrollBar::OnValueChanged(int value)
{
    if (this->isEnabled() == false)
    {
        return;
    }

    wrapper.SetFieldValue(GetFieldName(Fields::Value), value);
}

} // namespace TArc
} // namespace DAVA