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



#include "UI/UIControlBackground.h"
#include "Debug/DVAssert.h"
#include "UI/UIControl.h"
#include "Core/Core.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

#include <limits>

namespace DAVA
{

const uint16 UIControlBackground::StretchDrawData::indeces[18 * 3] = {
    0, 1, 4,
    1, 5, 4,
    1, 2, 5,
    2, 6, 5,
    2, 3, 6,
    3, 7, 6,
            
    4, 5, 8,
    5, 9, 8,
    5, 6, 9,
    6, 10, 9,
    6, 7, 10,
    7, 11, 10,
            
    8, 9, 12,
    9, 12, 13,
    9, 10, 13,
    10, 14, 13,
    10, 11, 14,
    11, 15, 14
};

UIControlBackground::UIControlBackground()
:	spr(NULL)
,	frame(0)
,	align(ALIGN_HCENTER|ALIGN_VCENTER)
,	type(DRAW_ALIGNED)
,	color(Color::White)
,	drawColor(Color::White)
,	leftStretchCap(0)
,	topStretchCap(0)
,	spriteModification(0)
,	colorInheritType(COLOR_IGNORE_PARENT)
,	perPixelAccuracyType(PER_PIXEL_ACCURACY_DISABLED)
,	lastDrawPos(0, 0)
,	tiledData(NULL)
,   stretchData(NULL)
,   rdoObject(NULL)
,   vertexStream(NULL)
,   texCoordStream(NULL)
,	shader(SafeRetain(RenderManager::TEXTURE_MUL_FLAT_COLOR))
,   margins(NULL)
,   renderState(RenderState::RENDERSTATE_2D_BLEND)
{
}

UIControlBackground *UIControlBackground::Clone()
{
    UIControlBackground *cb = new UIControlBackground();
    cb->CopyDataFrom(this);
    return cb;
}

void UIControlBackground::CopyDataFrom(UIControlBackground *srcBackground)
{
    SafeRelease(spr);
    spr = SafeRetain(srcBackground->spr);
    frame = srcBackground->frame;
    align = srcBackground->align;

    SafeRelease(rdoObject);
    SetDrawType(srcBackground->type);
    SetMargins(srcBackground->GetMargins());

    color = srcBackground->color;
    spriteModification = srcBackground->spriteModification;
    colorInheritType = srcBackground->colorInheritType;
    perPixelAccuracyType = srcBackground->perPixelAccuracyType;
    leftStretchCap = srcBackground->leftStretchCap;
    topStretchCap = srcBackground->topStretchCap;

    SetShader(srcBackground->shader);
}


UIControlBackground::~UIControlBackground()
{
    SafeRelease(rdoObject);
    SafeRelease(spr);
    SafeRelease(shader);
    SafeDelete(margins);
    ReleaseDrawData();
}

bool UIControlBackground::IsEqualTo( const UIControlBackground *back ) const
{
    if (GetDrawType() != back->GetDrawType() ||
        Sprite::GetPathString(GetSprite()) != Sprite::GetPathString(back->GetSprite()) ||
        GetFrame() != back->GetFrame() ||
        GetAlign() != back->GetAlign() ||
        GetColor() != back->GetColor() ||
        GetColorInheritType() != back->GetColorInheritType() ||
        GetModification() != back->GetModification() ||
        GetLeftRightStretchCap() != back->GetLeftRightStretchCap() ||
        GetTopBottomStretchCap() != back->GetTopBottomStretchCap() ||
        GetPerPixelAccuracyType() != back->GetPerPixelAccuracyType() ||
        GetMargins() != back->GetMargins())
        return false;
    return true;
}

Sprite*	UIControlBackground::GetSprite() const
{
    return spr;
}
int32	UIControlBackground::GetFrame() const
{
    return frame;
}
int32	UIControlBackground::GetAlign() const
{
    return align;
}

int32	UIControlBackground::GetModification() const
{
    return spriteModification;
}

UIControlBackground::eColorInheritType UIControlBackground::GetColorInheritType() const
{
    return (eColorInheritType)colorInheritType;
}


UIControlBackground::eDrawType	UIControlBackground::GetDrawType() const
{
    return type;
}


void UIControlBackground::SetSprite(const FilePath &spriteName, int32 drawFrame)
{
    Sprite *tempSpr = Sprite::Create(spriteName);
    SetSprite(tempSpr, drawFrame);
    SafeRelease(tempSpr);
}

void UIControlBackground::SetSprite(Sprite* drawSprite, int32 drawFrame)
{
    if (drawSprite == this->spr)
    {
        // Sprite is not changed - update frame only.
        frame = drawFrame;
        return;
    }

    SafeRelease(spr);
    spr = SafeRetain(drawSprite);
    frame =  drawFrame;
}
void UIControlBackground::SetFrame(int32 drawFrame)
{
    DVASSERT(spr);
    frame = drawFrame;
}

void UIControlBackground::SetAlign(int32 drawAlign)
{
    align = drawAlign;
}
void UIControlBackground::SetDrawType(UIControlBackground::eDrawType drawType)
{
    type = drawType;
    switch(type)
    {
    case DRAW_STRETCH_BOTH:
    case DRAW_STRETCH_HORIZONTAL:
    case DRAW_STRETCH_VERTICAL:
    case DRAW_TILED:
        {
            if (!rdoObject)
            {
                rdoObject = new RenderDataObject();
                vertexStream = rdoObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
                texCoordStream = rdoObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);
                //rdoObject->SetStream()
            }
        }
        break;
    default:
        break;
    }
    ReleaseDrawData();
}

