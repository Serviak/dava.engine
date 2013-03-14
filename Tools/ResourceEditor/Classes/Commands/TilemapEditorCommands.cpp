#include "TilemapEditorCommands.h"
#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../Qt/Main/GUIState.h"
#include "../SceneEditor/EditorBodyControl.h"

#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "../LandscapeEditor/EditorLandscape.h"
#include "../LandscapeEditor/LandscapesController.h"

//Show/Hide Tilemap Editor
CommandTilemapEditor::CommandTilemapEditor()
:   Command(Command::COMMAND_WITHOUT_UNDO_EFFECT)
{
}


void CommandTilemapEditor::Execute()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->TilemapTriggered();
        GUIState::Instance()->SetNeedUpdatedToolsMenu(true);
        GUIState::Instance()->SetNeedUpdatedToolbar(true);
    }
	
	//    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
	//    activeScene->RebuildSceneGraph();
}


CommandDrawTilemap::CommandDrawTilemap(Image* originalImage, Image* newImage, const String& pathname, Landscape* landscape)
:	Command(COMMAND_UNDO_REDO)
,	landscape(landscape)
{
	commandName = "Tilemap Draw";

	savedPathname = FileSystem::Instance()->ReplaceExtension(pathname, ".png");;
	undoImage = SafeRetain(originalImage);
	redoImage = SafeRetain(newImage);
}

CommandDrawTilemap::~CommandDrawTilemap()
{
	SafeRelease(undoImage);
	SafeRelease(redoImage);
}

void CommandDrawTilemap::Execute()
{
	LandscapeEditorColor* editor = GetEditor();
	if (editor == NULL)
	{
		SetState(STATE_INVALID);
		return;
	}

	UpdateLandscapeTilemap(redoImage);
}

void CommandDrawTilemap::Cancel()
{
	UpdateLandscapeTilemap(undoImage);
}

void CommandDrawTilemap::UpdateLandscapeTilemap(DAVA::Image *image)
{
	Texture* texture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight(), false);
	texture->relativePathname = savedPathname;
	texture->GenerateMipmaps();
	texture->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);

	LandscapeEditorBase* editor = GetActiveEditor();
	if (editor)
	{
		editor->UpdateLandscapeTilemap(texture);
	}
	else if (landscape)
	{
		landscape->SetTexture(Landscape::TEXTURE_TILE_MASK, texture);
		landscape->UpdateFullTiledTexture();
		ImageLoader::Save(image, savedPathname);
	}

	SafeRelease(texture);
}

LandscapeEditorBase* CommandDrawTilemap::GetActiveEditor()
{
	LandscapeEditorBase* editor = NULL;
	
	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = screen->FindCurrentBody()->bodyControl->GetCurrentLandscapeEditor();
	}

	return editor;
}

LandscapeEditorColor* CommandDrawTilemap::GetEditor()
{
	LandscapeEditorColor* editor = NULL;

	SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	if(screen)
	{
		editor = dynamic_cast<LandscapeEditorColor*>(screen->FindCurrentBody()->bodyControl->GetLandscapeEditor(SceneEditorScreenMain::ELEMID_COLOR_MAP));
	}

	return editor;
}