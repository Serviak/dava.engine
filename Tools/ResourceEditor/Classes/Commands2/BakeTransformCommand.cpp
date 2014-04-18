/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "Commands2/BakeTransformCommand.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Entity.h"

BakeTransformCommand::BakeTransformCommand(DAVA::Entity* _entity, bool _inverse)
	: Command2(CMDID_BAKE_TRANSFORM, "Bake transform")
	, entity(_entity)
    , inverse(_inverse)
{
    if(NULL != entity)
    {
	    origTransform = entity->GetLocalTransform();
    }

	DAVA::RenderObject * ro = GetRenderObject(entity);
    if(NULL != ro)
    {
        DAVA::AABBox3 box = ro->GetBoundingBox();
        toCenterModif.CreateTranslation(box.GetCenter());
    }
}

BakeTransformCommand::~BakeTransformCommand()
{

}

void BakeTransformCommand::Undo()
{
	if(NULL != entity)
	{
        // move pivot point back from the object center
        if(inverse)
        {
		    DAVA::RenderObject * ro = GetRenderObject(entity);
            if(NULL != ro)
		    {
                DAVA::Matrix4 trMove = toCenterModif;
                trMove.Inverse();

                ro->BakeTransform(toCenterModif);
                entity->SetLocalTransform(entity->GetLocalTransform() * trMove);
            }
        }
        // move pivot back from zero pos
        else
        {
		    DAVA::RenderObject * ro = GetRenderObject(entity);
            if(NULL != ro)
		    {
                DAVA::Matrix4 tr = origTransform;
                tr.Inverse();
                ro->BakeTransform(tr);
            }

            entity->SetLocalTransform(origTransform);
        }
	}
}

void BakeTransformCommand::Redo()
{
	if(NULL != entity)
	{
        // move pivot point to the object center
        if(inverse)
        {
		    DAVA::RenderObject * ro = GetRenderObject(entity);
            if(NULL != ro)
		    {
                DAVA::Matrix4 trBake = toCenterModif;
                trBake.Inverse();

                ro->BakeTransform(trBake);
                entity->SetLocalTransform(entity->GetLocalTransform() * toCenterModif);
            }
        }
        // move pivot point to zero pos
        else
        {
		    DAVA::RenderObject * ro = GetRenderObject(entity);
            if(NULL != ro)
		    {
                ro->BakeTransform(origTransform);
            }

            entity->SetLocalTransform(DAVA::Matrix4::IDENTITY);
        }
	}
}

DAVA::Entity* BakeTransformCommand::GetEntity() const
{
	return entity;
}

void BakeTransformCommand::PivotToZeroPos()
{

}

void BakeTransformCommand::PivotToCenterPos()
{

}
