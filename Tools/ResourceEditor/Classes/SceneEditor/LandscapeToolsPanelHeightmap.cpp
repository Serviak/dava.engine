#include "LandscapeToolsPanelHeightmap.h"
#include "ControlsFactory.h"
#include "LandscapeTool.h"

#pragma mark  --LandscapeToolsPanelHeightmap
LandscapeToolsPanelHeightmap::LandscapeToolsPanelHeightmap(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect)
    :   LandscapeToolsPanel(newDelegate, rect)
{
    Rect sizeRect(rect.dx - ControlsFactory::BUTTON_HEIGHT - TEXTFIELD_WIDTH, 
                  0, TEXTFIELD_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2);
    sizeValue = CreateTextField(sizeRect);
    AddControl(sizeValue);
    
    Rect strengthRect(sizeRect);
    strengthRect.y = sizeRect.y + sizeRect.dy;
    strengthValue = CreateTextField(strengthRect);
    AddControl(strengthValue);
    
    
    sizeSlider = CreateSlider(Rect(rect.dx - SLIDER_WIDTH - ControlsFactory::BUTTON_HEIGHT - TEXTFIELD_WIDTH,
                                   0, SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    sizeSlider->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeToolsPanelHeightmap::OnSizeChanged));
    sizeSlider->SetMinMaxValue(0.f, LandscapeTool::DefaultSize());
    sizeSlider->SetValue(LandscapeTool::DefaultSize() / 2.0f);
    
    strengthSlider = CreateSlider(Rect(rect.dx - SLIDER_WIDTH - ControlsFactory::BUTTON_HEIGHT - TEXTFIELD_WIDTH, 
                                       ControlsFactory::TOOLS_HEIGHT / 2, SLIDER_WIDTH, ControlsFactory::TOOLS_HEIGHT / 2));
    strengthSlider->AddEvent(UIControl::EVENT_VALUE_CHANGED, Message(this, &LandscapeToolsPanelHeightmap::OnStrengthChanged));
    strengthSlider->SetMinMaxValue(-LandscapeTool::DefaultStrength(), LandscapeTool::DefaultStrength());
    strengthSlider->SetValue(0.0f);

    Rect lineRect;
    lineRect.x = strengthSlider->GetRect().x + strengthSlider->GetRect().dx/2;
    lineRect.dx = 2;
    lineRect.y = strengthSlider->GetRect().y - 2;
    lineRect.dy = strengthSlider->GetRect().dy + 4;
    UIControl *line = ControlsFactory::CreateLine(lineRect);
    AddControl(line);
    SafeRelease(line);

    
    AddControl(sizeSlider);
    AddControl(strengthSlider);
    
    
    
        
    AddSliderHeader(sizeSlider, LocalizedString(L"landscapeeditor.size"));
    AddSliderHeader(strengthSlider, LocalizedString(L"landscapeeditor.strength"));
    
    sizeValue->SetText(Format(L"%.3f", LandscapeTool::DefaultSize()));
    strengthValue->SetText(Format(L"%.3f", LandscapeTool::DefaultStrength()));
    
    
    relative = CreateCkeckbox(Rect(0, ControlsFactory::TOOLS_HEIGHT, 
                                   ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_HEIGHT), 
                              LocalizedString(L"landscapeeditor.relative"));
    average = CreateCkeckbox(Rect(0, ControlsFactory::TOOLS_HEIGHT + ControlsFactory::BUTTON_HEIGHT, 
                                  ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_HEIGHT), 
                              LocalizedString(L"landscapeeditor.average"));
    
    Rect heightRect;
    heightRect.x = relative->GetRect().x + relative->GetRect().dx + TEXT_WIDTH + ControlsFactory::OFFSET + ControlsFactory::OFFSET/2.0;
    heightRect.y = relative->GetRect().y;
    heightRect.dx = TEXTFIELD_WIDTH;
    heightRect.dy = relative->GetRect().dy;
    heightValue = CreateTextField(Rect(heightRect));
    heightValue->SetText(L"0");
    AddControl(heightValue);
    
    heightRect.x = heightRect.x + heightRect.dx + ControlsFactory::OFFSET/2.0;
    heightRect.dx = TEXT_WIDTH;
    
    UIStaticText *textControl = new UIStaticText(heightRect);
    textControl->SetText(LocalizedString(L"landscapeeditor.height"));
    textControl->SetFont(ControlsFactory::GetFontLight());
    textControl->SetAlign(ALIGN_VCENTER | ALIGN_LEFT);
    AddControl(textControl);
    SafeRelease(textControl);

    
    relative->SetChecked(true, false);
    strengthSlider->SetValue(1.0f);
}


LandscapeToolsPanelHeightmap::~LandscapeToolsPanelHeightmap()
{
    SafeRelease(heightValue);
    
    SafeRelease(average);
    SafeRelease(relative);
    
    SafeRelease(sizeSlider);
    SafeRelease(strengthSlider);

    SafeRelease(sizeValue);
    SafeRelease(strengthValue);
}

