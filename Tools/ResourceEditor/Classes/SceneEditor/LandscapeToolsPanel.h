#ifndef __LANDSCAPE_TOOLS_PANEL_H__
#define __LANDSCAPE_TOOLS_PANEL_H__

#include "DAVAEngine.h"
#include "LandscapeToolsSelection.h"
#include "UICheckBox.h"

using namespace DAVA;

class LandscapeToolsPanelDelegate
{
public: 
    
    virtual void OnToolSelected(LandscapeTool *newTool) = 0;
    virtual void OnShowGrid(bool show) = 0;
};

class LandscapeToolsPanel: 
    public UIControl,
    public LandscapeToolsSelectionDelegate,
    public UICheckBoxDelegate
{
protected:
    
    enum eConst
    {
        OFFSET = 1,
        SLIDER_WIDTH = 250,
        
        TEXTFIELD_WIDTH = 40,
        TEXT_WIDTH = 60
    };
    
public:
    LandscapeToolsPanel(LandscapeToolsPanelDelegate *newDelegate, const Rect & rect);
    virtual ~LandscapeToolsPanel();
    
    virtual void WillAppear();
    virtual void Input(UIEvent *currentInput);

    LandscapeTool *CurrentTool();
    void SetSelectionPanel(LandscapeToolsSelection *newPanel);
    
    //LandscapeToolsSelectionDelegate
    virtual void OnToolSelected(LandscapeToolsSelection * forControl, LandscapeTool *newTool);

    //UICheckBoxDelegate
    virtual void ValueChanged(UICheckBox *forCheckbox, bool newValue);

    virtual void SetSize(const Vector2 &newSize);
    
protected:

    void UpdateRect();
    void SetSliderHeaderPoition(UISlider *slider, const WideString &headerText);
    
    virtual void ToolIconSelected(UIControl *focused);

    
    UISlider * CreateSlider(const Rect & rect);
    void AddSliderHeader(UISlider *slider, const WideString &text);

    UICheckBox *CreateCkeckbox(const Rect &rect, const WideString &text);

    void OnBrushTool(BaseObject * object, void * userData, void * callerData);

    
    LandscapeToolsPanelDelegate *delegate;
    
    UIControl *brushIcon;
    LandscapeTool *selectedTool;
    LandscapeTool *selectedBrushTool;

    UISlider *sizeSlider;
    UISlider *strengthSlider;
	virtual void OnSizeChanged(BaseObject * object, void * userData, void * callerData);
	virtual void OnStrengthChanged(BaseObject * object, void * userData, void * callerData);
    
    LandscapeToolsSelection *selectionPanel;
    
};

#endif // __LANDSCAPE_TOOLS_PANEL_H__
