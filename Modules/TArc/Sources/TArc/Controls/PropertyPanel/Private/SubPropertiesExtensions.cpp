#include "TArc/Controls/PropertyPanel/Private/SubPropertiesExtensions.h"
#include "TArc/Controls/PropertyPanel/Private/kDComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/MultiIntSpinBox.h"
#include "TArc/Controls/PropertyPanel/Private/MultiDoubleSpinBox.h"
#include "TArc/Controls/PropertyPanel/Private/TextComponentValue.h"
#include "TArc/Controls/ColorPicker/ColorPickerButton.h"

#include <QtTools/Utils/Utils.h>

#include <Math/Vector.h>
#include <Math/Rect.h>
#include <Math/AABBox3.h>
#include <Math/Color.h>
#include <Base/BaseTypes.h>
#include <Utils/StringFormat.h>

namespace DAVA
{
namespace TArc
{
namespace SubPropertiesExtensionsDetail
{
UnorderedSet<const Type*> subPropertyTypes;

void InitSubPropertyTypes()
{
    if (subPropertyTypes.empty())
    {
        subPropertyTypes.insert(Type::Instance<Vector2>());
        subPropertyTypes.insert(Type::Instance<Vector3>());
        subPropertyTypes.insert(Type::Instance<Vector4>());
        subPropertyTypes.insert(Type::Instance<Rect>());
        subPropertyTypes.insert(Type::Instance<AABBox3>());
        subPropertyTypes.insert(Type::Instance<Color>());
    }
}

FastName colorR = FastName("R");
FastName colorG = FastName("G");
FastName colorB = FastName("B");
FastName colorA = FastName("A");

void ReduceZeros(String& value)
{
    int32 zerosCount = 0;
    for (auto iter = value.rbegin(); iter != value.rend(); ++iter)
    {
        if ((*iter) != '0')
        {
            break;
        }
        ++zerosCount;
    }

    value.reserve(value.size() - zerosCount);
}

class ColorFieldAccessor : public IFieldAccessor
{
public:
    String GetFieldValue(const Any& v) const override
    {
        if (v.CanCast<Color>() == false)
        {
            return v.Cast<String>();
        }
        Color c = v.Cast<Color>();
        String result = Format("[ %.3f; %.3f; %.3f; %.3f]", c.r, c.g, c.b, c.a);
        ReduceZeros(result);
        return result;
    }

    Any CreateNewValue(const String& newFieldValue, const Any& propertyValue, M::ValidationResult& result) const override
    {
        return Parse(newFieldValue, result);
    }

    Any Parse(const String& strValue, M::ValidationResult& result) const override
    {
        if (strValue.empty())
        {
            result.state = M::ValidationResult::eState::Valid;
            return Color();
        }

        Vector<float32> parseResult = ParseFloatList(strValue);
        size_t componentCount = parseResult.size();
        if (componentCount > 4)
        {
            result.state = Metas::ValidationResult::eState::Invalid;
            result.message = "Incorrect color format. Color format [ r; g; b; a]";
            return Any();
        }

        result.state = M::ValidationResult::eState::Valid;
        Color c;
        if (componentCount > 0)
            c.r = parseResult[0];
        if (componentCount > 1)
            c.g = parseResult[1];
        if (componentCount > 2)
            c.b = parseResult[2];
        if (componentCount == 4)
            c.a = parseResult[3];
        return c;
    }
};

class ColorSubFieldAccessor : public IFieldAccessor
{
public:
    ColorSubFieldAccessor(int32 fieldIndex_, const QString& propertyName_)
        : fieldIndex(fieldIndex_)
        , propertyName(propertyName_)
    {
        DVASSERT(fieldIndex != -1);
    }

    String GetFieldValue(const Any& v) const override
    {
        if (v.CanCast<Color>() == false)
        {
            return v.Cast<String>();
        }

        Color c = v.Cast<Color>();
        String result = Format("%.3f", c.color[fieldIndex]);
        ReduceZeros(result);
        return result;
    }

    Any CreateNewValue(const String& newFieldValue, const Any& propertyValue, M::ValidationResult& result) const override
    {
        DVASSERT(propertyValue.CanCast<Color>());
        Any channelValue = Parse(newFieldValue, result);
        if (result.state == M::ValidationResult::eState::Invalid)
        {
            return result;
        }

        result.state = M::ValidationResult::eState::Valid;
        Color color = propertyValue.Cast<Color>();
        color.color[fieldIndex] = channelValue.Get<float32>();
        return color;
    }

    Any Parse(const String& strValue, M::ValidationResult& result) const override
    {
        Vector<float32> parseResult;
        if (strValue.empty())
        {
            parseResult.push_back(0.0f);
        }
        else
        {
            parseResult = ParseFloatList(strValue);
            if (parseResult.size() != 1)
            {
                result.state = M::ValidationResult::eState::Invalid;
                result.message = "Incorrect color channel format. Value should be float";
                return Any();
            }
        }

        result.state = M::ValidationResult::eState::Valid;
        return parseResult[0];
    }

