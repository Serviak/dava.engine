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


#ifndef __LOGENGINE_MATH2D_H__
#define __LOGENGINE_MATH2D_H__

//!	
//! All 2D & 3D math represent vectors & points (2D eq to vector) as vector array
//! for example 
//!	Math::Point2 < float32 > p; // [x y] (or [x y 1] for 3x3 matrix equations)
//! 

#include <math.h>
#include "BaseTypes.h"
#include "Math2DMatrix2.h"
#include "Math2DMatrix3.h"
#include "Math2DBase.h"



// definition of basic 2D types
namespace Log
{
namespace Math
{

	//! int Point2 
	typedef Point2Base<int32>		Point2i;
	//! float Point2 
	typedef Point2Base<float32>		Point2f;
	//! int Size2
	typedef Size2Base<int32>		Size2i;
	//! float Size2
	typedef Size2Base<float32>		Size2f;
	//! int Rect2
	typedef Rect2Base<int32>		Rect2i;
	//! float Rect2
	typedef Rect2Base<float32>		Rect2f;
};
};

#endif	//__LOGENGINE_MATH2D_H__

