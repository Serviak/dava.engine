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


#ifndef __DAVAENGINE_RENDER_RENDERSYSTEM_2D_H__
#define	__DAVAENGINE_RENDER_RENDERSYSTEM_2D_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Render/RenderBase.h"
#include "Render/2D/Sprite.h"

namespace DAVA
{

class Font;
class Sprite;
class TextBlock;
class RenderDataObject;
class RenderDataStream;
class RenderSystem2D : public Singleton<RenderSystem2D>
{
public:
    static FastName FLAT_COLOR_SHADER;
    static FastName TEXTURE_MUL_FLAT_COLOR_SHADER;

    static Shader * FLAT_COLOR;
    static Shader * TEXTURE_MUL_FLAT_COLOR;
    static Shader * TEXTURE_MUL_FLAT_COLOR_ALPHA_TEST;
    static Shader * TEXTURE_MUL_FLAT_COLOR_IMAGE_A8;

    RenderSystem2D();
    virtual ~RenderSystem2D();
    
    void Init();

    void Draw(Sprite * sprite, Sprite::DrawState * drawState = 0);
    
    void Reset();
    
    void SetClip(const Rect &rect);
	void ClipRect(const Rect &rect);
	void RemoveClip();
    
	void ClipPush();
	void ClipPop();
    
    void ScreenSizeChanged();
    
    void Setup2DMatrices();

private:
    //sprite draw
    void PrepareSpriteRenderData(Sprite * sprite, Sprite::DrawState * drawState);

    Matrix4 viewMatrix;
	std::stack<Rect> clipStack;
	Rect currentClip;
    
	float32 spriteTempVertices[8];
    RenderDataObject * spriteRenderObject;
    RenderDataStream * spriteVertexStream;
	RenderDataStream * spriteTexCoordStream;
	ePrimitiveType spritePrimitiveToDraw;
	int32 spriteVertexCount;
    
    Sprite::DrawState defaultSpriteDrawState;

    Vector<Vector2> spriteClippedTexCoords;
	Vector<Vector2> spriteClippedVertices;
};
    
} // ns

#endif	/* __DAVAENGINE_RENDER_RENDERSYSTEM_2D_H__ */

