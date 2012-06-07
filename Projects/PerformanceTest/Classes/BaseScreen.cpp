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

    Revision History:
        * Created by Ivan "Dizz" Petrochenko
=====================================================================================*/

#include "BaseScreen.h"
#include "GameCore.h"

int32 BaseScreen::globalScreenId = 1;

BaseScreen::BaseScreen(const String & _screenName, int32 skipBeforeTests)
    :   UIScreen()
{
	SetName(_screenName);
    
    skipCount = skipBeforeTests;
    skipCounter = 0;
    readyForTests = false;

    currentScreenId = globalScreenId++;
    GameCore::Instance()->RegisterScreen(this);
}

BaseScreen::BaseScreen()
    :   UIScreen()
{
    SetName("Dummy");

    skipCount = 10;
    skipCounter = 0;
    readyForTests = false;

    
    currentScreenId = globalScreenId++;
    GameCore::Instance()->RegisterScreen(this);
}

int32 BaseScreen::GetScreenId()
{
    return currentScreenId;
}

void BaseScreen::WillAppear()
{
    skipCounter = 0;
    readyForTests = (skipCounter == skipCount);
}


void BaseScreen::DidAppear()
{
    skipCounter = 0;
    readyForTests = (skipCounter == skipCount);
}

bool BaseScreen::ReadyForTests()
{
    return readyForTests;
}

void BaseScreen::Update(float32 timeElapsed)
{
    if(!readyForTests)
    {
        ++skipCounter;
        if(skipCount == skipCounter)
        {
            readyForTests = true;
        }
    }
    
    UIScreen::Update(timeElapsed);
}