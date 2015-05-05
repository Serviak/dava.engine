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


#ifndef __DAVAENGINE_RENDER_BASE_H__
#define __DAVAENGINE_RENDER_BASE_H__

#include "Base/BaseTypes.h"
#include "DAVAConfig.h"
#include "Base/FastName.h"
#include "Render/RHI/rhi_Type.h"

/**
	\defgroup render Rendering abstraction
	\defgroup render_2d 2D Rendering
	\defgroup render_3d 3D Rendering
*/

/*
 Boroda: Я не знаю почему но <glew/gl.h> при включение из .mm файлов выдает ошибку как будто кто-то уже подключил 
 стандартный GL хотя в проекте нигде нет на него упоминаний. Есть подозрение что какой-то конфликт с внутренностями ObjC
 Как обойти пока непонятно - я сделал этот хедер чтобы включать его только там где это реально надо. 
 
 Раньше это лежало в BaseTypes.h
 */

#ifdef __DAVAENGINE_IPHONE__
	#define __DAVAENGINE_OPENGL__
	#import <OpenGLES/ES1/gl.h>
	#import <OpenGLES/ES1/glext.h>
    #import <OpenGLES/ES2/gl.h>
    #import <OpenGLES/ES2/glext.h>
    #import <OpenGLES/ES3/gl.h>
    #import <OpenGLES/ES3/glext.h>
#elif defined(__DAVAENGINE_MACOS__)
	#define __DAVAENGINE_OPENGL__
	//	#include <GL/glew.h>
    #ifdef __DAVAENGINE_MACOS_VERSION_10_6__
        #include "OpenGL/gl.h"
    #endif //#ifdef __DAVAENGINE_MACOS_VERSION_10_6__

	#include <OpenGL/OpenGL.h>
	//	#include <GLUT/glut.h>
#elif defined(__DAVAENGINE_WIN32__)
 	#if defined(__DAVAENGINE_RENDER_AUTOCONFIG__)
 	#define __DAVAENGINE_DIRECTX9__
//	#define __DAVAENGINE_OPENGL__
 	#endif 

	#if defined(__DAVAENGINE_OPENGL__)
		#include <GL/glew.h>
		#include <direct.h>
	#elif defined(__DAVAENGINE_DIRECTX9__)
		// TODO: right now Sprite code do not compiles without GL types. Fix that. 
		#include <GL/gl.h>
		// #include <direct.h>
		
		#define D3D_DEBUG_INFO
		#include <d3d9.h>
		#include <dxerr.h>
#endif 
#elif defined(__DAVAENGINE_ANDROID__)
	#define __DAVAENGINE_OPENGL__
    #include <android/api-level.h>
	#include <GLES/gl.h>
	#include <GLES/glext.h>
    #include <EGL/egl.h>
#if (__ANDROID_API__ < 18)
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#else
    #include <GLES3/gl3.h>
    #include <GLES3/gl3ext.h>
#endif

#else //PLATFORMS
	//other platforms
#endif//PLATFORMS 


namespace DAVA
{

//RHI_COMPLETE pure blend mode is deprecated - here just for compilance
enum eBlendMode
{
    BLEND_NONE = 0,				// blend mode not defined
    BLEND_ZERO,
    BLEND_ONE,
    BLEND_DST_COLOR,
    BLEND_ONE_MINUS_DST_COLOR,
    BLEND_SRC_ALPHA,
    BLEND_ONE_MINUS_SRC_ALPHA,
    BLEND_DST_ALPHA,
    BLEND_ONE_MINUS_DST_ALPHA,
    BLEND_SRC_ALPHA_SATURATE,
    BLEND_SRC_COLOR,
    BLEND_ONE_MINUS_SRC_COLOR,

    BLEND_MODE_COUNT,
};

enum PixelFormat
{
    FORMAT_INVALID = 0,
    FORMAT_RGBA8888 = 1,
    FORMAT_RGBA5551,
    FORMAT_RGBA4444,
    FORMAT_RGB888,
    FORMAT_RGB565,
    FORMAT_A8,
    FORMAT_A16,
    FORMAT_PVR4,
    FORMAT_PVR2,
    
    FORMAT_RGBA16161616,
    FORMAT_RGBA32323232,

    FORMAT_DXT1,
    FORMAT_REMOVED_DXT_1N,  //to use it in
    FORMAT_DXT1A = 14, //back compatibility
    FORMAT_DXT3,
    FORMAT_DXT5,
    FORMAT_DXT5NM,
	
	FORMAT_ETC1,
	
	FORMAT_ATC_RGB,
	FORMAT_ATC_RGBA_EXPLICIT_ALPHA,
    FORMAT_ATC_RGBA_INTERPOLATED_ALPHA,
    
	FORMAT_PVR2_2,	//pvrtc2 generation
	FORMAT_PVR4_2,
	FORMAT_EAC_R11_UNSIGNED,
	FORMAT_EAC_R11_SIGNED,
	FORMAT_EAC_RG11_UNSIGNED,	//2 channels format for normal maps
	FORMAT_EAC_RG11_SIGNED,	//2 channels format for normal maps
	FORMAT_ETC2_RGB,
	FORMAT_ETC2_RGBA,
	FORMAT_ETC2_RGB_A1,

    FORMAT_BGR888, // windows BMP format
    FORMAT_BGRA8888, // android web view format only for ImageConvert

    FORMAT_COUNT,
    FORMAT_CLOSEST = 255 // fit PixelFormat at 8bits (PixelFormat format:8;)
};
    
// Please update JniDeviceInfo.java if change eGPUFamily enum
enum eGPUFamily
{
    GPU_POWERVR_IOS     =   0,
    GPU_POWERVR_ANDROID,
    GPU_TEGRA,
    GPU_MALI,
    GPU_ADRENO,
    GPU_PNG,
    GPU_FAMILY_COUNT,
    
