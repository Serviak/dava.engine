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



#include "VisibilityToolSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "Scene/SceneEditor2.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/VisibilityToolProxy.h"
#include "Deprecated/EditorConfig.h"
#include "Scene/SceneSignals.h"
#include "Scene/SceneHelper.h"
#include "Commands2/VisibilityToolActions.h"

#include "Render/Material/NMaterialNames.h"

VisibilityToolSystem::VisibilityToolSystem(Scene* scene)
:	LandscapeEditorSystem(scene, "~res:/LandscapeEditor/Tools/cursor/cursor.tex")
,	curToolSize(0)
,	editingIsEnabled(false)
,	originalImage(NULL)
,	state(VT_STATE_NORMAL)
,	textureLevel(Landscape::TEXTURE_TILE_FULL)
{
    cursorSize = 120;

	crossTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/setPointCursor.tex");
    DAVA::Function<void()> fn = DAVA::Bind(DAVA::MakeFunction(crossTexture, &DAVA::Texture::SetWrapMode), Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);
    DAVA::JobManager::Instance()->CreateMainJob(fn);
}

VisibilityToolSystem::~VisibilityToolSystem()
{
	SafeRelease(crossTexture);
}

LandscapeEditorDrawSystem::eErrorType VisibilityToolSystem::EnableLandscapeEditing()
{
	if (enabled)
	{
		return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
	}
	
	LandscapeEditorDrawSystem::eErrorType canBeEnabledError = IsCanBeEnabled();
	if ( canBeEnabledError!= LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return canBeEnabledError;
	}
	
	LandscapeEditorDrawSystem::eErrorType enableCustomDrawError = drawSystem->EnableCustomDraw();
	if (enableCustomDrawError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		return enableCustomDrawError;
	}

	SetState(VT_STATE_NORMAL);
	
	selectionSystem->SetLocked(true);
	modifSystem->SetLocked(true);

	Texture* visibilityToolTexture = drawSystem->GetVisibilityToolProxy()->GetSprite()->GetTexture();
	drawSystem->GetLandscapeProxy()->SetVisibilityCheckToolTexture(visibilityToolTexture);
	drawSystem->GetLandscapeProxy()->SetVisibilityCheckToolTextureEnabled(true);
	landscapeSize = visibilityToolTexture->GetWidth();

	drawSystem->EnableCursor(landscapeSize);
	drawSystem->SetCursorSize(0);

	PrepareConfig();

	enabled = true;
	return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool VisibilityToolSystem::DisableLandscapeEdititing()
{
	if (!enabled)
	{
		return true;
	}

	SetState(VT_STATE_NORMAL);

	selectionSystem->SetLocked(false);
	modifSystem->SetLocked(false);

	drawSystem->DisableCursor();
	drawSystem->DisableCustomDraw();

	drawSystem->GetLandscapeProxy()->SetVisibilityCheckToolTexture(NULL);
	drawSystem->GetLandscapeProxy()->SetVisibilityCheckToolTextureEnabled(false);

	enabled = false;
	return !enabled;
}

void VisibilityToolSystem::Process(DAVA::float32 timeElapsed)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}

	if (editingIsEnabled && isIntersectsLandscape)
	{
		if (prevCursorPos != cursorPosition)
		{
			prevCursorPos = cursorPosition;
		}
	}
}

void VisibilityToolSystem::Input(DAVA::UIEvent *event)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}

	UpdateCursorPosition();

	if (state != VT_STATE_SET_AREA && state != VT_STATE_SET_POINT)
	{
		return;
	}

	if (UIEvent::PHASE_KEYCHAR == event->phase)
	{
		if (event->tid == DVKEY_ESCAPE)
		{
			SetState(VT_STATE_NORMAL);
		}
	}
	else if (event->tid == UIEvent::BUTTON_1)
	{
		switch(event->phase)
		{
			case UIEvent::PHASE_BEGAN:
				if (isIntersectsLandscape)
				{
					StoreOriginalState();
					editingIsEnabled = true;
				}
				break;

			case UIEvent::PHASE_DRAG:
				break;

			case UIEvent::PHASE_ENDED:
				if (editingIsEnabled)
				{
					if (state == VT_STATE_SET_POINT)
					{
						SetVisibilityPointInternal();
					}
					else if (state == VT_STATE_SET_AREA)
					{
						SetVisibilityAreaInternal();
					}

					CreateUndoPoint();
					editingIsEnabled = false;
				}
				break;
		}
	}
}


