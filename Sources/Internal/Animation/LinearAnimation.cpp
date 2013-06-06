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
#include "Base/BaseMath.h"
#include "Animation/LinearAnimation.h"

namespace DAVA
{
	
	RectLinearAnimation::RectLinearAnimation(AnimatedObject * _owner, Rect * _var, Rect _endValue, float32 _animationTimeLength, Interpolation::FuncType _iType)
	: Animation(_owner, _animationTimeLength, _iType)
	{
		var = _var;
		endValue = _endValue;
	}
	
void RectLinearAnimation::OnStart()
{
	startValue = *var;
}

void RectLinearAnimation::Update(float32 timeElapsed)
{
	Animation::Update(timeElapsed);
	var->x = startValue.x + (endValue.x - startValue.x) * normalizedTime;
	var->y = startValue.y + (endValue.y - startValue.y) * normalizedTime;
	var->dx = startValue.dx + (endValue.dx - startValue.dx) * normalizedTime;
	var->dy = startValue.dy + (endValue.dy - startValue.dy) * normalizedTime;
}

TwoVector2LinearAnimation::TwoVector2LinearAnimation(AnimatedObject * _owner, Vector2 * _var1, Vector2 _endValue1, Vector2 * _var2, Vector2 _endValue2, float32 _animationTimeLength, Interpolation::FuncType _iType)
: Animation(_owner, _animationTimeLength, _iType)
{
	var1 = _var1;
	var2 = _var2;
	endValue1 = _endValue1;
	endValue2 = _endValue2;
}
void TwoVector2LinearAnimation::Update(float32 timeElapsed)
{
	Animation::Update(timeElapsed);
	*var1 = startValue1 + (endValue1 - startValue1) * normalizedTime;
	*var2 = startValue2 + (endValue2 - startValue2) * normalizedTime;
}
void TwoVector2LinearAnimation::OnStart()
{
	startValue1 = *var1;
	startValue2 = *var2;
}
	
};