#pragma once

#include "../rhi_Type.h"

#if defined(__DAVAENGINE_WIN32__)

    #include "GL/glew.h"
    #include <GL/GL.h>
    #include "GL/wglew.h"
    
    #define GetGLErrorString gluErrorString

#elif defined(__DAVAENGINE_MACOS__)

    #include <Carbon/Carbon.h>
    #include <AGL/agl.h>
    #include <OpenGL/glext.h>
    
    #define GetGLErrorString(code) #code

    #include "macos_gl.h"

#elif defined(__DAVAENGINE_IPHONE__)

    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>
    
    #define GetGLErrorString(code)  "<unknown>"

    #include "ios_gl.h"

#else

    #include <GL/GL.h>

#endif

#ifdef __DAVAENGINE_ANDROID__
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif //GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif //GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#endif //__DAVAENGINE_ANDROID__

#if !defined(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)
#define  GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG    0
#endif
#if !defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
#define  GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG    0
#endif

#if !defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
#define  GL_COMPRESSED_RGB_S3TC_DXT1_EXT    0
#endif

#if !defined(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
#define  GL_COMPRESSED_RGBA_S3TC_DXT1_EXT    0
#endif

#if !defined(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
#define  GL_COMPRESSED_RGBA_S3TC_DXT3_EXT    0
#endif

#if !defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
#define  GL_COMPRESSED_RGB_S3TC_DXT1_EXT    0
#endif

#if !defined(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
#define  GL_COMPRESSED_RGBA_S3TC_DXT5_EXT    0
#endif

#if !defined(GL_ETC1_RGB8_OES)
#define  GL_ETC1_RGB8_OES    0
#endif

#if !defined(GL_ATC_RGB_AMD)
#define  GL_ATC_RGB_AMD    0
#endif

#if !defined(GL_ATC_RGBA_EXPLICIT_ALPHA_AMD)
#define  GL_ATC_RGBA_EXPLICIT_ALPHA_AMD    0
#endif

#if !defined(GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD)
#define  GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD    0
#endif

#if !defined(GL_COMPRESSED_R11_EAC)
#define  GL_COMPRESSED_R11_EAC    0
#endif

#if !defined(GL_COMPRESSED_SIGNED_R11_EAC)
#define  GL_COMPRESSED_SIGNED_R11_EAC    0
#endif

#if !defined(GL_COMPRESSED_RG11_EAC)
#define  GL_COMPRESSED_RG11_EAC    0
#endif

#if !defined(GL_COMPRESSED_SIGNED_RG11_EAC)
#define  GL_COMPRESSED_SIGNED_RG11_EAC    0
#endif

#if !defined(GL_COMPRESSED_RGB8_ETC2)
#define  GL_COMPRESSED_RGB8_ETC2    0
#endif

#if !defined(GL_COMPRESSED_RGBA8_ETC2_EAC)
#define  GL_COMPRESSED_RGBA8_ETC2_EAC    0
#endif

#if !defined(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2)
#define  GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2    0
#endif

#if !defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG)
#define  GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG    0
#endif

#if !defined(GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG)
#define  GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG    0
#endif

#if !defined(GL_HALF_FLOAT_OES)
#define  GL_HALF_FLOAT_OES    0
#endif

#if !defined(GL_HALF_FLOAT)
#define GL_HALF_FLOAT GL_HALF_FLOAT_OES
#endif

#if !defined(GL_DEPTH_COMPONENT24)
#define GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24_OES
#endif

#if 0
#define GL_CALL(expr) \
{ \
    expr ; \
    int err = glGetError(); \
    if( err != GL_NO_ERROR ) \
        Log::Error( "gl", "FAILED  %s (%i) : %s\n", #expr, err, gluErrorString(err) ); \
}
#else
#define GL_CALL(expr) expr
#endif

extern GLuint   _GLES2_Binded_FrameBuffer;
extern GLuint   _GLES2_Default_FrameBuffer;
extern void*    _GLES2_Native_Window;
extern void*    _GLES2_Context;
extern void     (*_GLES2_AcquireContext)();
extern void     (*_GLES2_ReleaseContext)();

extern int      _GLES2_DefaultFrameBuffer_Width;
extern int      _GLES2_DefaultFrameBuffer_Height;

bool            GetGLTextureFormat( rhi::TextureFormat rhiFormat, GLint* internalFormat, GLint* format, GLenum* type, bool* compressed );