void UIControlBackground::SetModification(int32 modification)
{
    spriteModification = modification;
}

void UIControlBackground::SetColorInheritType(UIControlBackground::eColorInheritType inheritType)
{
    DVASSERT(inheritType >= 0 && inheritType < COLOR_INHERIT_TYPES_COUNT);
    colorInheritType = inheritType;
}

void UIControlBackground::SetPerPixelAccuracyType(ePerPixelAccuracyType accuracyType)
{
    perPixelAccuracyType = accuracyType;
}

UIControlBackground::ePerPixelAccuracyType UIControlBackground::GetPerPixelAccuracyType() const
{
    return perPixelAccuracyType;
}

const Color &UIControlBackground::GetDrawColor() const
{
    return drawColor;
}

void UIControlBackground::SetDrawColor(const Color &c)
{
    drawColor = c;
}

void UIControlBackground::SetParentColor(const Color &parentColor)
{
    switch (colorInheritType)
    {
        case COLOR_MULTIPLY_ON_PARENT:
        {
            drawColor.r = color.r * parentColor.r;
            drawColor.g = color.g * parentColor.g;
            drawColor.b = color.b * parentColor.b;
            drawColor.a = color.a * parentColor.a;
        }
            break;
        case COLOR_ADD_TO_PARENT:
        {
            drawColor.r = Min(color.r + parentColor.r, 1.0f);
            drawColor.g = Min(color.g + parentColor.g, 1.0f);
            drawColor.b = Min(color.b + parentColor.b, 1.0f);
            drawColor.a = Min(color.a + parentColor.a, 1.0f);
        }
            break;
        case COLOR_REPLACE_TO_PARENT:
        {
            drawColor = parentColor;
        }
            break;
        case COLOR_IGNORE_PARENT:
        {
            drawColor = color;
        }
            break;
        case COLOR_MULTIPLY_ALPHA_ONLY:
        {
            drawColor = color;
            drawColor.a = color.a * parentColor.a;
        }
            break;
        case COLOR_REPLACE_ALPHA_ONLY:
        {
            drawColor = color;
            drawColor.a = parentColor.a;
        }
            break;
    }
}