void VisibilityToolSystem::ResetAccumulatorRect()
{
	float32 inf = std::numeric_limits<float32>::infinity();
	updatedRectAccumulator = Rect(inf, inf, -inf, -inf);
}

void VisibilityToolSystem::AddRectToAccumulator(const Rect &rect)
{
	updatedRectAccumulator = updatedRectAccumulator.Combine(rect);
}

Rect VisibilityToolSystem::GetUpdatedRect()
{
	Rect r = updatedRectAccumulator;
	drawSystem->ClampToTexture(textureLevel, r);

	return r;
}

void VisibilityToolSystem::SetBrushSize(int32 brushSize)
{
	if (brushSize > 0)
	{
		cursorSize = (uint32)brushSize;

		if (state == VT_STATE_SET_AREA)
		{
			drawSystem->SetCursorSize(cursorSize);
		}
	}
}

void VisibilityToolSystem::StoreOriginalState()
{
	DVASSERT(originalImage == NULL);
	originalImage = drawSystem->GetVisibilityToolProxy()->GetSprite()->GetTexture()->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
	ResetAccumulatorRect();
}

void VisibilityToolSystem::CreateUndoPoint()
{
	SafeRelease(originalImage);
}

void VisibilityToolSystem::PrepareConfig()
{
	EditorConfig* config = EditorConfig::Instance();

	pointsDensity = 10.f;
	VariantType* value = config->GetPropertyDefaultValue("LevelVisibilityDensity");
	if(value && config->GetPropertyValueType("LevelVisibilityDensity") == VariantType::TYPE_FLOAT)
		pointsDensity = value->AsFloat();

	visibilityPointHeight = 2.f;
	areaPointHeights.clear();
	const Vector<String>& heights = config->GetComboPropertyValues("LevelVisibilityPointHeights");
	if(heights.size() != 0)
	{
		if(heights.front() != "none")
		{
			std::sscanf(heights[0].c_str(), "%f", &visibilityPointHeight);

            areaPointHeights.reserve(heights.size() - 1);
			for(uint32 i = 1; i < heights.size(); ++i)
			{
				float32 val;
				std::sscanf(heights[i].c_str(), "%f", &val);
				areaPointHeights.push_back(val);
			}
		}
	}

	areaPointColors = config->GetColorPropertyValues("LevelVisibilityColors");
}

void VisibilityToolSystem::SetState(eVisibilityToolState newState)
{
	if(state == newState)
		state = VT_STATE_NORMAL;
	else
		state = newState;

	switch(state)
	{
		case VT_STATE_SET_POINT:
			drawSystem->SetCursorTexture(crossTexture);
			drawSystem->SetCursorSize(CROSS_TEXTURE_SIZE);
			break;

		case VT_STATE_SET_AREA:
			drawSystem->SetCursorTexture(cursorTexture);
			drawSystem->SetCursorSize(cursorSize);
			break;

		default:
			if (IsLandscapeEditingEnabled())
			{
				drawSystem->SetCursorSize(0);
			}
			break;
	}
	SceneSignals::Instance()->EmitVisibilityToolStateChanged(dynamic_cast<SceneEditor2*>(GetScene()), state);
}

void VisibilityToolSystem::SetVisibilityPoint()
{
	SetState(VT_STATE_SET_POINT);
}

void VisibilityToolSystem::SetVisibilityArea()
{
	SetState(VT_STATE_SET_AREA);
}

void VisibilityToolSystem::SetVisibilityPointInternal()
{
	Sprite* cursorSprite = Sprite::CreateFromTexture(crossTexture, 0, 0,
													 crossTexture->GetWidth(), crossTexture->GetHeight());

	SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
	DVASSERT(scene);
	scene->Exec(new ActionSetVisibilityPoint(originalImage, cursorSprite,
											 drawSystem->GetVisibilityToolProxy(), cursorPosition, Vector2(CROSS_TEXTURE_SIZE, CROSS_TEXTURE_SIZE)));

	SafeRelease(originalImage);
	SafeRelease(cursorSprite);

	SetState(VT_STATE_NORMAL);
}

