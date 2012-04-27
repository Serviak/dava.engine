#ifndef __CONTROLS_FACTORY_H__
#define __CONTROLS_FACTORY_H__

#include "DAVAEngine.h"

using namespace DAVA;

class ControlsFactory 
{
public:
    
    enum eGeneralControlSizes
    {
        BUTTON_HEIGHT = 20,
        BUTTON_WIDTH = 80,
        
        LEFT_PANEL_WIDTH = 200,
        RIGHT_PANEL_WIDTH = 200,
        OUTPUT_PANEL_HEIGHT = 200,
       
        OFFSET = 10,
        
        ERROR_MESSAGE_HEIGHT = 30,
        
        TEXTURE_PREVIEW_HEIGHT = 100,
        TEXTURE_PREVIEW_WIDTH = 200,
        
        TOOLS_HEIGHT = 40,
        TOOL_BUTTON_SIDE = 32,
        
        CELL_HEIGHT = 20,
    };
    
    enum eColorPickerSizes
    {
        COLOR_MAP_SIDE = 202,
        COLOR_SELECTOR_WIDTH = 20,
        COLOR_PREVIEW_SIDE = 80,
    };
    
    
public:
    
    static void AddBorder(UIControl *c);
    
    static UIButton *CreateButton(Vector2 pos, const WideString &buttonText);
    static UIButton *CreateButton(const Rect & rect, const WideString &buttonText);
    static void CustomizeButton(UIButton *btn, const WideString &buttonText);

    static void CustomizeButtonExpandable(UIButton *btn);

    static UIButton *CreateImageButton(const Rect & rect, const String &imagePath);
    static void CustomizeImageButton(UIButton *btn, const String &imagePath);
    
    static UIButton *CreateCloseWindowButton(const Rect & rect);
    static void CustomizeCloseWindowButton(UIButton *btn);

    
    static Font* GetFontLight();
    static Font* GetFontDark();
    static Font* GetFontError();
    static void CustomizeFontLight(Font *font);
    static void CustomizeFontDark(Font *font);
    static void CustomizeFontError(Font *font);

    static void CustomizeScreenBack(UIControl *screen);
    
    static UIControl * CreateLine(const Rect & rect);
    static UIControl * CreateLine(const Rect & rect, Color color);
    
    static void CusomizeBottomLevelControl(UIControl *c);

    static void CusomizeTopLevelControl(UIControl *c);

    static void CusomizeListControl(UIControl *c);
    
    static void CusomizeTransparentControl(UIControl *c, float32 transparentLevel);
    
    
    static UIControl *CreatePanelControl(const Rect & rect, bool addBorder = true);
    static void CustomizePanelControl(UIControl *c, bool addBorder = true);
    
    static void CustomizeExpandButton(UIButton *btn);

    static void CustomizeListCell(UIListCell *c, const WideString &text);
    static void CustomizeListCellAlternative(UIListCell *c, const WideString &text);
    static void CustomizeSceneGraphCell(UIHierarchyCell *c);
    
    static void CustomizeMenuPopupCell(UIListCell *c, const WideString &text);
    
    static void CustomizePropertyCell(UIControl *c, bool isActivePart);
    static void CustomizeEditablePropertyCell(UIControl *c);
    static void CustomizeUneditablePropertyCell(UIControl *c);
    static void CustomizePropertySectionCell(UIControl *c);
    static void CustomizePropertySubsectionCell(UIControl *c);
    static void CustomizePropertyButtonCell(UIListCell *c);
    
    static void CustomizeDialogFreeSpace(UIControl *c);
    static void CustomizeDialog(UIControl *c);
    
    static void SetScrollbar(UIList *l);
    static void SetScrollbar(UIHierarchy *l);
    
    static Font* fontLight;
    static Font* fontDark;
    static Font* fontError;
};



#endif // __CONTROLS_FACTORY_H__