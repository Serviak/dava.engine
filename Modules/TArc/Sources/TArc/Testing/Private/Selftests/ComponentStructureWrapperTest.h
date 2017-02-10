#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/PropertyPanel/Private/PropertyPanelMeta.h"
#include "TArc/Controls/PropertyPanel/StaticEditorDrawer.h"

#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Base/GlobalEnum.h>
#include <Base/Any.h>

namespace ComponentStructureWrapperTestDetail
{
enum DummyEnum
{
    First,
    Second,
    Third
};

struct ValueNode : DAVA::ReflectionBase
{
    int x;
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ValueNode)
    {
        DAVA::ReflectionRegistrator<ValueNode>::Begin()
        .Field("x", &ValueNode::x)[DAVA::M::Range(1, 20, 3), DAVA::M::EnumT<DummyEnum>()]
        .End();
    }
};

class DummyComponentValue : public DAVA::TArc::BaseComponentValue
{
public:
    int GetIntValue() const
    {
        return 0;
    }

    void SetIntValue(int v)
    {
    }

    void Add(const std::shared_ptr<DAVA::TArc::PropertyNode>& node)
    {
        AddPropertyNode(node);
    }

protected:
    DAVA::float32 y;
    DAVA::Any GetValue() const override
    {
        return DAVA::Any();
    }

    bool IsValidValueToSet(const DAVA::Any& /*newValue*/, const DAVA::Any& /*currentValue*/) const override
    {
        return true;
    }

    const DAVA::TArc::StaticEditorDrawer* GetStaticEditorDrawer() const override
    {
        return nullptr;
    }

    QWidget* AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option) override
    {
        return nullptr;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DummyComponentValue, DAVA::TArc::BaseComponentValue)
    {
        DAVA::ReflectionRegistrator<DummyComponentValue>::Begin(DAVA::TArc::CreateComponentStructureWrapper<DummyComponentValue>())
        .Field("value", &DummyComponentValue::GetIntValue, &DummyComponentValue::SetIntValue)[DAVA::M::ProxyMetaRequire()]
        .Field("notProxyField", &DummyComponentValue::y)[DAVA::M::ReadOnly()]
        .End();
    }
};
}

ENUM_DECLARE(ComponentStructureWrapperTestDetail::DummyEnum)
{
    ENUM_ADD(ComponentStructureWrapperTestDetail::First);
    ENUM_ADD(ComponentStructureWrapperTestDetail::Second);
    ENUM_ADD(ComponentStructureWrapperTestDetail::Third);
}

DAVA_TARC_TESTCLASS(ComponentStructureWrapperTest)
{
  void CheckValueField(const DAVA::Reflection& field)
  {
  using namespace ComponentStructureWrapperTestDetail;
TEST_VERIFY(field.HasMeta<DAVA::M::Range>());
TEST_VERIFY(field.HasMeta<DAVA::M::Enum>());

const DAVA::M::Range* rangeMeta = field.GetMeta<DAVA::M::Range>();
TEST_VERIFY(rangeMeta->minValue.Cast<int>() == 1);
TEST_VERIFY(rangeMeta->maxValue.Cast<int>() == 20);
TEST_VERIFY(rangeMeta->step.Cast<int>() == 3);

const DAVA::M::Enum* enumMeta = field.GetMeta<DAVA::M::Enum>();
TEST_VERIFY(enumMeta->GetEnumMap() == GlobalEnumMap<DummyEnum>::Instance());

TEST_VERIFY(field.HasMeta<DAVA::M::ProxyMetaRequire>() == false);
}

void CheckNoMetaField(const DAVA::Reflection& field)
{
    TEST_VERIFY(field.HasMeta<DAVA::M::Range>() == false);
    TEST_VERIFY(field.HasMeta<DAVA::M::Enum>() == false);
}

DAVA_TEST (ProxyMetaTest)
{
    using namespace ComponentStructureWrapperTestDetail;

    ValueNode node;
    DAVA::Reflection r = DAVA::Reflection::Create(&node);
    std::shared_ptr<DAVA::TArc::PropertyNode> propNode(new DAVA::TArc::PropertyNode());
    propNode->field.ref = r.GetField("x");
    propNode->field.key = DAVA::Any("x");
    propNode->propertyType = DAVA::TArc::PropertyNode::RealProperty;
    propNode->cachedValue = propNode->field.ref.GetValue();

    DummyComponentValue value;
    value.Add(propNode);

    DAVA::Reflection valueR = DAVA::Reflection::Create(&value);
    CheckValueField(valueR.GetField("value"));
    CheckNoMetaField(valueR.GetField("notProxyField"));

    {
        DAVA::Vector<DAVA::Reflection::Field> fields = valueR.GetFields();
        for (const DAVA::Reflection::Field& f : fields)
        {
            if (f.key.Cast<DAVA::String>() == "value")
                CheckValueField(f.ref);
            else if (f.key.Cast<DAVA::String>() == "notProxyField")
            {
                CheckNoMetaField(f.ref);
            }
        }
    }
}
}
;
