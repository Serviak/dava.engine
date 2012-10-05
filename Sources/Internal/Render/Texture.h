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
        * Created by Vitaliy Borodovsky 
        \todo Add possibility to change premultiplication & make possibility to change format on fly
=====================================================================================*/
#ifndef __DAVAENGINE_TEXTURE_H__
#define __DAVAENGINE_TEXTURE_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Render/RenderResource.h"


namespace DAVA
{
/**
	\ingroup render
	\brief Class that represents texture objects in our SDK. 
	This class support the following formats: RGBA8888, RGB565, RGBA4444, A8 on all platforms. 
	For iOS it also support compressed PVR formats. (PVR2 and PVR4)
 */
class Image;
class Texture : public RenderResource
{
public:
	
	enum TextureWrap
	{
		WRAP_CLAMP_TO_EDGE = 0,
		WRAP_CLAMP,
		WRAP_REPEAT,
	};

	enum DepthFormat
	{
		DEPTH_NONE = 0,
		DEPTH_RENDERBUFFER
	};
	
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	static const int MAX_WIDTH = 1024;
	static const int MIN_WIDTH = 8;
	static const int MAX_HEIGHT = 1024;
	static const int MIN_HEIGHT = 8;
#else //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	static const int MAX_WIDTH = 4096;
	static const int MIN_WIDTH = 8;
	static const int MAX_HEIGHT = 4096;
	static const int MIN_HEIGHT = 8;
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

	// Main constructurs
	
    static void InitializePixelFormatDescriptors();

    
	/**
        \brief Return size of pixel format in bits 
        \returns size in bits, for example for FORMAT_RGBA8888 function will return 32.
     */
	static int32 GetPixelFormatSizeInBytes(PixelFormat format);
	static int32 GetPixelFormatSizeInBits(PixelFormat format);
	/**
        \brief Return string representation of pixel format
        \returns string value describing pixel format
     */
    static const char * GetPixelFormatString(PixelFormat format);
    
    /**
        \brief Create texture from data arrray
        This function creates texture from given format, data pointer and width + height
     
        \param[in] format desired pixel format
        \param[in] data desired data 
        \param[in] width width of new texture
        \param[in] height height of new texture
     */
	static Texture * CreateFromData(PixelFormat format, const uint8 *data, uint32 width, uint32 height);

    /**
        \brief Create text texture from data arrray
        This function creates texture from given format, data pointer and width + height, but adds addInfo string to relativePathname variable for easy identification of textures
        
        \param[in] format desired pixel format
        \param[in] data desired data 
        \param[in] width width of new texture
        \param[in] height height of new texture
        \param[in] addInfo additional info
     */
	static Texture * CreateTextFromData(PixelFormat format, uint8 * data, uint32 width, uint32 height, const char * addInfo = 0);
    
	/**
        \brief Create texture from given file. Supported formats .png, .pvr (only on iOS). 
		If file cannot be opened, returns "pink placeholder" texture.
        \param[in] pathName path to the png or pvr file
     */
	static Texture * CreateFromFile(const String & pathName);

	/**
        \brief Create texture from given file. Supported formats .png, .pvr (only on iOS). 
		If file cannot be opened, returns 0
        \param[in] pathName path to the png or pvr file
     */
	static Texture * PureCreate(const String & pathName);
    
	/**
        \brief Create FBO from given width, height and format
        \param[in] width width of the fbo
        \param[in] height height of the fbo
        \param[in] format format of the fbo
		\param[in] useDepthbuffer if set to true, addition depthbuffer will be created for this fbo
        \todo reorder variables in function, and make format variable first to make it similar to CreateFromData function.
     */
	static Texture * CreateFBO(uint32 width, uint32 height, PixelFormat format, DepthFormat depthFormat);
	
    /**
        \brief Function to load specific mip-map level from file
        \param[in] level level of mip map you want to replace
        \param[in] pathName path to file you want to use for texture
     */
	void LoadMipMapFromFile(int32 level, const String & pathName);

	/**
        \brief Sets default RGBA format that is used for textures loaded from files. 
        This functino define which format is used by default when you are loading files from disk. 
        By default it's RGBA8888 format. But for example if you want to load something in RGBA4444 format you 
        can use the following code
     
        \code
        Texture::SetDefaultRGBAFormat(FORMAT_RGBA4444);
        texRGBA4444 = Texture::CreateFromFile("~res:/Scenes/Textures/texture.png");
        Texture::SetDefaultRGBAFormat(FORMAT_RGBA8888);
        \endcode
     */
	static void SetDefaultRGBAFormat(PixelFormat format);
	static PixelFormat GetDefaultRGBAFormat();
	
	virtual int32 Release();

	static void	DumpTextures();

	inline int32 GetWidth() { return width; }
	inline int32 GetHeight() { return height; }
	
