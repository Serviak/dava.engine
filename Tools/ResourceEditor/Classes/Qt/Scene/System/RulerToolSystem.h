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

#ifndef __RESOURCEEDITORQT__RULERTOOLSYSTEM__
#define __RESOURCEEDITORQT__RULERTOOLSYSTEM__

#include "Entity/SceneSystem.h"
#include "EditorScene.h"

class SceneCollisionSystem;
class SceneSelectionSystem;
class EntityModificationSystem;
class LandscapeEditorDrawSystem;

class RulerToolSystem: public DAVA::SceneSystem
{
	static const DAVA::int32 APPROXIMATION_COUNT = 10;

public:
	RulerToolSystem(Scene* scene);
	virtual ~RulerToolSystem();

	bool EnableLandscapeEditing();
	bool DisableLandscapeEdititing();
	bool IsLandscapeEditingEnabled() const;

	void Update(DAVA::float32 timeElapsed);
	void ProcessUIEvent(DAVA::UIEvent *event);

protected:
	bool enabled;

	SceneCollisionSystem* collisionSystem;
	SceneSelectionSystem* selectionSystem;
	EntityModificationSystem* modifSystem;
	LandscapeEditorDrawSystem* drawSystem;

	Texture* cursorTexture;
	uint32 cursorSize;
	uint32 curToolSize;
	Sprite* toolImageSprite;

	int32 landscapeSize;
	bool isIntersectsLandscape;
	Vector2 cursorPosition;
	Vector2 prevCursorPos;

	List<Vector3> linePoints;
	float32 length;
	Vector3 previewPoint;
	float32 previewLength;

	void UpdateCursorPosition(int32 landscapeSize);

	void SetStartPoint(const Vector3 &point);
	void AddPoint(const Vector3 &point);
	void CalcPreviewPoint(const Vector3& point);
	float32 GetLength(const Vector3 &startPoint, const Vector3 &endPoint);
	void DrawPoints();
};

#endif /* defined(__RESOURCEEDITORQT__RULERTOOLSYSTEM__) */