void UIControlBackground::Draw(const UIGeometricData &parentGeometricData)
{
    UIGeometricData geometricData;
    geometricData.size = parentGeometricData.size;
    if (margins)
    {
        geometricData.position = Vector2(margins->left, margins->top);
        geometricData.size += Vector2(-(margins->right + margins->left), -(margins->bottom + margins->top));
    }

    geometricData.AddGeometricData(parentGeometricData);
    Rect drawRect = geometricData.GetUnrotatedRect();

    RenderManager::Instance()->SetColor(drawColor.r, drawColor.g, drawColor.b, drawColor.a);

    Sprite::DrawState drawState;
    drawState.SetRenderState(renderState);
    if (spr)
    {
        drawState.SetShader(shader);
        drawState.frame = frame;
        if (spriteModification)
        {
            drawState.flags = spriteModification;
        }
//		spr->Reset();
//		spr->SetFrame(frame);
//		spr->SetModification(spriteModification);
    }
    switch (type)
    {
        case DRAW_ALIGNED:
        {
            if (!spr)break;
            if(align & ALIGN_LEFT)
            {
                drawState.position.x = drawRect.x;
            }
            else if(align & ALIGN_RIGHT)
            {
                drawState.position.x = drawRect.x + drawRect.dx - spr->GetWidth() * geometricData.scale.x;
            }
            else
            {
                drawState.position.x = drawRect.x + ((drawRect.dx - spr->GetWidth() * geometricData.scale.x) * 0.5f) ;
            }
            if(align & ALIGN_TOP)
            {
                drawState.position.y = drawRect.y;
            }
            else if(align & ALIGN_BOTTOM)
            {
                drawState.position.y = drawRect.y + drawRect.dy - spr->GetHeight() * geometricData.scale.y;
            }
            else
            {
                drawState.position.y = drawRect.y + ((drawRect.dy - spr->GetHeight() * geometricData.scale.y + spr->GetDefaultPivotPoint().y * geometricData.scale.y) * 0.5f) ;
            }
            if(geometricData.angle != 0)
            {
                float tmpX = drawState.position.x;
                drawState.position.x = (tmpX - geometricData.position.x) * geometricData.cosA  + (geometricData.position.y - drawState.position.y) * geometricData.sinA + geometricData.position.x;
                drawState.position.y = (tmpX - geometricData.position.x) * geometricData.sinA  + (drawState.position.y - geometricData.position.y) * geometricData.cosA + geometricData.position.y;
//				spr->SetAngle(geometricData.angle);
                drawState.angle = geometricData.angle;
            }
//			spr->SetPosition(x, y);
            drawState.scale = geometricData.scale;
            drawState.pivotPoint = spr->GetDefaultPivotPoint();
//			spr->SetScale(geometricData.scale);
            //if (drawState.scale.x == 1.0 && drawState.scale.y == 1.0)
            {
                switch(perPixelAccuracyType)
                {
                    case PER_PIXEL_ACCURACY_ENABLED:
                        if(lastDrawPos == drawState.position)
                        {
                            drawState.usePerPixelAccuracy = true;
                        }
                        break;
                    case PER_PIXEL_ACCURACY_FORCED:
                        drawState.usePerPixelAccuracy = true;
                        break;
                    default:
                        break;
                }
            }

            lastDrawPos = drawState.position;

            spr->Draw(&drawState);
        }
        break;

        case DRAW_SCALE_TO_RECT:
        {
            if (!spr)break;

            drawState.position = geometricData.position;
            drawState.flags = spriteModification;
            drawState.scale.x = drawRect.dx / spr->GetSize().dx;
            drawState.scale.y = drawRect.dy / spr->GetSize().dy;
            drawState.pivotPoint.x = geometricData.pivotPoint.x / (geometricData.size.x / spr->GetSize().dx);
            drawState.pivotPoint.y = geometricData.pivotPoint.y / (geometricData.size.y / spr->GetSize().dy);
            drawState.angle = geometricData.angle;
            {
                switch(perPixelAccuracyType)
                {
                case PER_PIXEL_ACCURACY_ENABLED:
                    if(lastDrawPos == drawState.position)
                    {
                        drawState.usePerPixelAccuracy = true;
                    }
                    break;
                case PER_PIXEL_ACCURACY_FORCED:
                    drawState.usePerPixelAccuracy = true;
                    break;
                default:
                    break;
                }
            }

            lastDrawPos = drawState.position;

//			spr->SetPosition(geometricData.position);
//			spr->SetScale(drawRect.dx / spr->GetSize().dx, drawRect.dy / spr->GetSize().dy);
//			spr->SetPivotPoint(geometricData.pivotPoint.x / (geometricData.size.x / spr->GetSize().dx), geometricData.pivotPoint.y / (geometricData.size.y / spr->GetSize().dy));
//			spr->SetAngle(geometricData.angle);

            spr->Draw(&drawState);
        }
        break;

        case DRAW_SCALE_PROPORTIONAL:
        case DRAW_SCALE_PROPORTIONAL_ONE:
        {
            if (!spr)break;
            float32 w, h;
            w = drawRect.dx / (spr->GetWidth() * geometricData.scale.x);
            h = drawRect.dy / (spr->GetHeight() * geometricData.scale.y);
            float ph = spr->GetDefaultPivotPoint().y;

            if(w < h)
            {
                if(type==DRAW_SCALE_PROPORTIONAL_ONE)
                {
                    w = spr->GetWidth() * h * geometricData.scale.y;
                    ph *= h;
                    h = drawRect.dy;
                }
                else
                {
                    h = spr->GetHeight() * w * geometricData.scale.x;
                    ph *= w;
                    w = drawRect.dx;
                }
            }
            else
            {
                if(type==DRAW_SCALE_PROPORTIONAL_ONE)
                {
                    h = spr->GetHeight() * w * geometricData.scale.x;
                    ph *= w;
                    w = drawRect.dx;
                }
                else
                {
                    w = spr->GetWidth() * h * geometricData.scale.y;
                    ph *= h;
                    h = drawRect.dy;
                }
            }

            if(align & ALIGN_LEFT)
            {
                drawState.position.x = drawRect.x;
            }
            else if(align & ALIGN_RIGHT)
            {
                drawState.position.x = (drawRect.x + drawRect.dx - w);
            }
            else
            {
                drawState.position.x = drawRect.x + (int32)((drawRect.dx - w) * 0.5) ;
            }
            if(align & ALIGN_TOP)
            {
                drawState.position.y = drawRect.y;
            }
            else if(align & ALIGN_BOTTOM)
            {
                drawState.position.y = (drawRect.y + drawRect.dy - h);
            }
            else
            {
                drawState.position.y = (drawRect.y) + (int32)((drawRect.dy - h + ph) * 0.5) ;
            }
            drawState.scale.x = w / spr->GetWidth();
            drawState.scale.y = h / spr->GetHeight();
//			spr->SetScaleSize(w, h);
            if(geometricData.angle != 0)
            {
                float32 tmpX = drawState.position.x;
                drawState.position.x = ((tmpX - geometricData.position.x) * geometricData.cosA  + (geometricData.position.y - drawState.position.y) * geometricData.sinA + geometricData.position.x);
                drawState.position.y = ((tmpX - geometricData.position.x) * geometricData.sinA  + (drawState.position.y - geometricData.position.y) * geometricData.cosA + geometricData.position.y);
                drawState.angle = geometricData.angle;
//				spr->SetAngle(geometricData.angle);
            }
//			spr->SetPosition((float32)x, (float32)y);
            {
                switch(perPixelAccuracyType)
                {
                case PER_PIXEL_ACCURACY_ENABLED:
                    if(lastDrawPos == drawState.position)
                    {
                        drawState.usePerPixelAccuracy = true;
                    }
                    break;
                case PER_PIXEL_ACCURACY_FORCED:
                    drawState.usePerPixelAccuracy = true;
                    break;
                default:
                    break;
                }
            }

            lastDrawPos = drawState.position;

            spr->Draw(&drawState);
        }
        break;

        case DRAW_FILL:
        {
            RenderManager::Instance()->SetTextureState(RenderState::TEXTURESTATE_EMPTY);
            DrawFilled( geometricData, drawState.GetRenderState() );
        }
        break;

        case DRAW_STRETCH_BOTH:
        case DRAW_STRETCH_HORIZONTAL:
        case DRAW_STRETCH_VERTICAL:
            DrawStretched(geometricData, drawState.GetRenderState());
        break;

        case DRAW_TILED:
            DrawTiled(geometricData, drawState.GetRenderState());
        break;
        default:
            break;
    }

    RenderManager::Instance()->ResetColor();

}