    GPU_DEVICE_COUNT = GPU_PNG,
    GPU_INVALID = 0x07
};
      



enum eIndexFormat
{
	EIF_16 = 0x0,
	EIF_32 = 0x1,
};
    
static const int32 INDEX_FORMAT_SIZE[2] = {2, 4};   

const int32 STENCILOP_COUNT = 8; //rhi::StencilOperation
const int32 CMP_TEST_MODE_COUNT = 8; //rhi::CmpFunc
const int32 FILLMODE_COUNT = 3;
extern const String CMP_FUNC_NAMES[CMP_TEST_MODE_COUNT];
extern const String STENCIL_OP_NAMES[STENCILOP_COUNT];     

extern const String FILL_MODE_NAMES[FILLMODE_COUNT];

    
enum ePrimitiveType
{
	PRIMITIVETYPE_POINTLIST = 0,
	PRIMITIVETYPE_LINELIST,
	PRIMITIVETYPE_LINESTRIP,
	PRIMITIVETYPE_TRIANGLELIST,
	PRIMITIVETYPE_TRIANGLESTRIP,
	PRIMITIVETYPE_TRIANGLEFAN,

	PRIMITIVETYPE_COUNT
};

    
// TODO: we have same structs & functions in PolygonGroup -- we should find a right place for them
enum eVertexFormat
{
    EVF_VERTEX          = 1,
    EVF_NORMAL          = 1 << 1,
    EVF_COLOR           = 1 << 2,
    EVF_TEXCOORD0       = 1 << 3,
    EVF_TEXCOORD1       = 1 << 4,
    EVF_TEXCOORD2       = 1 << 5,
    EVF_TEXCOORD3       = 1 << 6,
    EVF_TANGENT         = 1 << 7,
    EVF_BINORMAL        = 1 << 8,
  // nine bit skipped cause legacy; for now it unused
    EVF_TIME            = 1 << 10,
    EVF_PIVOT           = 1 << 11,
    EVF_FLEXIBILITY     = 1 << 12,
    EVF_ANGLE_SIN_COS   = 1 << 13,
    EVF_JOINTINDEX      = 1 << 14,
    EVF_JOINTWEIGHT     = 1 << 15,
    EVF_CUBETEXCOORD0   = 1 << 16,
    EVF_CUBETEXCOORD1   = 1 << 17,
    EVF_CUBETEXCOORD2   = 1 << 18,
    EVF_CUBETEXCOORD3   = 1 << 19,	
    EVF_LOWER_BIT       = EVF_VERTEX,
    EVF_HIGHER_BIT      = EVF_JOINTWEIGHT, 
    EVF_NEXT_AFTER_HIGHER_BIT
    = (EVF_HIGHER_BIT << 1),
    EVF_FORCE_DWORD     = 0x7fffffff,
};
enum
{
    VERTEX_FORMAT_STREAM_MAX_COUNT = 16
};


inline int32 GetTexCoordCount(int32 vertexFormat)
{
	int32 ret = 0;
	for(int32 i = EVF_TEXCOORD0; i < EVF_TEXCOORD3+1; i = (i << 1))
	{
		if(vertexFormat & i)
		{
			ret++;
		}
	}

	return ret;
}
	
inline int32 GetCubeTexCoordCount(int32 vertexFormat)
{
	int32 ret = 0;
	for(int32 i = EVF_CUBETEXCOORD0; i < EVF_CUBETEXCOORD3+1; i = (i << 1))
	{
		if(vertexFormat & i)
		{
			ret++;
		}
	}
		
	return ret;
}

    
    
inline int32 GetVertexSize(int32 flags)
{
    int32 size = 0;
    if (flags & EVF_VERTEX) size += 3 * sizeof(float32);
    if (flags & EVF_NORMAL) size += 3 * sizeof(float32);
    if (flags & EVF_COLOR) size += 4;
    if (flags & EVF_TEXCOORD0) size += 2 * sizeof(float32);
    if (flags & EVF_TEXCOORD1) size += 2 * sizeof(float32);
    if (flags & EVF_TEXCOORD2) size += 2 * sizeof(float32);
    if (flags & EVF_TEXCOORD3) size += 2 * sizeof(float32);
    if (flags & EVF_TANGENT) size += 3 * sizeof(float32);
    if (flags & EVF_BINORMAL) size += 3 * sizeof(float32);
	
	if (flags & EVF_CUBETEXCOORD0) size += 3 * sizeof(float32);
    if (flags & EVF_CUBETEXCOORD1) size += 3 * sizeof(float32);
    if (flags & EVF_CUBETEXCOORD2) size += 3 * sizeof(float32);
    if (flags & EVF_CUBETEXCOORD3) size += 3 * sizeof(float32);

	if (flags & EVF_TIME) size+=sizeof(float32);
	
    if (flags & EVF_PIVOT) size += 3 * sizeof(float32);
    if (flags & EVF_FLEXIBILITY) size += sizeof(float32);
    if (flags & EVF_ANGLE_SIN_COS) size += 2 * sizeof(float32);

    if (flags & EVF_JOINTINDEX) size += 4;
    if (flags & EVF_JOINTWEIGHT) size += 4;

    return size;
}


rhi::CmpFunc GetCmpFuncByName(const String & cmpFuncStr);
rhi::StencilOperation GetStencilOpByName(const String & stencilOpStr);          
    
};

#endif // __DAVAENGINE_RENDER_BASE_H__
