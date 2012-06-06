/*==================================================================================
    Copyright (c) 2012, DAVA Consulting, LLC
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
        * Created by Dmitry Shpakov 
=====================================================================================*/

#ifndef __DAVAENGINE_TOUCH_ACTION_H__
#define __DAVAENGINE_TOUCH_ACTION_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/Action.h"

namespace DAVA
{

class TouchAction : public Action
{
public:
    TouchAction(int32 _id);
    virtual ~TouchAction();

protected:
    void TouchDown(const Vector2 &point);
    void TouchDown(const Vector<String> &controlPath);
    void TouchUp();
    void TouchMove(const Vector2 &point);

    Vector2 GetPhysPoint(const Vector2 &p);

    int32 id;
};

class TouchDownAction : public TouchAction
{
public:
    TouchDownAction(const Vector2 &_point, int32 _id);
    virtual ~TouchDownAction();

    virtual void Execute();
protected:
    Vector2 point;
};

class TouchDownControlAction : public TouchAction
{
public:
    TouchDownControlAction(const String &_controlName, int32 _id);
    TouchDownControlAction(const Vector<String> &_controlPath, int32 _id);
    virtual ~TouchDownControlAction();

    virtual void Execute();
protected:
    Vector<String> controlPath;
};

class TouchUpAction : public TouchAction
{
public:
    TouchUpAction(int32 id);
    virtual ~TouchUpAction();

    virtual void Execute();
};

class TouchMoveAction : public TouchAction
{
public:
    TouchMoveAction(const Vector2 &_point, float32 _moveTime, int32 _id);
    virtual ~TouchMoveAction();

    virtual void Execute();
    virtual void Update(float32 timeElapsed);

protected:
    virtual bool TestCondition();
    Vector2 point;
    float32 moveTime;
};

class TouchMoveControlAction : public TouchMoveAction
{
public:
    TouchMoveControlAction(const String &_controlName, float32 _moveTime, int32 _id);
    TouchMoveControlAction(const Vector<String> &_controlPath, float32 _moveTime, int32 _id);
    virtual ~TouchMoveControlAction();

    virtual void Execute();
protected:
    Vector<String> controlPath;
};

class ScrollControlAction : public WaitAction
{
public:
    ScrollControlAction(const String &_controlName, int32 _id, float32 timeout);
    ScrollControlAction(const Vector<String> &_controlPath, int32 _id, float32 timeout);
    virtual ~ScrollControlAction();

    virtual void Update(float32 timeElapsed);
    virtual void Execute();
protected:
    virtual bool TestCondition();
    void FindScrollPoints();

    bool isFound;
    Action* currentAction;
    Deque<Action*> actions;

    Vector2 touchDownPoint;
    Vector2 touchUpPoint;

    Vector<String> controlPath;
    int32 id;
};

};

#endif //__DAVAENGINE_AUTOTESTING__

#endif //__DAVAENGINE_TOUCH_TEST_H__