void UIControlBackground::DrawStretched(const UIGeometricData &geometricData, UniqueHandle renderState)
{
    DVASSERT(rdoObject);
    if (!spr)return;
    if (!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
    {
        return;
    }

    const Vector2 &size = geometricData.size;

    if (leftStretchCap < 0.0f || topStretchCap < 0.0f ||
        size.x <= 0.0f || size.y <= 0.0f)
        return;

    Vector2 stretchCap(Min(size.x, spr->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH)),
                       Min(size.y, spr->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT)));

    stretchCap.x = Min(stretchCap.x * 0.5f, leftStretchCap);
    stretchCap.y = Min(stretchCap.y * 0.5f, topStretchCap);

    UniqueHandle textureHandle = spr->GetTextureHandle(frame);
    
    bool needGenerateData = false;
    if( !stretchData )
    {
        stretchData = new StretchDrawData();
        needGenerateData = true;
    }
    else
    {
        needGenerateData |= spr != stretchData->sprite;
        needGenerateData |= frame != stretchData->frame;
        needGenerateData |= geometricData.size != stretchData->size;
        needGenerateData |= type != stretchData->type;
        needGenerateData |= stretchCap != stretchData->stretchCap;
    }
    
    StretchDrawData &sd = *stretchData;
    
    if( needGenerateData )
    {
        sd.sprite = spr;
        sd.frame = frame;
        sd.size = geometricData.size;
        sd.type = type;
        sd.stretchCap = stretchCap;
        sd.GenerateStretchData();
    }
    
    Matrix3 transformMatr;
    geometricData.BuildTransformMatrix( transformMatr );
    
    if( needGenerateData || sd.transformMatr != transformMatr )
    {
        sd.transformMatr = transformMatr;
        sd.GenerateTransformData();
    }

    vertexStream->Set(TYPE_FLOAT, 2, 0, &sd.transformedVertices[0]);
    texCoordStream->Set(TYPE_FLOAT, 2, 0, &sd.texCoords[0]);

    RenderManager::Instance()->SetTextureState(textureHandle);
    RenderManager::Instance()->SetRenderState(renderState);
    RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(rdoObject);
    RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, sd.GetVertexInTrianglesCount(), EIF_16, (void*)sd.indeces);

    /*GLenum glErr = glGetError();
    if (glErr != GL_NO_ERROR)
    {
        Logger::FrameworkDebug("GLError: 0x%x", glErr);
    }*/
}

