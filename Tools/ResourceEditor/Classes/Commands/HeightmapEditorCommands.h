#ifndef __RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS__
#define __RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS__

#include "DAVAEngine.h"
#include "Command.h"
#include "../SceneEditor/LandscapeEditorHeightmap.h"

class CommandHeightmapEditor: public Command
{
public:
	CommandHeightmapEditor();
    
protected:
    
    virtual void Execute();
};

class HeightmapModificationCommand: public Command
{
public:
	HeightmapModificationCommand(Command::eCommandType type);

protected:
	static String TimeString();
	static String SaveHeightmap(Heightmap* heightmap);
	static LandscapeEditorHeightmap* GetEditor();
	static void UpdateLandscapeHeightmap(String filename);
};

class CommandDrawHeightmap: public HeightmapModificationCommand
{
public:
	CommandDrawHeightmap(Heightmap* originalHeightmap, Heightmap* newHeightmap);
	virtual ~CommandDrawHeightmap();
	
protected:
	String undoFilename;
	String redoFilename;

	virtual void Execute();
	virtual void Cancel();
};

class CommandCopyPasteHeightmap: public HeightmapModificationCommand
{
public:
	CommandCopyPasteHeightmap(bool copyHeightmap, bool copyTilemap, Heightmap* originalHeightmap, Heightmap* newHeightmap, Image* originalTilemap, Image* newTilemap, const String& tilemapSavedPath);
	virtual ~CommandCopyPasteHeightmap();

protected:
	String heightmapUndoFilename;
	String heightmapRedoFilename;

	Image* tilemapUndoImage;
	Image* tilemapRedoImage;
	String tilemapSavedPathname;

	LandscapeNode* landscape;

	bool heightmap;
	bool tilemap;

	virtual void Execute();
	virtual void Cancel();

	void UpdateLandscapeTilemap(Image* image);
	LandscapeEditorBase* GetActiveEditor();
};

#endif /* defined(__RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS__) */
