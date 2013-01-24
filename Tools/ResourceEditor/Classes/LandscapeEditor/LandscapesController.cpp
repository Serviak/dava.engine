/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "LandscapesController.h"

#include "EditorHeightmap.h"
#include "NotPassableTerrain.h"
#include "LandscapeRenderer.h"
#include "RulerToolLandscape.h"

#include "../Qt/Scene/SceneData.h"
#include "../Qt/Scene/SceneDataManager.h"

using namespace DAVA;

LandscapesController::LandscapesController()
    :   BaseObject()
{
    scene = NULL;
    renderedHeightmap = NULL;
    notPassableTerrain = NULL;
    landscapeRenderer = NULL;
    editorLandscape = NULL;
    rulerToolLandscape = NULL;
    
    currentLandscape = NULL;
    
    savedLandscape = NULL;
    savedHeightmap = NULL;
}

LandscapesController::~LandscapesController()
{
    ReleaseScene();
}

void LandscapesController::SetScene(DAVA::Scene *scene)
{
    ReleaseScene();

    this->scene = SafeRetain(scene);
    
    if(scene)
    {
        Vector<LandscapeNode *>landscapes;
        scene->GetChildNodes(landscapes);
        
        if(0 < landscapes.size())
        {
            SaveLandscape(landscapes[0]);
        }
    }
}

void LandscapesController::ReleaseScene()
{
    ReleaseLandscape(notPassableTerrain);
    notPassableTerrain = NULL;
    

    ReleaseLandscape(editorLandscape);
    editorLandscape = NULL;


    ReleaseLandscape(rulerToolLandscape);
    rulerToolLandscape = NULL;

    
    SafeRelease(renderedHeightmap);
    SafeRelease(landscapeRenderer);
    

    if(savedHeightmap && savedLandscape)
    {
        savedLandscape->SetHeightmap(savedHeightmap);
    }
    SafeRelease(savedHeightmap);
    SafeRelease(savedLandscape);
    
    
    SafeRelease(scene);
    
    currentLandscape = NULL;
}

void LandscapesController::ReleaseLandscape(EditorLandscapeNode *landscapeNode)
{
    // RETURN TO THIS CODE LATER
    //    if(landscapeNode && landscapeNode->GetParent())
    //    {
    //        landscapeNode->GetParent()->RemoveNode(landscapeNode);
    //    }
    SafeRelease(landscapeNode);
}


void LandscapesController::SaveLandscape(DAVA::LandscapeNode *landscape)
{
    SafeRelease(savedHeightmap);
    SafeRelease(savedLandscape);
    
    if(landscape)
    {
        savedLandscape = SafeRetain(landscape);
        savedHeightmap = SafeRetain(landscape->GetHeightmap());
    }
    
    currentLandscape = savedLandscape;
}


void LandscapesController::ToggleNotPassableLandscape()
{
    DVASSERT(scene && "Need set scene before");
    
    if(notPassableTerrain)
    {
		SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
		activeScene->ResetLandsacpeSelection();


        bool hidden = HideEditorLandscape(notPassableTerrain);
        if(hidden)
        {
            notPassableTerrain = NULL;
        }
    }
    else
    {
        notPassableTerrain = new NotPassableTerrain();
        bool showed = ShowEditorLandscape(notPassableTerrain);
        if(!showed)
        {
            SafeRelease(notPassableTerrain);
        }
    }
}

bool LandscapesController::ShowEditorLandscape(EditorLandscapeNode *displayingLandscape)
{
	LandscapeNode *landscape = EditorScene::GetLandscape(scene);
	if (!landscape)
    {
        Logger::Error("[LandscapesController::ShowEditorLandscape] Can be only one landscape");
        return false;
    }
	
    displayingLandscape->SetNestedLandscape(landscape);
    
    if(!landscapeRenderer)
    {
        renderedHeightmap = new EditorHeightmap(landscape->GetHeightmap());
        landscapeRenderer = new LandscapeRenderer(renderedHeightmap, landscape->GetBoundingBox());

        displayingLandscape->SetHeightmap(renderedHeightmap);
    }
    displayingLandscape->SetRenderer(landscapeRenderer);
	
	displayingLandscape->SetWorldTransformPtr(landscape->GetWorldTransformPtr());
	SceneNode* lanscapeNode = EditorScene::GetLandscapeNode(scene);
	
	lanscapeNode->RemoveComponent(Component::RENDER_COMPONENT);
	RenderComponent* component = new RenderComponent(displayingLandscape);
	lanscapeNode->AddComponent(component);

    currentLandscape = displayingLandscape;
    return true;
}