void UIControlBackground::ReleaseDrawData()
{
    SafeDelete(tiledData);
    SafeDelete(stretchData);
}

void UIControlBackground::DrawTiled(const UIGeometricData &gd, UniqueHandle renderState)
{
    DVASSERT(rdoObject);
    if (!spr)return;
    if (!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
    {
        return;
    }

    const Vector2 &size = gd.size;

    if( leftStretchCap < 0.0f || topStretchCap < 0.0f ||
        size.x <= 0.0f || size.y <= 0.0f )
        return;

    Vector2 stretchCap( Min( size.x, spr->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH) ),
                        Min( size.y, spr->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT) ) );

    UniqueHandle textureHandle = spr->GetTextureHandle(frame);

    stretchCap.x = Min( stretchCap.x * 0.5f, leftStretchCap );
    stretchCap.y = Min( stretchCap.y * 0.5f, topStretchCap );

    bool needGenerateData = false;
    if( !tiledData )
    {
        tiledData = new TiledDrawData();
        needGenerateData = true;
    }
    else
    {
        needGenerateData |= stretchCap != tiledData->stretchCap;
        needGenerateData |= frame != tiledData->frame;
        needGenerateData |= spr != tiledData->sprite;
        needGenerateData |= size != tiledData->size;
    }

    TiledDrawData &td = *tiledData;

    if( needGenerateData )
    {
        td.stretchCap = stretchCap;
        td.size = size;
        td.frame = frame;
        td.sprite = spr;
        td.GenerateTileData();
    }

    Matrix3 transformMatr;
    gd.BuildTransformMatrix( transformMatr );

    if( needGenerateData || td.transformMatr != transformMatr )
    {
        td.transformMatr = transformMatr;
        td.GenerateTransformData();
    }

    if (td.vertices.empty())
        return;

    vertexStream->Set(TYPE_FLOAT, 2, 0, &td.transformedVertices[0]);
    texCoordStream->Set(TYPE_FLOAT, 2, 0, &td.texCoords[0]);

    RenderManager::Instance()->SetTextureState(textureHandle);
    RenderManager::Instance()->SetRenderState(renderState);
    RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(rdoObject);
    RenderManager::Instance()->DrawElements(PRIMITIVETYPE_TRIANGLELIST, td.indeces.size(), EIF_16, &td.indeces[0]);
}

void UIControlBackground::DrawFilled( const UIGeometricData &gd, UniqueHandle renderState )
{
    if( gd.angle != 0.0f )
    {
        Polygon2 poly;
        gd.GetPolygon( poly );

        RenderHelper::Instance()->FillPolygon( poly, renderState );
    }
    else
    {
        RenderHelper::Instance()->FillRect( gd.GetUnrotatedRect(), renderState );
    }
}

void UIControlBackground::SetLeftRightStretchCap(float32 _leftStretchCap)
{
    leftStretchCap = _leftStretchCap;
}

void UIControlBackground::SetTopBottomStretchCap(float32 _topStretchCap)
{
    topStretchCap = _topStretchCap;
}

float32 UIControlBackground::GetLeftRightStretchCap() const
{
    return leftStretchCap;
}

float32 UIControlBackground::GetTopBottomStretchCap() const
{
    return topStretchCap;
}

uint32 UIControlBackground::StretchDrawData::GetVertexInTrianglesCount() const
{
    switch(type)
    {
        case DRAW_STRETCH_HORIZONTAL:
        case DRAW_STRETCH_VERTICAL:
                return 18;
        case DRAW_STRETCH_BOTH:
                return 18 * 3;
        default:
            DVASSERT(0);
            return 0;
    }
}

void UIControlBackground::StretchDrawData::GenerateTransformData()
{
    for( uint32 index = 0; index < vertices.size(); ++index )
    {
        transformedVertices[index] = vertices[index] * transformMatr;
    }
}
    