	static void EnableMipmapGeneration();
	static void DisableMipmapGeneration();
    static bool IsMipmapGenerationEnabled() { return isMipmapGenerationEnabled; };
	void GenerateMipmaps();
	void GeneratePixelesation();
	
	void TexImage(int32 level, uint32 width, uint32 height, const void * _data);

	void SetWrapMode(TextureWrap wrapS, TextureWrap wrapT);
	
	void UsePvrMipmaps();
        
    /**
        \brief This function can enable / disable autosave for render targets.
        It's actual only for DX9 and for other systems is does nothing
        If you refreshing your rendertargets every frame you can disable autosave for them for performance on DX9
        By default autosave is enabled for all DX9 textures. 
     */
    inline void EnableRenderTargetAutosave(bool isEnabled);
    
    
    /**
        \brief Function to receive pathname of texture object
        \returns pathname of texture
     */
    inline const String & GetPathname() const;
    
    Image * CreateImageFromMemory();

	static Texture * GetPinkPlaceholder();
	static void ReleasePinkPlaceholder();

	/**
        \brief Check if texture was created by GetPinkPlaceholder()
     */
	bool IsPinkPlaceholder();
    
    
    static PixelFormatDescriptor GetPixelFormatDescriptor(PixelFormat formatID);

public:							// properties for fast access

#if defined(__DAVAENGINE_OPENGL__)
	uint32		id;				// OpenGL id for texture

#if defined(__DAVAENGINE_ANDROID__) || defined (__DAVAENGINE_MACOS__)
	bool		 renderTargetModified;
    bool         renderTargetAutosave;

	virtual void SaveToSystemMemory();
	virtual void Lost();
	virtual void Invalidate();
	void InvalidateFromFile();
	void InvalidateFromSavedData();

    void SaveData(PixelFormat format, uint8 * data, uint32 width, uint32 height);
    void SaveData(uint8 * data, int32 dataSize);
    
	uint8 *savedData;
	int32 savedDataSize;
#endif //#if defined(__DAVAENGINE_ANDROID__) 

#elif defined(__DAVAENGINE_DIRECTX9__)
	static LPDIRECT3DTEXTURE9 CreateTextureNative(Vector2 & size, PixelFormat & format, bool isRenderTarget, int32 flags);
	void SetAsHardwareCursor(const Vector2 & hotSpot);
	LPDIRECT3DTEXTURE9 id;
	LPDIRECT3DTEXTURE9 saveTexture;
	bool		 renderTargetModified;
    bool         renderTargetAutosave;

	virtual void SaveToSystemMemory();
	virtual void Lost();
	virtual void Invalidate();
	
#endif //#if defined(__DAVAENGINE_OPENGL__)

	String		relativePathname;
	String		debugInfo;
	uint32		width;			// texture width 
	uint32		height;			// texture height
//	uint32		imageWidth;		// image width
//	uint32		imageHeight;	// image height
#if defined(__DAVAENGINE_OPENGL__)
	uint32		fboID;			// id of frame buffer object
	uint32		rboID;
#endif //#if defined(__DAVAENGINE_OPENGL__)
	PixelFormat format;			// texture format 
	DepthFormat depthFormat;
	bool		isRenderTarget;

    bool isMimMapTexture;
	bool isAlphaPremultiplied;

    TextureWrap wrapModeS; 
    TextureWrap wrapModeT;

	void SetDebugInfo(const String & _debugInfo);

	static const Map<String, Texture*> & GetTextureMap();
    
    int32 GetDataSize();
    
    void ReleaseTextureData();
    
private:
	static Map<String, Texture*> textureMap;	
	static Texture * Get(const String & name);
	static Texture * CreateFromPNG(const String & pathName);// , PixelFormat format = SELECT_CLOSEST_FORMAT, bool premultipliedAlpha = false);
	static Texture * CreateFromPVR(const String & pathName);// , PixelFormat format = SELECT_CLOSEST_FORMAT);

	
	static Texture * UnpackPVRData(uint8 * data, uint32 dataSize);

	static PixelFormat defaultRGBAFormat;
	static bool	isMipmapGenerationEnabled;
	Texture();
	virtual ~Texture();
    
    Image * ReadDataToImage();

	static Texture * pinkPlaceholder;
    
    static PixelFormatDescriptor pixelDescriptors[FORMAT_COUNT];
    static void SetPixelDescription(PixelFormat index, const String &name, int32 size, GLenum type, GLenum format, GLenum internalFormat);
};
    
// Implementation of inline functions
inline void Texture::EnableRenderTargetAutosave(bool isEnabled)
{
#if defined(__DAVAENGINE_DIRECTX9__) || defined(__DAVAENGINE_ANDROID__)
    renderTargetAutosave = isEnabled;
#endif //#if defined(__DAVAENGINE_DIRECTX9__) || defined(__DAVAENGINE_ANDROID__)
}
inline const String & Texture::GetPathname() const
{
    return relativePathname;
}

};
#endif // __DAVAENGINE_TEXTUREGLES_H__
