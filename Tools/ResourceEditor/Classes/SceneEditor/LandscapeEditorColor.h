#ifndef __LANDSCAPE_EDITOR_COLOR_H__
#define __LANDSCAPE_EDITOR_COLOR_H__

#include "DAVAEngine.h"
#include "LandscapeEditorBase.h"
#include "LandscapeToolsPanel.h"
#include "LandscapeEditorPropertyControl.h"

using namespace DAVA;

class LandscapeEditorColor
    :   public LandscapeEditorBase
    ,   public LandscapeEditorPropertyControlDelegate

{
    
public:
    
    LandscapeEditorColor(LandscapeEditorDelegate *newDelegate, EditorBodyControl *parentControl, const Rect &toolsRect); 
    virtual ~LandscapeEditorColor();
    
	virtual void Draw(const UIGeometricData &geometricData);

    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect);
    
    //LE property control delegate
    virtual void LandscapeEditorSettingsChanged(LandscapeEditorSettings *settings);
    virtual void TextureWillChanged(const String &forKey);
    virtual void TextureDidChanged(const String &forKey);
    
protected:

    virtual void InputAction(int32 phase, bool intersects);
    virtual void HideAction();
    virtual void ShowAction();
    virtual void SaveTextureAction(const String &pathToFile);
	virtual void UpdateCursor();
    virtual void UndoAction();
    virtual void RedoAction();

    virtual void RecreateHeightmapNode();

    
    void CreateMaskTexture();
    void CreateMaskFromTexture(Texture *tex);

	void UpdateTileMaskTool();
    void UpdateTileMask();
    
    Sprite *maskSprite;
	Sprite *oldMaskSprite;
	Sprite *toolSprite;
    
    Texture *savedTexture;

	bool wasTileMaskToolUpdate;
    
    LandscapeEditorSettings *settings;
    
    eBlendMode srcBlendMode;
    eBlendMode dstBlendMode;
    Color paintColor;
    
	Shader * tileMaskEditorShader;
    bool editingIsEnabled;
};


#endif //__LANDSCAPE_EDITOR_COLOR_H__
