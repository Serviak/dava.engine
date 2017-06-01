#include "UIStaticTextComponent.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIStaticTextComponent)
{
    ReflectionRegistrator<UIStaticTextComponent>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](UIStaticTextComponent* o) { o->Release(); })
    .Field("align", &UIStaticTextComponent::GetAlign, &UIStaticTextComponent::SetAlign)[M::FlagsT<eAlign>()]
    .Field("text", &UIStaticTextComponent::GetText, &UIStaticTextComponent::SetText)
    .Field("fontName", &UIStaticTextComponent::GetFontName, &UIStaticTextComponent::SetFontName)
    .Field("multiline", &UIStaticTextComponent::GetMultiline, &UIStaticTextComponent::SetMultiline)[M::EnumT<eTextMultiline>()]
    .Field("fitting", &UIStaticTextComponent::GetFitting, &UIStaticTextComponent::SetFitting)[M::EnumT<eTextFitting>()]
    .Field("color", &UIStaticTextComponent::GetColor, &UIStaticTextComponent::SetColor)
    .Field("colorInheritType", &UIStaticTextComponent::GetColorInheritType, &UIStaticTextComponent::SetColorInheritType)[M::EnumT<UIControlBackground::eColorInheritType>()]
    .Field("shadowOffset", &UIStaticTextComponent::GetShadowOffset, &UIStaticTextComponent::SetShadowOffset)
    .Field("shadowColor", &UIStaticTextComponent::GetShadowColor, &UIStaticTextComponent::SetShadowColor)
    .Field("perPixelAccuracyType", &UIStaticTextComponent::GetPerPixelAccuracyType, &UIStaticTextComponent::SetPerPixelAccuracyType)[M::EnumT<UIControlBackground::ePerPixelAccuracyType>()]
    .Field("useRtlAlign", &UIStaticTextComponent::GetUseRtlAlign, &UIStaticTextComponent::SetUseRtlAlign)[M::EnumT<TextBlock::eUseRtlAlign>()]
    .Field("forceBiDiSupport", &UIStaticTextComponent::IsForceBiDiSupportEnabled, &UIStaticTextComponent::SetForceBiDiSupportEnabled)
    .End();
}

UIStaticTextComponent::UIStaticTextComponent(const UIStaticTextComponent& src)
    : UIBaseComponent(src)
    , align(src.align)
    , text(src.text)
    , fontName(src.fontName)
    , multiline(src.multiline)
    , fitting(src.fitting)
    , color(src.color)
    , colorInheritType(src.colorInheritType)
    , shadowOffset(src.shadowOffset)
    , shadowColor(src.shadowColor)
    , perPixelAccuracyType(src.perPixelAccuracyType)
    , useRtlAlign(src.useRtlAlign)
    , forceBiDiSupport(src.forceBiDiSupport)
    , modified(true)
{
}

UIStaticTextComponent* UIStaticTextComponent::Clone() const
{
    return new UIStaticTextComponent(*this);
}

void UIStaticTextComponent::SetAlign(int32 _align)
{
    align = _align;
    modified = true;
}

int32 UIStaticTextComponent::GetAlign() const
{
    return align;
}

void UIStaticTextComponent::SetText(const String& text_)
{
    text = text_;
    modified = true;
}

String UIStaticTextComponent::GetText() const
{
    return text;
}

void UIStaticTextComponent::SetFitting(eTextFitting fitting_)
{
    fitting = fitting_;
    modified = true;
}

UIStaticTextComponent::eTextFitting UIStaticTextComponent::GetFitting() const
{
    return fitting;
}

void UIStaticTextComponent::SetFontName(const String& fontName_)
{
    fontName = fontName_;
    modified = true;
}

String UIStaticTextComponent::GetFontName() const
{
    return fontName;
}

void UIStaticTextComponent::SetColor(const Color& color_)
{
    color = color_;
    modified = true;
}

const Color& UIStaticTextComponent::GetColor() const
{
    return color;
}

void UIStaticTextComponent::SetMultiline(eTextMultiline multiline_)
{
    multiline = multiline_;
    modified = true;
}

UIStaticTextComponent::eTextMultiline UIStaticTextComponent::GetMultiline() const
{
    return multiline;
}

void UIStaticTextComponent::SetColorInheritType(UIControlBackground::eColorInheritType type)
{
    colorInheritType = type;
    modified = true;
}

UIControlBackground::eColorInheritType UIStaticTextComponent::GetColorInheritType() const
{
    return colorInheritType;
}

void UIStaticTextComponent::SetShadowOffset(const Vector2& offset)
{
    shadowOffset = offset;
    modified = true;
}

const Vector2& UIStaticTextComponent::GetShadowOffset() const
{
    return shadowOffset;
}

void UIStaticTextComponent::SetShadowColor(const Color& color_)
{
    shadowColor = color_;
    modified = true;
}

const Color& UIStaticTextComponent::GetShadowColor() const
{
    return shadowColor;
}

void UIStaticTextComponent::SetPerPixelAccuracyType(UIControlBackground::ePerPixelAccuracyType type)
{
    perPixelAccuracyType = type;
    modified = true;
}

UIControlBackground::ePerPixelAccuracyType UIStaticTextComponent::GetPerPixelAccuracyType() const
{
    return perPixelAccuracyType;
}

void UIStaticTextComponent::SetUseRtlAlign(TextBlock::eUseRtlAlign useRtlAlign_)
{
    useRtlAlign = useRtlAlign_;
    modified = true;
}

TextBlock::eUseRtlAlign UIStaticTextComponent::GetUseRtlAlign() const
{
    return useRtlAlign;
}

void UIStaticTextComponent::SetForceBiDiSupportEnabled(bool value)
{
    forceBiDiSupport = value;
    modified = true;
}
bool UIStaticTextComponent::IsForceBiDiSupportEnabled() const
{
    return forceBiDiSupport;
}
};
