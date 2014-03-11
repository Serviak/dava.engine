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



#ifndef __RESOURCEEDITORQT__GRASSEDITORSYSTEM__
#define __RESOURCEEDITORQT__GRASSEDITORSYSTEM__

#include "DAVAEngine.h"
#include "LandscapeEditorDrawSystem.h"
#include "Render/Highlevel/VegetationRenderObject.h"

class SceneCollisionSystem;
class SceneSelectionSystem;
class EntityModificationSystem;
class Command2;

using namespace DAVA;

class GrassEditorSystem: public DAVA::SceneSystem
{
public:
	GrassEditorSystem(Scene* scene);
	virtual ~GrassEditorSystem();

	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);
    void ProcessCommand(const Command2 *command, bool redo);

    bool EnableGrassEdit(bool enable);
    bool IsEnabledGrassEdit() const;

    void SetLayerVisible(uint8 layer, bool visible);
    bool IsLayerVisible(uint8 layer) const;

    void SetCurrentLayer(uint8 layer);
    uint8 GetCurrentLayer() const;

    void SetBrushHeight(uint8 height);
    uint8 GetBrushHeight() const;

    void SetBrushDensity(uint8 density);
    uint8 GetBrushDensity() const;

    DAVA::VegetationRenderObject *GetCurrentVegetationObject() const;

    static DAVA::Rect2i GetAffectedImageRect(DAVA::AABBox2 &area);

protected:
	bool isEnabled;
    bool inDrawState;

    DAVA::Vector2 curCursorPos;
    DAVA::AABBox2 affectedArea;
    
    uint8 curBrush;
    uint8 curLayer;

	SceneCollisionSystem* collisionSystem;
	SceneSelectionSystem* selectionSystem;
	EntityModificationSystem* modifSystem;
	LandscapeEditorDrawSystem* drawSystem;

	Texture* cursorTexture;
    DAVA::Vector2 cursorPosition;
    DAVA::VegetationMap *vegetationMap;
    DAVA::VegetationMap *vegetationMapCopy;
    DAVA::VegetationRenderObject *curVegetation; 

    void UpdateCursorPos();
    void DrawGrass(DAVA::Vector2 pos);
    void DrawGrassEnd();
    void BuildGrassCopy(DAVA::AABBox2 area = DAVA::AABBox2());

    DAVA::VegetationRenderObject* SearchVegetation(DAVA::Entity *entity) const;
};

#endif /* defined(__RESOURCEEDITORQT__GRASSEDITORSYSTEM__) */
