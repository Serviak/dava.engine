#if !defined __METAL_H__
#define __METAL_H__

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

extern id<MTLDevice>                _Metal_Device;
extern id<MTLCommandQueue>          _Metal_DefCmdQueue;
extern MTLRenderPassDescriptor*     _Metal_DefRenderPassDescriptor;
extern id<MTLTexture>               _Metal_DefFrameBuf;
extern id<MTLTexture>               _Metal_DefDepthBuf;
extern id<MTLTexture>               _Metal_DefStencilBuf;
extern id<MTLDepthStencilState>     _Metal_DefDepthState;
extern CAMetalLayer*                _Metal_Layer;


#endif // __METAL_H__