void UIControlBackground::StretchDrawData::GenerateStretchData()
{
    const Vector2 sizeInTex(sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH), sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT));
    const Vector2 offsetInTex(sprite->GetRectOffsetValueForFrame(frame, Sprite::X_OFFSET_TO_ACTIVE), sprite->GetRectOffsetValueForFrame(frame, Sprite::Y_OFFSET_TO_ACTIVE));
    const Vector2 spriteSize(sprite->GetWidth(), sprite->GetHeight());

    const Vector2 xyLeftTopCap(offsetInTex - stretchCap);
    const Vector2 xyRightBottomCap(spriteSize - sizeInTex - offsetInTex - stretchCap);

    const Vector2 xyRealLeftTopCap(Max(0.0f, -xyLeftTopCap.x), Max(0.0f, -xyLeftTopCap.y));
    const Vector2 xyRealRightBottomCap(Max(0.0f, -xyRightBottomCap.x), Max(0.0f, -xyRightBottomCap.y));

    const Vector2 xyNegativeLeftTopCap(Max(0.0f, xyLeftTopCap.x), Max(0.0f, xyLeftTopCap.y));

    const Vector2 scaleFactor = (size - stretchCap*2.0f) / (spriteSize - stretchCap*2.0f);

    Vector2 xyPos;
    Vector2 xySize;

    if (DRAW_STRETCH_BOTH == type || DRAW_STRETCH_HORIZONTAL == type)
    {
        xySize.x = xyRealLeftTopCap.x + xyRealRightBottomCap.x + (sizeInTex.x - xyRealLeftTopCap.x - xyRealRightBottomCap.x) * scaleFactor.x;
        xyPos.x = stretchCap.x + xyNegativeLeftTopCap.x * scaleFactor.x - xyRealLeftTopCap.x;
    }
    else
    {
        xySize.x = sizeInTex.x;
        xyPos.x = offsetInTex.x + (size.x - spriteSize.x) * 0.5f;
    }

    if (DRAW_STRETCH_BOTH == type || DRAW_STRETCH_VERTICAL == type)
    {
        xySize.y = xyRealLeftTopCap.y + xyRealRightBottomCap.y + (sizeInTex.y - xyRealLeftTopCap.y - xyRealRightBottomCap.y) * scaleFactor.y;
        xyPos.y = stretchCap.y + xyNegativeLeftTopCap.y * scaleFactor.y - xyRealLeftTopCap.y;
    }
    else
    {
        xySize.y = sizeInTex.y;
        xyPos.y = offsetInTex.y + (size.y - spriteSize.y) * 0.5f;
    }

    const Texture* texture = sprite->GetTexture(frame);
    const float32 resMulFactor = 1.0f / Core::Instance()->GetResourceToVirtualFactor(sprite->GetResourceSizeIndex());
    const Vector2 textureSize((float32)texture->GetWidth(), (float32)texture->GetHeight());

    const Vector2 uvPos(sprite->GetRectOffsetValueForFrame(frame, Sprite::X_POSITION_IN_TEXTURE) / textureSize.x,
                        sprite->GetRectOffsetValueForFrame(frame, Sprite::Y_POSITION_IN_TEXTURE) / textureSize.y);

    const Vector2 uvSize(sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH) * resMulFactor / textureSize.x,
                         sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT) * resMulFactor / textureSize.y);

    const Vector2 uvLeftTopCap(xyRealLeftTopCap * resMulFactor / textureSize);

    const Vector2 uvRightBottomCap(xyRealRightBottomCap * resMulFactor / textureSize);

    switch (type)
    {
        case DRAW_STRETCH_HORIZONTAL:
        {
            vertices.resize(8);
            transformedVertices.resize(8);
            texCoords.resize(8);
            
            vertices[0] = Vector2(xyPos.x, xyPos.y);
            vertices[1] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y);
            vertices[2] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y);
            vertices[3] = Vector2(xyPos.x + xySize.x, xyPos.y);
            
            vertices[4] = Vector2(xyPos.x, xyPos.y + xySize.y);
            vertices[5] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y + xySize.y);
            vertices[6] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y + xySize.y);
            vertices[7] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y);
            
            texCoords[0] = Vector2(uvPos.x, uvPos.y);
            texCoords[1] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y);
            texCoords[2] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y);
            texCoords[3] = Vector2(uvPos.x + uvSize.x, uvPos.y);
            
            texCoords[4] = Vector2(uvPos.x, uvPos.y + uvSize.y);
            texCoords[5] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y + uvSize.y);
            texCoords[6] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y + uvSize.y);
            texCoords[7] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y);
        }
        break;
        case DRAW_STRETCH_VERTICAL:
        {
            vertices.resize(8);
            transformedVertices.resize(8);
            texCoords.resize(8);
            
            vertices[0] = Vector2(xyPos.x, xyPos.y);
            vertices[1] = Vector2(xyPos.x, xyPos.y + xyRealLeftTopCap.y);
            vertices[2] = Vector2(xyPos.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
            vertices[3] = Vector2(xyPos.x, xyPos.y + xySize.y);
            
            vertices[4] = Vector2(xyPos.x + xySize.x, xyPos.y);
            vertices[5] = Vector2(xyPos.x + xySize.x, xyPos.y + xyRealLeftTopCap.y);
            vertices[6] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
            vertices[7] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y);
            
            texCoords[0] = Vector2(uvPos.x, uvPos.y);
            texCoords[1] = Vector2(uvPos.x, uvPos.y + uvLeftTopCap.y);
            texCoords[2] = Vector2(uvPos.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
            texCoords[3] = Vector2(uvPos.x, uvPos.y + uvSize.y);
            
            texCoords[4] = Vector2(uvPos.x + uvSize.x, uvPos.y);
            texCoords[5] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvLeftTopCap.y);
            texCoords[6] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
            texCoords[7] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y);
        }
        break;
        case DRAW_STRETCH_BOTH:
        {
            vertices.resize(16);
            transformedVertices.resize(16);
            texCoords.resize(16);
            
            vertices[0] = Vector2(xyPos.x, xyPos.y);
            vertices[1] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y);
            vertices[2] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y);
            vertices[3] = Vector2(xyPos.x + xySize.x, xyPos.y);
            
            vertices[4] = Vector2(xyPos.x, xyPos.y + xyRealLeftTopCap.y);
            vertices[5] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y + xyRealLeftTopCap.y);
            vertices[6] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y + xyRealLeftTopCap.y);
            vertices[7] = Vector2(xyPos.x + xySize.x, xyPos.y + xyRealLeftTopCap.y);
            
            vertices[8] = Vector2(xyPos.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
            vertices[9] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
            vertices[10] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
            vertices[11] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y - xyRealRightBottomCap.y);
            
            vertices[12] = Vector2(xyPos.x, xyPos.y + xySize.y);
            vertices[13] = Vector2(xyPos.x + xyRealLeftTopCap.x, xyPos.y + xySize.y);
            vertices[14] = Vector2(xyPos.x + xySize.x - xyRealRightBottomCap.x, xyPos.y + xySize.y);
            vertices[15] = Vector2(xyPos.x + xySize.x, xyPos.y + xySize.y);
            
            texCoords[0] = Vector2(uvPos.x, uvPos.y);
            texCoords[1] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y);
            texCoords[2] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y);
            texCoords[3] = Vector2(uvPos.x + uvSize.x, uvPos.y);
            
            texCoords[4] = Vector2(uvPos.x, uvPos.y + uvLeftTopCap.y);
            texCoords[5] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y + uvLeftTopCap.y);
            texCoords[6] = Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y + uvLeftTopCap.y);
            texCoords[7] = Vector2(uvPos.x + uvSize.x, uvPos.y + uvLeftTopCap.y);
            
            texCoords[8] = Vector2(uvPos.x, uvPos.y + uvSize.y - uvRightBottomCap.y );
            texCoords[9] = Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y + uvSize.y - uvRightBottomCap.y );
            texCoords[10]= Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
            texCoords[11]= Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y - uvRightBottomCap.y);
            
            texCoords[12]= Vector2(uvPos.x, uvPos.y + uvSize.y);
            texCoords[13]= Vector2(uvPos.x + uvLeftTopCap.x, uvPos.y + uvSize.y);
            texCoords[14]= Vector2(uvPos.x + uvSize.x - uvRightBottomCap.x, uvPos.y + uvSize.y);
            texCoords[15]= Vector2(uvPos.x + uvSize.x, uvPos.y + uvSize.y);
        }
        break;
    }
}
    