void VisibilityToolSystem::SetVisibilityAreaInternal()
{
	if (drawSystem->GetVisibilityToolProxy()->IsVisibilityPointSet())
	{
		Vector2 areaSize = Vector2(cursorSize, cursorSize);
		Vector2 areaPos = cursorPosition;// - areaSize / 2;

		Rect updatedRect;
		updatedRect.SetPosition(areaPos - areaSize / 2.f);
		updatedRect.SetSize(areaSize);
		AddRectToAccumulator(updatedRect);

		Vector2 visibilityPoint = drawSystem->GetVisibilityToolProxy()->GetVisibilityPoint();

		Vector3 point(visibilityPoint);
		point.z = drawSystem->GetHeightAtPoint(drawSystem->TexturePointToHeightmapPoint(textureLevel, visibilityPoint));
		point.z += visibilityPointHeight;

		Vector<Vector3> resP;
		PerformHeightTest(point, areaPos, cursorSize / 2.f, pointsDensity, areaPointHeights, &resP);
		DrawVisibilityAreaPoints(resP);

		SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
		DVASSERT(scene);
		scene->Exec(new ActionSetVisibilityArea(originalImage, drawSystem->GetVisibilityToolProxy(), GetUpdatedRect()));

		SafeRelease(originalImage);
	}
	else
	{
		// show "could not check visibility without visibility point" error message
	}
}

void VisibilityToolSystem::PerformHeightTest(Vector3 spectatorCoords,
											 Vector2 circleCenter,
											 float32 circleRadius,
											 float32 density,
											 const Vector<float32>& heightValues,
											 Vector<Vector3>* colorizedPoints)
{
	DVASSERT(colorizedPoints);
	if(heightValues.size() == 0 )
	{
		return;
	}

	Vector2 startOfCounting(circleCenter.x - circleRadius, circleCenter.y - circleRadius);
	Vector2 SpectatorCoords2D(spectatorCoords.x, spectatorCoords.y);

	// get source point in propper coords system
	Vector3 sourcePoint(drawSystem->TexturePointToLandscapePoint(textureLevel, SpectatorCoords2D));

	sourcePoint.z = spectatorCoords.z;

	const uint32	height = heightValues.size();
	const uint32	sideLength = (circleRadius * 2) / density;

    Vector< Vector< Vector< Vector3 > > > points;
    points.resize(sideLength);

    //detect points for ray test
    for(uint32 y = 0; y < sideLength; ++y)
    {
        points[y].resize(sideLength);
        
        const float32 yOfPoint = startOfCounting.y + density * y;
        for(uint32 x = 0; x < sideLength; ++x)
        {
            points[y][x].resize(height);
            
            const float32 xOfPoint = startOfCounting.x + density * x;
            const float32 zOfPoint = drawSystem->GetHeightAtTexturePoint(textureLevel, Vector2(xOfPoint, yOfPoint));
            
            for(uint32 layerIndex = 0; layerIndex < height; ++layerIndex)
            {
                points[y][x][layerIndex] = Vector3(xOfPoint, yOfPoint, zOfPoint + heightValues[layerIndex]);
            }
        }
    }
    
    //Detect intersections in cursor area with landscape and objects
    const float32 textureSize = drawSystem->GetTextureSize(textureLevel);
    const Rect textureRect(Vector2(0.f, 0.f), Vector2(textureSize, textureSize));
    
    Vector<Vector<int32> > colorIndex;
    colorIndex.resize(sideLength);
    
    for(uint32 y = 0; y < sideLength; ++y)
    {
        colorIndex[y].resize(sideLength, 0); //mark all points as intersectable
        for(uint32 x = 0; x < sideLength; ++x)
        {
            const Vector2 xyPoint(points[y][x][0].x, points[y][x][0].y);
            
            if (!IsCircleContainsPoint(circleCenter, circleRadius, xyPoint) || !textureRect.PointInside(xyPoint))
            {
                colorIndex[y][x] = -1; //exclude point from cursor
                continue;
            }
            
            Vector3 target(drawSystem->TexturePointToLandscapePoint(textureLevel, xyPoint));
            for(uint32 layerIndex = 0; layerIndex < height; ++layerIndex)
            {
                target.z = points[y][x][layerIndex].z;
                
                const EntityGroup* intersectedObjects = collisionSystem->ObjectsRayTest(sourcePoint, target);
                EntityGroup entityGroup(*intersectedObjects);
                ExcludeEntities(&entityGroup);
                
				bool wasIntersection = (entityGroup.Size() == 0) ? false : true;
				if (!wasIntersection)
				{
					Vector3 p;
                    wasIntersection = collisionSystem->LandRayTest(sourcePoint, target, p);
				}

                if(!wasIntersection)
                {
                    colorIndex[y][x] = 1 + layerIndex;
                    break;
                }
            }
        }
    }
    
    colorizedPoints->clear();
    for(uint32 y = 0; y < sideLength; ++y)
    {
        for(uint32 x = 0; x < sideLength; ++x)
        {
            if(colorIndex[y][x] != -1)
            {
                Vector3 targetTmp = points[y][x][0];
                targetTmp.z = colorIndex[y][x];
                
                colorizedPoints->push_back(targetTmp);
            }
        }
    }
}

