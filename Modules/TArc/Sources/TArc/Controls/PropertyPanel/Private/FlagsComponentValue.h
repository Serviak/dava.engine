#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
namespace TArc
{
class FlagsComponentValue : public BaseComponentValue
{
public:
    FlagsComponentValue() = default;

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override;

private:
    Any GetValueAny() const;
    void SetValueAny(const Any& newValue);

private:
    DAVA_VIRTUAL_REFLECTION(FlagsComponentValue, BaseComponentValue);
};
}
}