void UIControlBackground::TiledDrawData::GenerateTileData()
{
    Texture *texture = sprite->GetTexture(frame);

    Vector< Vector3 > cellsWidth;
    GenerateAxisData( size.x, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_WIDTH), (float32)texture->GetWidth() * sprite->GetResourceToVirtualFactor(), stretchCap.x, cellsWidth );

    Vector< Vector3 > cellsHeight;
    GenerateAxisData( size.y, sprite->GetRectOffsetValueForFrame(frame, Sprite::ACTIVE_HEIGHT), (float32)texture->GetHeight() * sprite->GetResourceToVirtualFactor(), stretchCap.y, cellsHeight );

    int32 vertexCount = 4 * cellsHeight.size() * cellsWidth.size();
    if (vertexCount>= std::numeric_limits<uint16>::max())
    {
        vertices.clear();
        transformedVertices.clear();
        texCoords.clear();
        Logger::Error("[TiledDrawData::GenerateTileData] tile background too big!");
        return;
    }
    vertices.resize( vertexCount );
    transformedVertices.resize( vertexCount );
    texCoords.resize( vertexCount );

    int32 indecesCount = 6 * cellsHeight.size() * cellsWidth.size();
    indeces.resize( indecesCount );

    int32 offsetIndex = 0;
    const float32 * textCoords = sprite->GetTextureCoordsForFrame(frame);
    Vector2 trasformOffset;
    const Vector2 tempTexCoordsPt( textCoords[0], textCoords[1] );
    for( uint32 row = 0; row < cellsHeight.size(); ++row )
    {
        Vector2 cellSize( 0.0f, cellsHeight[row].x );
        Vector2 texCellSize( 0.0f, cellsHeight[row].y );
        Vector2 texTrasformOffset( 0.0f, cellsHeight[row].z );
        trasformOffset.x = 0.0f;

        for( uint32 column = 0; column < cellsWidth.size(); ++column, ++offsetIndex )
        {
            cellSize.x = cellsWidth[column].x;
            texCellSize.x = cellsWidth[column].y;
            texTrasformOffset.x = cellsWidth[column].z;

            int32 vertIndex = offsetIndex*4;
            vertices[vertIndex + 0] = trasformOffset;
            vertices[vertIndex + 1] = trasformOffset + Vector2( cellSize.x, 0.0f );
            vertices[vertIndex + 2] = trasformOffset + Vector2( 0.0f, cellSize.y );
            vertices[vertIndex + 3] = trasformOffset + cellSize;

            const Vector2 texel = tempTexCoordsPt + texTrasformOffset;
            texCoords[vertIndex + 0] = texel;
            texCoords[vertIndex + 1] = texel + Vector2( texCellSize.x, 0.0f );
            texCoords[vertIndex + 2] = texel + Vector2( 0.0f, texCellSize.y );
            texCoords[vertIndex + 3] = texel + texCellSize;

            int32 indecesIndex = offsetIndex*6;
            indeces[indecesIndex + 0] = vertIndex;
            indeces[indecesIndex + 1] = vertIndex + 1;
            indeces[indecesIndex + 2] = vertIndex + 2;

            indeces[indecesIndex + 3] = vertIndex + 1;
            indeces[indecesIndex + 4] = vertIndex + 3;
            indeces[indecesIndex + 5] = vertIndex + 2;

            trasformOffset.x += cellSize.x;
        }
        trasformOffset.y += cellSize.y;
    }
}