void VisibilityToolSystem::ExcludeEntities(EntityGroup *entities) const
{
    if (!entities || (entities->Size() == 0)) return;
    
    uint32 count = entities->Size();
    while(count)
    {
        Entity *object = entities->GetEntity(count - 1);
        bool needToExclude = false;

        KeyedArchive * customProps = GetCustomPropertiesArchieve(object);
        if(customProps)
        {   // exclude not collised by bullet objects
            const int32 collisiontype = customProps->GetInt32( "CollisionType", 0 );
            if(     (ResourceEditor::ESOT_TREE == collisiontype)
                ||  (ResourceEditor::ESOT_BUSH == collisiontype)
                ||  (ResourceEditor::ESOT_FRAGILE_PROJ_INV == collisiontype)
                ||  (ResourceEditor::ESOT_FALLING == collisiontype)
                ||  (ResourceEditor::ESOT_SPEED_TREE == collisiontype)
                )
            {
                needToExclude = true;
            }
        }
        
        if(!needToExclude)
        {
            RenderObject *ro = GetRenderObject(object);
            if(ro)
            {
                switch (ro->GetType())
                {
                    case RenderObject::TYPE_LANDSCAPE:
                    case RenderObject::TYPE_SKYBOX:
                    case RenderObject::TYPE_SPEED_TREE:
                    case RenderObject::TYPE_SPRITE:
                    case RenderObject::TYPE_VEGETATION:
                    case RenderObject::TYPE_PARTICLE_EMTITTER:
                        needToExclude = true;
                        break;
                        
                    default:
                        break;
                }
            }
        }

        if(!needToExclude)
        {   // exclude sky
            
            Vector<NMaterial *> materials;
            SceneHelper::EnumerateMaterialInstances(object, materials);
            
            const uint32 matCount = materials.size();
            for(uint32 m = 0; m < matCount; ++m)
            {
                const NMaterialTemplate *matTemplate = materials[m]->GetMaterialTemplate();
                if((NMaterialName::SKYOBJECT == matTemplate->name)  || (NMaterialName::SKYBOX == matTemplate->name))
                {
                    needToExclude = true;
                    break;
                }
            }
        }

        if(needToExclude)
        {
            entities->Rem(object);
        }
        
        --count;
    }
}




bool VisibilityToolSystem::IsCircleContainsPoint(const Vector2& circleCenter, float32 circleRadius, const Vector2& point)
{
	return (point - circleCenter).Length() < (circleRadius * 0.9f);
}

void VisibilityToolSystem::DrawVisibilityAreaPoints(const Vector<DAVA::Vector3> &points)
{
	VisibilityToolProxy* visibilityToolProxy = drawSystem->GetVisibilityToolProxy();
	Sprite* visibilityAreaSprite = visibilityToolProxy->GetSprite();

    RenderSystem2D::Instance()->PushRenderTarget();
    RenderSystem2D::Instance()->SetRenderTarget(visibilityAreaSprite);

	for(uint32 i = 0; i < points.size(); ++i)
	{
		uint32 colorIndex = (uint32)points[i].z;
		Vector2 pos(points[i].x, points[i].y);

        RenderManager::Instance()->SetRenderState(RenderState::RENDERSTATE_2D_BLEND);
        RenderManager::Instance()->SetColor(areaPointColors[colorIndex]);
        RenderSystem2D::Instance()->Setup2DMatrices();
        RenderHelper::Instance()->DrawPoint(VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtual(pos), 5.f, DAVA::RenderState::RENDERSTATE_2D_BLEND);
	}

    RenderManager::Instance()->ResetColor();
    RenderSystem2D::Instance()->PopRenderTarget();
}

void VisibilityToolSystem::SaveTexture(const FilePath& filePath)
{
	if (filePath.IsEmpty())
	{
		return;
	}

	Sprite* visibilityToolSprite = drawSystem->GetVisibilityToolProxy()->GetSprite();
	Texture* visibilityToolTexture = visibilityToolSprite->GetTexture();

	Image* image = visibilityToolTexture->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);
    ImageSystem::Instance()->Save(filePath, image);
	SafeRelease(image);
}

VisibilityToolSystem::eVisibilityToolState VisibilityToolSystem::GetState()
{
	return state;
}

int32 VisibilityToolSystem::GetBrushSize()
{
	return cursorSize;
}