UICheckBox *LandscapeToolsPanelHeightmap::CreateCkeckbox(const Rect &rect, const WideString &text)
{
    UICheckBox *checkbox = new UICheckBox("~res:/Gfx/UI/chekBox", rect);
    checkbox->SetDelegate(this);
    AddControl(checkbox);
    
    Rect textRect;
    textRect.x = rect.x + rect.dx + ControlsFactory::OFFSET;
    textRect.y = rect.y;
    textRect.dx = TEXT_WIDTH;
    textRect.dy = rect.dy;
    
    UIStaticText *textControl = new UIStaticText(textRect);
    textControl->SetText(text);
    textControl->SetFont(ControlsFactory::GetFontLight());
    textControl->SetAlign(ALIGN_VCENTER | ALIGN_LEFT);
    AddControl(textControl);
    SafeRelease(textControl);
    
    return checkbox;
}

void LandscapeToolsPanelHeightmap::OnStrengthChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedTool)
    {
        selectedTool->strength = strengthSlider->GetValue();
    }
}

void LandscapeToolsPanelHeightmap::OnSizeChanged(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(selectedTool)
    {
        selectedTool->size = sizeSlider->GetValue();
    }
}

UITextField *LandscapeToolsPanelHeightmap::CreateTextField(const Rect &rect)
{
    Font * font = ControlsFactory::GetFontLight();
    UITextField *tf = new UITextField(rect);
    ControlsFactory::CustomizeEditablePropertyCell(tf);
    tf->SetFont(font);
    tf->SetDelegate(this);
    
    return tf;
}

#pragma mark  --UITextFieldDelegate
void LandscapeToolsPanelHeightmap::TextFieldShouldReturn(UITextField * textField)
{
    textField->ReleaseFocus();
}

void LandscapeToolsPanelHeightmap::TextFieldShouldCancel(UITextField * textField)
{
    textField->ReleaseFocus();
}

void LandscapeToolsPanelHeightmap::TextFieldLostFocus(UITextField * textField)
{
    if(textField == sizeValue)
    {
        float32 value = atof(WStringToString(sizeValue->GetText()).c_str());
        
        sizeSlider->SetMinMaxValue(0.f, value);
        selectedTool->maxSize = value;
        if(value < selectedTool->size)
        {
            selectedTool->size = value;
        }
        sizeSlider->SetValue(selectedTool->size);
    }
    else if(textField == strengthValue)
    {
        float32 value = fabsf(atof(WStringToString(strengthValue->GetText()).c_str()));

        strengthSlider->SetMinMaxValue(-value, value);
        selectedTool->maxStrength = value;
        if(value < selectedTool->strength)
        {
            selectedTool->strength = value;
        }
        else if(selectedTool->strength < -value)
        {
            selectedTool->strength = -value;
        }
        strengthSlider->SetValue(selectedTool->strength);
    }
    else if(heightValue)
    {
        float32 value = fabsf(atof(WStringToString(heightValue->GetText()).c_str()));
        if(selectedTool)
        {
            selectedTool->height = value;
        }
    }
}

bool LandscapeToolsPanelHeightmap::TextFieldKeyPressed(UITextField * textField, int32 replacementLocation, int32 replacementLength, const WideString & replacementString)
{
    if (replacementLength < 0) 
    {
        return true;
    }

    WideString newText = textField->GetAppliedChanges(replacementLocation, replacementLength, replacementString);
    bool allOk;
    int pointsCount = 0;
    for (int i = 0; i < newText.length(); i++) 
    {
        allOk = false;
        if (newText[i] == L'-' && i == 0)
        {
            allOk = true;
        }
        else if(newText[i] >= L'0' && newText[i] <= L'9')
        {
            allOk = true;
        }
        else if(newText[i] == L'.' && pointsCount == 0)
        {
            allOk = true;
            pointsCount++;
        }
        if (!allOk) 
        {
            return false;
        }
    }
    return true;
};

#pragma mark  --UICheckBoxDelegate
void LandscapeToolsPanelHeightmap::ValueChanged(UICheckBox *forCheckbox, bool newValue)
{
    if(forCheckbox == average)
    {
        if(newValue)
        {
            relative->SetChecked(false, false);
        }
    }
    else if(forCheckbox == relative)
    {
        if(newValue)
        {
            average->SetChecked(false, false);
        }
    }
    
    selectedTool->averageDrawing = average->Checked();
    selectedTool->relativeDrawing = relative->Checked();
}

#pragma mark  --LandscapeToolsSelectionDelegate
void LandscapeToolsPanelHeightmap::OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool)
{
    newTool->size = sizeSlider->GetValue();
    newTool->strength = strengthSlider->GetValue();
    newTool->height = fabsf(atof(WStringToString(heightValue->GetText()).c_str()));
    newTool->averageDrawing = average->Checked();
    newTool->relativeDrawing = relative->Checked();
    
    LandscapeToolsPanel::OnToolSelected(forControl, newTool);
}

