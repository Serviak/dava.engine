/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "Commands2/AddEntityCommand.h"
#include "../Qt/Scene/SceneDataManager.h"
#include "Scene3D/Entity.h"


AddEntityCommand::AddEntityCommand(DAVA::Entity* _entityToAdd, DAVA::Scene* _scene)
	: Command2(CMDID_ADD_ENTITY, "Add Entity")
    , entityToAdd(_entityToAdd)
    , scene(_scene)
{
	SafeRetain(entityToAdd);
}

AddEntityCommand::~AddEntityCommand()
{
	SafeRelease(entityToAdd);
}

void AddEntityCommand::Undo()
{
    if(NULL != scene && NULL != entityToAdd)
    {
        scene->RemoveNode(entityToAdd);
    }
}

void AddEntityCommand::Redo()
{
	scene->AddNode(entityToAdd);
}

Entity* AddEntityCommand::GetEntity() const
{
	return entityToAdd;
}