void UIControlBackground::TiledDrawData::GenerateAxisData( float32 size, float32 spriteSize, float32 textureSize, float32 stretchCap, Vector< Vector3 > &axisData )
{
    int32 gridSize = 0;

    float32 sideSize = stretchCap;
    float32 sideTexSize = sideSize / textureSize;

    float32 centerSize = spriteSize - sideSize * 2.0f;
    float32 centerTexSize = centerSize / textureSize;

    float32 partSize = 0.0f;

    if( centerSize > 0.0f )
    {
        gridSize = (int32)ceilf( ( size - sideSize * 2.0f ) / centerSize );
        const float32 tileAreaSize = size - sideSize * 2.0f;
        partSize = tileAreaSize - floorf( tileAreaSize / centerSize ) * centerSize;
    }

    if( sideSize > 0.0f )
        gridSize += 2;

      axisData.resize( gridSize );

    int32 beginOffset = 0;
    int32 endOffset = 0;
    if( sideSize > 0.0f )
    {
        axisData.front() = Vector3( sideSize, sideTexSize, 0.0f );
        axisData.back() = Vector3( sideSize, sideTexSize, sideTexSize + centerTexSize );
        beginOffset = 1;
        endOffset = 1;
    }

    if( partSize > 0.0f )
    {
        ++endOffset;
        const int32 index = gridSize - endOffset;
        axisData[index].x = partSize;
        axisData[index].y = partSize / textureSize;
        axisData[index].z = sideTexSize;
    }

    if( centerSize > 0.0f )
    {
        std::fill( axisData.begin() + beginOffset, axisData.begin() + gridSize - endOffset, Vector3( centerSize, centerTexSize, sideTexSize ) );
    }
}

void UIControlBackground::TiledDrawData::GenerateTransformData()
{
    for( uint32 index = 0; index < vertices.size(); ++index )
    {
        transformedVertices[index] = vertices[index] * transformMatr;
    }
}

void UIControlBackground::SetShader(Shader *_shader)
{
    if(shader != _shader)
    {
        SafeRelease(shader);
        shader = SafeRetain(_shader);
    }
}

void UIControlBackground::SetMargins(const UIMargins* uiMargins)
{
    if (!uiMargins || uiMargins->empty())
    {
        SafeDelete(margins);
        return;
    }

    if (!margins)
    {
        margins = new UIControlBackground::UIMargins();
    }

    *margins = *uiMargins;
}

};