bool LandscapesController::HideEditorLandscape(EditorLandscapeNode *hiddingLandscape)
{
    hiddingLandscape->FlushChanges();
    
    EditorLandscapeNode *parentLandscape = hiddingLandscape->GetParentLandscape();
    LandscapeNode *nestedLandscape = hiddingLandscape->GetNestedLandscape();
    
    if(parentLandscape)
    {
        Heightmap *hmap = SafeRetain(parentLandscape->GetHeightmap());
        parentLandscape->SetNestedLandscape(nestedLandscape);
        parentLandscape->SetHeightmap(hmap);
        SafeRelease(hmap);

        currentLandscape = parentLandscape;
    }
    else
    {
        EditorLandscapeNode *editorLandscape = dynamic_cast<EditorLandscapeNode *>(nestedLandscape);
        if(editorLandscape)
        {
            editorLandscape->SetParentLandscape(NULL);
        }
		
		SceneNode* lanscapeNode = EditorScene::GetLandscapeNode(scene);
		lanscapeNode->RemoveComponent(Component::RENDER_COMPONENT);
		lanscapeNode->AddComponent(new RenderComponent(nestedLandscape));
        
        if(NeedToKillRenderer(nestedLandscape))
        {
            SafeRelease(renderedHeightmap);
            SafeRelease(landscapeRenderer);
        }
        
        currentLandscape = nestedLandscape;
    }
    

    SafeRelease(hiddingLandscape);
    return true;
}


bool LandscapesController::NeedToKillRenderer(DAVA::LandscapeNode *landscapeForDetection)
{
    return !(IsPointerToExactClass<EditorLandscapeNode>(landscapeForDetection));
}


bool LandscapesController::EditorLandscapeIsActive()
{
    return (NULL != notPassableTerrain) || (NULL != landscapeRenderer) || (NULL != renderedHeightmap) || (NULL != rulerToolLandscape);
}

EditorLandscapeNode *LandscapesController::CreateEditorLandscapeNode()
{
    editorLandscape = new EditorLandscapeNode();
    bool showed = ShowEditorLandscape(editorLandscape);
    if(!showed)
    {
        SafeRelease(editorLandscape);
    }
    
    return editorLandscape;
}

void LandscapesController::ReleaseEditorLandscapeNode()
{
    bool hidden = HideEditorLandscape(editorLandscape);
    if(hidden)
    {
        editorLandscape = NULL;
    }
}

DAVA::LandscapeNode * LandscapesController::GetCurrentLandscape()
{
    return currentLandscape;
}

DAVA::Heightmap * LandscapesController::GetCurrentHeightmap()
{
    if(currentLandscape)
    {
        return currentLandscape->GetHeightmap();
    }
    
    return NULL;
}


void LandscapesController::HeghtWasChanged(const DAVA::Rect &changedRect)
{
    landscapeRenderer->RebuildVertexes(changedRect);
    renderedHeightmap->HeghtWasChanged(changedRect);

    EditorLandscapeNode *editorLandscape = dynamic_cast<EditorLandscapeNode *>(currentLandscape);
    if(editorLandscape)
    {
        editorLandscape->HeihghtmapUpdated(changedRect);
    }
}


void LandscapesController::CursorEnable()
{
    currentLandscape->CursorEnable();
}

void LandscapesController::CursorDisable()
{
    currentLandscape->CursorDisable();
}

RulerToolLandscape *LandscapesController::CreateRulerToolLandscape()
{
    rulerToolLandscape = new RulerToolLandscape();
    bool showed = ShowEditorLandscape(rulerToolLandscape);
    if(!showed)
    {
        SafeRelease(rulerToolLandscape);
    }
    
    return rulerToolLandscape;
}

void LandscapesController::ReleaseRulerToolLandscape()
{
    bool hidden = HideEditorLandscape(rulerToolLandscape);
    if(hidden)
    {
        rulerToolLandscape = NULL;
    }
}


