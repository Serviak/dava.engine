#pragma once

#include "../rhi_Type.h"
#include "Debug/Profiler.h"

#if defined(__DAVAENGINE_WIN32__)

    #include "GL/glew.h"
    #include <GL/GL.h>
    #include "GL/wglew.h"
    
    #define GetGLErrorString gluErrorString

    #include "win_gl.h"

#elif defined(__DAVAENGINE_MACOS__)

    #include <Carbon/Carbon.h>
    #include <AGL/agl.h>
    #include <OpenGL/glext.h>
    
    #define GetGLErrorString(code) #code

    #include "macos_gl.h"

#elif defined(__DAVAENGINE_IPHONE__)

    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
    
    #define GetGLErrorString(code) "<unknown>"

    #include "ios_gl.h"

#elif defined(__DAVAENGINE_ANDROID__)

	#include <EGL/egl.h>
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>

    #define GetGLErrorString(code) "<unknown>"

	#include "android_gl.h"

#ifdef __arm__
    #include <machine/cpu-features.h>
    #if __ARM_ARCH__ == 7
        #ifdef USE_NEON
            #define __DAVAENGINE_ARM_7__
        #endif
    #endif
#endif

#else

    #include <GL/GL.h>

#endif

#if defined(__DAVAENGINE_ANDROID__)

typedef void(GL_APIENTRY* PFNGLEGL_GLDRAWELEMENTSINSTANCED)(GLenum, GLsizei, GLenum, const void*, GLsizei);
typedef void(GL_APIENTRY* PFNGLEGL_GLDRAWARRAYSINSTANCED)(GLenum, GLint, GLsizei, GLsizei);
typedef void(GL_APIENTRY* PFNGLEGL_GLVERTEXATTRIBDIVISOR)(GLuint, GLuint);

extern PFNGLEGL_GLDRAWELEMENTSINSTANCED glDrawElementsInstanced;
extern PFNGLEGL_GLDRAWARRAYSINSTANCED glDrawArraysInstanced;
extern PFNGLEGL_GLVERTEXATTRIBDIVISOR glVertexAttribDivisor;

#endif

#if defined(__DAVAENGINE_ANDROID__) || defined(__DAVAENGINE_WIN_UAP__)
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif //GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif //GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#endif //defined(__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_WIN_UAP__)

#if !defined(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0
#endif
#if !defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0
#endif

#if !defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0
#endif

#if !defined(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0
#endif

#if !defined(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0
#endif

#if !defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0
#endif

#if !defined(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0
#endif

#if !defined(GL_ETC1_RGB8_OES)
#define GL_ETC1_RGB8_OES 0
#endif

#if !defined(GL_ATC_RGB_AMD)
#define GL_ATC_RGB_AMD 0
#endif

#if !defined(GL_ATC_RGBA_EXPLICIT_ALPHA_AMD)
#define GL_ATC_RGBA_EXPLICIT_ALPHA_AMD 0
#endif

#if !defined(GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD)
#define GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD 0
#endif

#if !defined(GL_COMPRESSED_R11_EAC)
#define GL_COMPRESSED_R11_EAC 0
#endif

#if !defined(GL_COMPRESSED_SIGNED_R11_EAC)
#define GL_COMPRESSED_SIGNED_R11_EAC 0
#endif

#if !defined(GL_COMPRESSED_RG11_EAC)
#define GL_COMPRESSED_RG11_EAC 0
#endif

#if !defined(GL_COMPRESSED_SIGNED_RG11_EAC)
#define GL_COMPRESSED_SIGNED_RG11_EAC 0
#endif

#if !defined(GL_COMPRESSED_RGB8_ETC2)
#define GL_COMPRESSED_RGB8_ETC2 0
#endif

#if !defined(GL_COMPRESSED_RGBA8_ETC2_EAC)
#define GL_COMPRESSED_RGBA8_ETC2_EAC 0
#endif

#if !defined(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2)
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0
#endif

#if !defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG)
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG 0
#endif

#if !defined(GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG)
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG 0
#endif

#if !defined(GL_HALF_FLOAT_OES)
#define GL_HALF_FLOAT_OES 0
#endif

#if !defined GL_QUERY_RESULT_AVAILABLE_EXT
#define GL_QUERY_RESULT_AVAILABLE_EXT 0
#endif