    bool OverridePropertyName(QString& name) const override
    {
        name = propertyName;
        return true;
    }

private:
    int32 fieldIndex = -1;
    const QString propertyName;
};

class ColorComponentValue : public TextComponentValue
{
public:
    ColorComponentValue()
        : TextComponentValue(std::make_unique<ColorFieldAccessor>())
    {
    }

protected:
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override
    {
        Widget* w = new Widget(parent);
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setMargin(0);
        layout->setSpacing(1);
        w->SetLayout(layout);

        ColorPickerButton::Params params(GetAccessor(), GetUI(), GetWindowKey());
        params.fields[ColorPickerButton::Fields::Color] = "color";
        params.fields[ColorPickerButton::Fields::IsReadOnly] = readOnlyFieldName;
        w->AddControl(new ColorPickerButton(params, wrappersProcessor, model, w->ToWidgetCast()));
        w->AddControl(TextComponentValue::CreateEditorWidget(w->ToWidgetCast(), model, wrappersProcessor));

        return w;
    }

    Any GetColor() const
    {
        return GetValue();
    }

    void SetColor(const Any& color)
    {
        SetValue(color);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ColorComponentValue, TextComponentValue)
    {
        ReflectionRegistrator<ColorComponentValue>::Begin()
        .Field("color", &ColorComponentValue::GetColor, &ColorComponentValue::SetColor)
        .End();
    }
};
}

void SubPropertyValueChildCreator::ExposeChildren(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    using namespace SubPropertiesExtensionsDetail;
    InitSubPropertyTypes();
    if (parent->field.ref.GetValueType()->Decay() == DAVA::Type::Instance<DAVA::Color>() &&
        parent->propertyType == PropertyNode::RealProperty)
    {
        {
            DAVA::Reflection::Field field = parent->field;
            field.key = colorR;
            children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<DAVA::int32>(children.size()), PropertyNode::VirtualProperty));
        }
        {
            DAVA::Reflection::Field field = parent->field;
            field.key = colorG;
            children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<DAVA::int32>(children.size()), PropertyNode::VirtualProperty));
        }
        {
            DAVA::Reflection::Field field = parent->field;
            field.key = colorB;
            children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<DAVA::int32>(children.size()), PropertyNode::VirtualProperty));
        }
        {
            DAVA::Reflection::Field field = parent->field;
            field.key = colorA;
            children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<DAVA::int32>(children.size()), PropertyNode::VirtualProperty));
        }
    }
    else
    {
        if (parent->propertyType == PropertyNode::RealProperty)
        {
            const Type* valueType = parent->field.ref.GetValueType()->Decay();
            if (subPropertyTypes.count(valueType) > 0)
            {
                return;
            }
        }
    }

    ChildCreatorExtension::ExposeChildren(parent, children);
}

std::unique_ptr<BaseComponentValue> SubPropertyEditorCreator::GetEditor(const std::shared_ptr<const PropertyNode>& node) const
{
    using namespace SubPropertiesExtensionsDetail;
    InitSubPropertyTypes();

    if (node->field.ref.GetValueType()->Decay() == Type::Instance<Color>())
    {
        if (node->propertyType == PropertyNode::RealProperty)
        {
            return std::make_unique<ColorComponentValue>();
        }
        else if (node->propertyType == PropertyNode::VirtualProperty)
        {
            int32 fieldIndex = -1;
            QString name;
            if (node->field.key == colorR)
            {
                fieldIndex = 0;
                name = QString(colorR.c_str());
            }
            else if (node->field.key == colorG)
            {
                fieldIndex = 1;
                name = QString(colorG.c_str());
            }
            else if (node->field.key == colorB)
            {
                fieldIndex = 2;
                name = QString(colorB.c_str());
            }
            else if (node->field.key == colorA)
            {
                fieldIndex = 3;
                name = QString(colorA.c_str());
            }

            return std::make_unique<TextComponentValue>(std::make_unique<ColorSubFieldAccessor>(fieldIndex, name));
        }
    }

    const Type* valueType = node->field.ref.GetValueType()->Decay();
    if (subPropertyTypes.count(valueType) > 0)
    {
        if (valueType == Type::Instance<Vector2>())
        {
            return std::make_unique<kDComponentValue<Vector2, MultiDoubleSpinBox, float32>>();
        }
        else if (valueType == Type::Instance<Vector3>())
        {
            return std::make_unique<kDComponentValue<Vector3, MultiDoubleSpinBox, float32>>();
        }
        else if (valueType == Type::Instance<Vector4>())
        {
            return std::make_unique<kDComponentValue<Vector4, MultiDoubleSpinBox, float32>>();
        }
        else if (valueType == Type::Instance<Rect>())
        {
            return std::make_unique<kDComponentValue<Rect, MultiDoubleSpinBox, float32>>();
        }
        else if (valueType == Type::Instance<Color>())
        {
            if (node->field.ref.GetMeta<DAVA::M::IntColor>() == nullptr)
            {
                return std::make_unique<kDComponentValue<Color, MultiDoubleSpinBox, float32>>();
            }
            else
            {
                return std::make_unique<kDComponentValue<Color, MultiIntSpinBox, uint32>>();
            }
        }
        else if (valueType == Type::Instance<AABBox3>())
        {
            return std::make_unique<kDComponentValue<AABBox3, MultiDoubleSpinBox, float32>>();
        }
    }

    return EditorComponentExtension::GetEditor(node);
}

} // namespace TArc
} // namespace DAVA