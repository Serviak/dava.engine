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

#include "Base/BaseTypes.h"

#if defined(__DAVAENGINE_WINDOWS_DESKTOP__)

#include "Render/Cursor.h"
#include "Platform/TemplateWin32/CoreWin32PlatformBase.h"

void DAVA::Cursor::SetCursorPinning(bool pin)
{
    static DAVA::Point2i lastCursorPosition;

    ShowSystemCursor(!pin);

    CoreWin32PlatformBase * winCore = static_cast<CoreWin32PlatformBase *>(Core::Instance());
    if (pin)
    {
        lastCursorPosition = winCore->GetCursorPosition();
        winCore->SetCursorPositionCenter();
    }
    else
    {
        winCore->SetCursorPosition(lastCursorPosition);
    }
}

void DAVA::Cursor::ShowSystemCursor(bool show)
{
    ShowCursor(show);
}

#endif

#if defined(__DAVAENGINE_WIN32__) && defined (__DAVAENGINE_DIRECTX9__)

#include "FileSystem/FileSystem.h"
#include "Render/RenderManager.h"
#include "Render/D3D9Helpers.h"

namespace DAVA
{

Cursor * Cursor::Create(const String & cursorPathname, const Vector2 & hotSpot)
{
	//const String & realPath = FileSystem::Instance()->SystemPathForFrameworkPath(cursorPathname);
	Texture * cursorTexture = Texture::CreateFromFile(cursorPathname);
	if (!cursorTexture)return 0;

	Cursor * cursor = new Cursor();
	cursor->cursorTexture = cursorTexture;
	cursor->hotSpot = hotSpot;

	//Sprite * cursorSprite = Sprite::Create("~res:/Gfx/upgrade/list");//Sprite::CreateFromTexture(cursorTexture, 0, 0, cursorTexture->GetWidth(), cursorTexture->GetHeight());
	Sprite * cursorSprite = Sprite::CreateFromTexture(cursorTexture, 0, 0, (float32)cursorTexture->GetWidth(), (float32)cursorTexture->GetHeight());
	cursor->cursorSprite = cursorSprite;
	cursorSprite->SetDefaultPivotPoint(hotSpot);
	cursorSprite->Reset();
	return cursor;
}

Cursor::Cursor()
: show(true)
{
	cursorSprite = 0;
	cursorTexture = 0;
}
	
Cursor::~Cursor()
{
	SafeRelease(cursorSprite);
	SafeRelease(cursorTexture);
}

void Cursor::SoftwareDraw(const Vector2 & pos)
{
	if(show)
	{
		cursorSprite->SetPosition(pos);
		cursorSprite->Draw();
	}
}

void Cursor::HardwareSet()
{
	/*
		
	*/
	if (RenderManager::Instance()->GetCaps().isHardwareCursorSupported)
	{
		//SetCursor(0);
		//ShowCursor(false);
		cursorTexture->SetAsHardwareCursor(hotSpot);
		LPDIRECT3DDEVICE9 device = RenderManager::Instance()->GetD3DDevice();
		device->ShowCursor(true);
	}
}

void Cursor::Show(bool _show)
{
	show = _show;
	LPDIRECT3DDEVICE9 device = RenderManager::Instance()->GetD3DDevice();
	device->ShowCursor(show);
}

DAVA::Vector2 Cursor::GetPosition()
{
	POINT ptCursor;
	GetCursorPos( &ptCursor );
	ScreenToClient(RenderManager::Instance()->hWnd, &ptCursor);

	return Vector2((float32)ptCursor.x, (float32)ptCursor.y);
}


};
#else 

#endif