#if !defined GL_QUERY_RESULT_EXT
#define GL_QUERY_RESULT_EXT 0
#endif

#if !defined(GL_HALF_FLOAT)
#define GL_HALF_FLOAT GL_HALF_FLOAT_OES
#endif

#if !defined(GL_DEPTH_COMPONENT24)
#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
#endif

#if !defined(GL_BGRA)
#define GL_BGRA GL_BGRA_EXT
#endif

#if !defined(GL_DEPTH24_STENCIL8)
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#endif

#if !defined(GL_QUERY_RESULT_AVAILABLE)
#define GL_QUERY_RESULT_AVAILABLE GL_QUERY_RESULT_AVAILABLE_EXT
#endif

#if !defined(GL_QUERY_RESULT)
#define GL_QUERY_RESULT GL_QUERY_RESULT_EXT
#endif

#if !defined(GL_RED)
#define GL_RED 0x1903
#endif

#if !defined(GL_RG)
#define GL_RG 0x8227
#endif

#if !defined(GL_R8)
#define GL_R8 0x8229
#endif

#if !defined(GL_R16)
#define GL_R16 0x822A
#endif

#if !defined(GL_RG8)
#define GL_RG8 0x822B
#endif

#if !defined(GL_RG16)
#define GL_RG16 0x822C
#endif

#if !defined(GL_R16F)
#define GL_R16F 0x822D
#endif

#if !defined(GL_R32F)
#define GL_R32F 0x822E
#endif

#if !defined(GL_RG16F)
#define GL_RG16F 0x822F
#endif

#if !defined(GL_RG32F)
#define GL_RG32F 0x8230
#endif

#if !defined(GL_RGBA32F)
#define GL_RGBA32F 0x8814
#endif

#if !defined(GL_RGB32F)
#define GL_RGB32F 0x8815
#endif

#if !defined(GL_RGBA16F)
#define GL_RGBA16F 0x881A
#endif

#if !defined(GL_RGB16F)
#define GL_RGB16F 0x881B
#endif

#if !defined(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT)
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#if !defined(GL_TEXTURE_MAX_ANISOTROPY_EXT)
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif

#if 0
#define GL_CALL(expr) \
{ \
    expr; \
    int err = glGetError(); \
    if (err != GL_NO_ERROR) \
        Logger::Error("gl", "FAILED  %s (%i)\n", #expr, err); \
}

#else

#if defined(__DAVAENGINE_ANDROID__) && defined(__DAVAENGINE_ARM_7__)

extern volatile DAVA::uint8 pre_call_registers[64];

#define GL_CALL(expr) \
{ \
	if (_GLES2_ValidateNeonCalleeSavedRegisters) \
	{ \
		asm volatile("vstmia %0, {q4-q7}" ::"r"(pre_call_registers) \
                         : "memory"); \
        expr; \
        asm volatile("vldmia %0, {q4-q7}" ::"r"(pre_call_registers) \
                         : "q4", "q5", "q6", "q7"); \
	} \
	else \
	{ \
		expr; \
	}\
}

#else

#define GL_CALL(expr) expr;

#endif

#endif

extern GLuint _GLES2_Binded_FrameBuffer;
extern GLuint _GLES2_Default_FrameBuffer;
extern void* _GLES2_Native_Window;
extern void* _GLES2_Context;
extern void (*_GLES2_AcquireContext)();
extern void (*_GLES2_ReleaseContext)();

extern int _GLES2_DefaultFrameBuffer_Width;
extern int _GLES2_DefaultFrameBuffer_Height;

extern GLuint _GLES2_LastSetIB;
extern DAVA::uint8* _GLES2_LastSetIndices;
extern GLuint _GLES2_LastSetVB;
extern GLuint _GLES2_LastSetTex0;
extern GLenum _GLES2_LastSetTex0Target;
extern int _GLES2_LastActiveTexture;

#if defined(__DAVAENGINE_WIN32__)
extern HDC _GLES2_WindowDC;
#endif

extern bool _GLES2_IsGlDepth24Stencil8Supported;
extern bool _GLES2_IsGlDepthNvNonLinearSupported;
extern bool _GLES2_UseUserProvidedIndices;
extern volatile bool _GLES2_ValidateNeonCalleeSavedRegisters;

bool GetGLTextureFormat(rhi::TextureFormat rhiFormat, GLint* internalFormat, GLint* format, GLenum* type, bool* compressed);
