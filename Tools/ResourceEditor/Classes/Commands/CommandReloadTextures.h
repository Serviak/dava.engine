#ifndef __COMMAND_RELOAD_TEXTURES_H__
#define __COMMAND_RELOAD_TEXTURES_H__

#include "Command.h"
#include "../Constants.h"

class CommandReloadTextures: public Command
{
public:	
	DAVA_DEPRECATED(CommandReloadTextures());// DEPRECATED: using SceneDataManager
    
protected:
    virtual void Execute();
};




#endif // #ifndef __COMMAND_RELOAD_TEXTURES_H__