#include "UICheckBox.h"

REGISTER_CLASS(UICheckBox);

UICheckBox::UICheckBox()
	:   UIControl()
{
	checked = false;
    checkboxDelegate = NULL;

    GetBackground()->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    
    AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UICheckBox::OnClick));
}

UICheckBox::UICheckBox(const String &spriteName, const Rect &rect, bool rectInAbsoluteCoordinates)
    :   UIControl(rect, rectInAbsoluteCoordinates)
{
    checkboxDelegate = NULL;
    
    GetBackground()->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    
    SetSprite(spriteName, 0);
    SetChecked(false);

    AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &UICheckBox::OnClick));
}

void UICheckBox::LoadFromYamlNode( YamlNode * node, UIYamlLoader * loader )
{
	UIControl::LoadFromYamlNode(node, loader);
}

void UICheckBox::LoadFromYamlNodeCompleted()
{
    UIControl::LoadFromYamlNodeCompleted();

	SetChecked(checked);
}

void UICheckBox::SetChecked( bool _checked )
{
	checked = _checked;

    if(GetSprite())
    {
        SetSpriteFrame((checked) ? 1 : 0);
    }
    
    if(checkboxDelegate)
    {
        checkboxDelegate->ValueChanged(checked);
    }
}

bool UICheckBox::Checked()
{
	return checked;
}

void UICheckBox::OnClick( BaseObject * owner, void * userData, void * callerData )
{
	SetChecked(!checked);
}

void UICheckBox::SetDelegate(UICheckBoxDelegate *delegate)
{
    checkboxDelegate = delegate;
}