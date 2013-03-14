#ifndef __RESOURCEEDITORQT__TILEMAPEDITORCOMMANDS__
#define __RESOURCEEDITORQT__TILEMAPEDITORCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"
#include "../SceneEditor/LandscapeEditorColor.h"

class CommandTilemapEditor: public Command
{
public:
	CommandTilemapEditor();
    
protected:
    
    virtual void Execute();
};

class CommandDrawTilemap: public Command
{
public:
	CommandDrawTilemap(Image* originalImage, Image* newImage, const String& pathname, Landscape* landscape);
	virtual ~CommandDrawTilemap();

protected:
	Image* undoImage;
	Image* redoImage;
	String savedPathname;

	Landscape* landscape;

	virtual void Execute();
	virtual void Cancel();

	LandscapeEditorColor* GetEditor();
	LandscapeEditorBase* GetActiveEditor();
	void UpdateLandscapeTilemap(Image* image);
};

#endif /* defined(__RESOURCEEDITORQT__TILEMAPEDITORCOMMANDS__) */
