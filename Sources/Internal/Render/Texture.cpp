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

#include "Render/RenderBase.h"
#include "Render/Texture.h"
#include "Utils/Utils.h"
#include "FileSystem/Logger.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Render/Renderer.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/RenderHelper.h"

#if defined(__DAVAENGINE_IPHONE__) 
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#elif defined(__DAVAENGINE_MACOS__)
#include <ApplicationServices/ApplicationServices.h>
#endif //PLATFORMS

#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageConvert.h"

#include "Render/TextureDescriptor.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Job/JobManager.h"
#include "Math/MathHelpers.h"
#include "Thread/LockGuard.h"



#ifdef __DAVAENGINE_ANDROID__
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif //GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif //GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#endif //__DAVAENGINE_ANDROID__

namespace DAVA 
{
    

Array<String, Texture::CUBE_FACE_COUNT> Texture::FACE_NAME_SUFFIX =
{{
    String("_px"),
    String("_nx"),
    String("_py"),
    String("_ny"),
    String("_pz"),
    String("_nz")
}};

class TextureMemoryUsageInfo
{
public:
	TextureMemoryUsageInfo()
	{
		pvrTexturesMemoryUsed = 0;
		texturesMemoryUsed = 0;
		fboMemoryUsed = 0;
	}
	
	void AllocPVRTexture(int size)
	{
		pvrTexturesMemoryUsed += size;
	}
	
	void ReleasePVRTexture(int size)
	{
		pvrTexturesMemoryUsed -= size;
	}
	
	void AllocTexture(int size)
	{
		texturesMemoryUsed += size;
	}
	
	void ReleaseTexture(int size)
	{
		texturesMemoryUsed -= size;
	}
	
	void AllocFBOTexture(int size)
	{
		fboMemoryUsed += size;
	}
	
	void ReleaseFBOTexture(int size)
	{
		fboMemoryUsed -= size;
	}
	
	// STATISTICS
	int pvrTexturesMemoryUsed;
	int texturesMemoryUsed;
	int	fboMemoryUsed;
};

eGPUFamily Texture::defaultGPU = GPU_ORIGIN;
    
static TextureMemoryUsageInfo texMemoryUsageInfo;
	
TexturesMap Texture::textureMap;

Mutex Texture::textureMapMutex;

static int32 textureFboCounter = 0;

bool Texture::pixelizationFlag = false;

// Main constructors
Texture * Texture::Get(const FilePath & pathName)
{
    LockGuard<Mutex> guard(textureMapMutex);

	Texture * texture = NULL;
	TexturesMap::iterator it = textureMap.find(FILEPATH_MAP_KEY(pathName));
	if (it != textureMap.end())
	{
		texture = it->second;
		texture->Retain();

		return texture;
	}

    return 0;
}

void Texture::AddToMap(Texture *tex)
{
    if(!tex->texDescriptor->pathname.IsEmpty())
    {
        textureMapMutex.Lock();
        DVASSERT(textureMap.find(FILEPATH_MAP_KEY(tex->texDescriptor->pathname)) == textureMap.end());
		textureMap[FILEPATH_MAP_KEY(tex->texDescriptor->pathname)] = tex;
        textureMapMutex.Unlock();
    }
}

    
Texture::Texture()
:	width(0)
,	height(0)
,	loadedAsFile(GPU_ORIGIN)
,	state(STATE_INVALID)
,	textureType(rhi::TEXTURE_TYPE_2D)
,	isRenderTarget(false)
,	isPink(false)
,	invalidater(NULL)
{
	texDescriptor = new TextureDescriptor();
}

Texture::~Texture()
{
    if(invalidater)
    {
        invalidater->RemoveTexture(this);
        invalidater = NULL;
    }
    ReleaseTextureData();
	SafeDelete(texDescriptor);
}
    
void Texture::ReleaseTextureData()
{
    if (handle.IsValid())
        rhi::DeleteTexture(handle);
    handle = rhi::HTexture(rhi::InvalidHandle);

	state = STATE_INVALID;
    isRenderTarget = false;
}




Texture * Texture::CreateTextFromData(PixelFormat format, uint8 * data, uint32 width, uint32 height, bool generateMipMaps, const char * addInfo)
{
	Texture * tx = CreateFromData(format, data, width, height, generateMipMaps);
    
	if (!addInfo)
    {
        tx->texDescriptor->pathname = Format("Text texture %d", textureFboCounter);
    }
	else
    {
        tx->texDescriptor->pathname = Format("Text texture %d info:%s", textureFboCounter, addInfo);
    }
    AddToMap(tx);
    
	textureFboCounter++;
	return tx;
}
	
void Texture::TexImage(int32 level, uint32 width, uint32 height, const void * _data, uint32 dataSize, uint32 cubeFaceId)
{
    rhi::UpdateTexture(handle, _data, level, (rhi::TextureFace)cubeFaceId);
}
    
Texture * Texture::CreateFromData(PixelFormat _format, const uint8 *_data, uint32 _width, uint32 _height, bool generateMipMaps)
{
	Image *image = Image::CreateFromData(_width, _height, _format, _data);
	if(!image) return NULL;

	Texture * texture = new Texture();
	texture->texDescriptor->Initialize(rhi::TEXADDR_CLAMP, generateMipMaps);
    
    Vector<Image *> *images = new Vector<Image *>();
    images->push_back(image);
	
    texture->SetParamsFromImages(images);
	texture->FlushDataToRenderer(images);

	return texture;
}
    
Texture * Texture::CreateFromData(Image *image, bool generateMipMaps)
{
	Texture * texture = new Texture();
    texture->texDescriptor->Initialize(rhi::TEXADDR_CLAMP, generateMipMaps);
    
    Vector<Image *> *images = new Vector<Image *>();
    image->Retain();
    images->push_back(image);
	
    texture->SetParamsFromImages(images);
	texture->FlushDataToRenderer(images);
    
	return texture;
}

	
void Texture::SetWrapMode(rhi::TextureAddrMode wrapU, rhi::TextureAddrMode wrapV, rhi::TextureAddrMode wrapW)
{
    samplerState.addrU = wrapU;
    samplerState.addrV = wrapV;
    samplerState.addrW = wrapW;
}

void Texture::SetMinMagFilter(rhi::TextureFilter minFilter, rhi::TextureFilter magFilter, rhi::TextureMipFilter mipFilter)
{
    samplerState.minFilter = minFilter;
    samplerState.magFilter = magFilter;
    samplerState.mipFilter = mipFilter;
}
	
void Texture::GenerateMipmaps()
{
    DVASSERT("Mipmap generation on fly is not supported anymore!")	
}



Texture * Texture::CreateFromImage(TextureDescriptor *descriptor, eGPUFamily gpu)
{
	Texture * texture = new Texture();
	texture->texDescriptor->Initialize(descriptor);

    Vector<Image *> * images = new Vector<Image *> ();
    
	bool loaded = texture->LoadImages(gpu, images);
    if(!loaded)
	{
		Logger::Error("[Texture::CreateFromImage] Cannot load texture from image. Descriptor: %s, GPU: %s",
            descriptor->pathname.GetAbsolutePathname().c_str(), GlobalEnumMap<eGPUFamily>::Instance()->ToString(gpu));

        SafeDelete(images);
		SafeRelease(texture);
		return NULL;
	}

	texture->SetParamsFromImages(images);
	texture->FlushDataToRenderer(images);

	return texture;
}

bool Texture::LoadImages(eGPUFamily gpu, Vector<Image *> * images)
{
    DVASSERT(gpu != GPU_INVALID);
    
    if (!IsLoadAvailable(gpu))
    {
        Logger::Error("[Texture::LoadImages] Load not avalible: invalid requsted GPU family (%s)", GlobalEnumMap<eGPUFamily>::Instance()->ToString(gpu));
        return false;
    }
	
    const int32 baseMipMap = GetBaseMipMap();
	if(texDescriptor->IsCubeMap() && (!GPUFamilyDescriptor::IsGPUForDevice(gpu)))
	{
        Vector<FilePath> facePathes;
        texDescriptor->GetFacePathnames(facePathes);
        
        PixelFormat imagesFormat = FORMAT_INVALID;
		for(auto i = 0; i < CUBE_FACE_COUNT; ++i)
		{
            auto & currentfacePath = facePathes[i];
            if (currentfacePath.IsEmpty())
                continue;

            Vector<Image *> faceImage;
            ImageSystem::Instance()->Load(currentfacePath, faceImage, baseMipMap);
            if(faceImage.size() == 0)
			{
                Logger::Error("[Texture::LoadImages] Cannot open file %s", currentfacePath.GetAbsolutePathname().c_str());

				ReleaseImages(images);
				return false;
			}
            
			DVASSERT(faceImage.size() == 1);

			faceImage[0]->cubeFaceID = i;
			faceImage[0]->mipmapLevel = 0;

            //cubemap formats validation
            if(FORMAT_INVALID == imagesFormat)
            {
                imagesFormat = faceImage[0]->format;
            }
            else if(imagesFormat != faceImage[0]->format)
            {
                Logger::Error("[Texture::LoadImages] Face(%s) has different pixel format(%s)", currentfacePath.GetAbsolutePathname().c_str(), PixelFormatDescriptor::GetPixelFormatString(faceImage[0]->format));
                
                ReleaseImages(images);
                return false;
            }
            //end of cubemap formats validation
            
            if(texDescriptor->GetGenerateMipMaps())
            {
                Vector<Image *> mipmapsImages = faceImage[0]->CreateMipMapsImages();
                images->insert(images->end(), mipmapsImages.begin(), mipmapsImages.end());
                SafeRelease(faceImage[0]);
            }
            else
            {
			    images->push_back(faceImage[0]);
            }
		}
	}
	else
	{
		FilePath imagePathname = texDescriptor->CreatePathnameForGPU(gpu);

        ImageSystem::Instance()->Load(imagePathname, *images, baseMipMap);
        ImageSystem::Instance()->EnsurePowerOf2Images(*images);
        if(images->size() == 1 && gpu == GPU_ORIGIN && texDescriptor->GetGenerateMipMaps())
        {
            Image * img = *images->begin();
            *images = img->CreateMipMapsImages(texDescriptor->dataSettings.GetIsNormalMap());
            SafeRelease(img);
        }
    }

    if (0 == images->size())
    {
        Logger::Error("[Texture::LoadImages] Loaded images count is zero");
        return false;
    }

	bool isSizeCorrect = CheckImageSize(*images);
	if(!isSizeCorrect)
	{
        Logger::Error("[Texture::LoadImages] Size if loaded images is invalid (not power of 2)");

		ReleaseImages(images);
		return false;
	}

	isPink = false;
	state = STATE_DATA_LOADED;

	return true;
}


void Texture::ReleaseImages(Vector<Image *> *images)
{
	for_each(images->begin(), images->end(), SafeRelease<Image>);
	images->clear();
}

void Texture::SetParamsFromImages(const Vector<Image *> * images)
{
	DVASSERT(images->size() != 0);

    Image *img = *images->begin();
	width = img->width;
	height = img->height;
	texDescriptor->format = img->format;

	textureType = (img->cubeFaceID != Texture::INVALID_CUBEMAP_FACE) ? rhi::TEXTURE_TYPE_CUBE : rhi::TEXTURE_TYPE_2D;
    
    state = STATE_DATA_LOADED;
}

void Texture::FlushDataToRenderer(Vector<Image *> * images)
{
    DVASSERT(images->size() != 0);

    const PixelFormatDescriptor & formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(texDescriptor->format);
    rhi::Texture::Descriptor descriptor;
    descriptor.autoGenMipmaps = false;
    descriptor.isRenderTarget = false;
    descriptor.width = (*images)[0]->width;
    descriptor.height = (*images)[0]->height;
    descriptor.type = ((*images)[0]->cubeFaceID == Texture::INVALID_CUBEMAP_FACE) ? rhi::TEXTURE_TYPE_2D : rhi::TEXTURE_TYPE_CUBE;
    descriptor.format = formatDescriptor.format;

    descriptor.mipCount = images->size();
    for (Image * img : (*images))
        descriptor.mipCount = Max(descriptor.mipCount, img->mipmapLevel + 1);

    DVASSERT(descriptor.format != -1);//unsupported format
    handle = rhi::CreateTexture(descriptor);

    for (uint32 i = 0; i < (uint32)images->size(); ++i)
    {
        Image *img = (*images)[i];
        TexImage((img->mipmapLevel != (uint32)-1) ? img->mipmapLevel : i, img->width, img->height, img->data, img->dataSize, img->cubeFaceID);
    }

    samplerState.addrU = texDescriptor->drawSettings.wrapModeS;
    samplerState.addrV = texDescriptor->drawSettings.wrapModeT;
    samplerState.minFilter = texDescriptor->drawSettings.minFilter;
    samplerState.magFilter = texDescriptor->drawSettings.magFilter;
    samplerState.mipFilter = texDescriptor->drawSettings.mipFilter;

    state = STATE_VALID;

    ReleaseImages(images);
    SafeDelete(images);
}

bool Texture::CheckImageSize(const Vector<DAVA::Image *> &imageSet)
{
    for (int32 i = 0; i < (int32)imageSet.size(); ++i)
    {
        if(!IsPowerOf2(imageSet[i]->GetWidth()) || !IsPowerOf2(imageSet[i]->GetHeight()))
        {
            return false;
        }
    }
    
    return true;
}

Texture * Texture::CreateFromFile(const FilePath & pathName, const FastName &group, rhi::TextureType typeHint)
{
	Texture * texture = PureCreate(pathName, group);
 	if(!texture)
	{
        TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(pathName);
        if(descriptor)
        {
            texture = CreatePink(descriptor->IsCubeMap() ? rhi::TEXTURE_TYPE_CUBE : typeHint);
            texture->texDescriptor->Initialize(descriptor);
            SafeDelete(descriptor);
        }
        else
        {
            texture = CreatePink(typeHint);
            texture->texDescriptor->pathname = (!pathName.IsEmpty()) ? TextureDescriptor::GetDescriptorPathname(pathName) : FilePath();
        }
        
        texture->texDescriptor->SetQualityGroup(group);
        
        AddToMap(texture);
	}

	return texture;
}

Texture * Texture::PureCreate(const FilePath & pathName, const FastName &group)
{
	if(pathName.IsEmpty() || pathName.GetType() == FilePath::PATH_IN_MEMORY)
		return NULL;

    if(!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::TEXTURE_LOAD_ENABLED))
        return NULL;

    FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(pathName);
    Texture * texture = Texture::Get(descriptorPathname);
	if (texture) return texture;
    
    TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(descriptorPathname);
    if(!descriptor) return NULL;
    
    descriptor->SetQualityGroup(group);

	eGPUFamily gpuForLoading = GetGPUForLoading(defaultGPU, descriptor);
	texture = CreateFromImage(descriptor, gpuForLoading);
	if(texture)
	{
		texture->loadedAsFile = gpuForLoading;
		AddToMap(texture);
	}

	delete descriptor;
	return texture;
}
    
void Texture::ReloadFromData(PixelFormat format, uint8 * data, uint32 _width, uint32 _height)
{
    ReleaseTextureData();
    
    Image *image = Image::CreateFromData(_width, _height, format, data);
	if(!image) return;
    
    Vector<Image *> *images = new Vector<Image *>();
    images->push_back(image);
	
    SetParamsFromImages(images);
	FlushDataToRenderer(images);
}
    
void Texture::Reload()
{
    ReloadAs(loadedAsFile);
}
    
void Texture::ReloadAs(eGPUFamily gpuFamily)
{
    DVASSERT(isRenderTarget == false);
    
    ReleaseTextureData();

	bool descriptorReloaded = texDescriptor->Reload();
    
	eGPUFamily gpuForLoading = GetGPUForLoading(gpuFamily, texDescriptor);
    Vector<Image *> *images = new Vector<Image *> ();
    
    bool loaded = false;
    if(descriptorReloaded && Renderer::GetOptions()->IsOptionEnabled(RenderOptions::TEXTURE_LOAD_ENABLED))
    {
	    loaded = LoadImages(gpuForLoading, images);
    }

	if(loaded)
	{
		loadedAsFile = gpuForLoading;

		SetParamsFromImages(images);
		FlushDataToRenderer(images);
	}
	else
    {
        SafeDelete(images);
        
        Logger::Error("[Texture::ReloadAs] Cannot reload from file %s", texDescriptor->pathname.GetAbsolutePathname().c_str());
        MakePink();
    }
}

    
bool Texture::IsLoadAvailable(const eGPUFamily gpuFamily) const
{
    if(texDescriptor->IsCompressedFile())
    {
        return true;
    }
    
    if(GPUFamilyDescriptor::IsGPUForDevice(gpuFamily) && texDescriptor->compression[gpuFamily].format == FORMAT_INVALID)
    {
        return false;
    }
    
    return true;
}

    
int32 Texture::Release()
{
	if(GetRetainCount() == 1)
	{
        textureMapMutex.Lock();
		textureMap.erase(FILEPATH_MAP_KEY(texDescriptor->pathname));
        textureMapMutex.Unlock();
	}
	return BaseObject::Release();
}
	
Texture * Texture::CreateFBO(uint32 w, uint32 h, PixelFormat format, rhi::TextureType requestedType)
{
    int32 dx = Max((int32)w, 8);
    EnsurePowerOf2(dx);

    int32 dy = Max((int32)h, 8);
    EnsurePowerOf2(dy);
    
    Texture *tx = new Texture();
    
    const PixelFormatDescriptor & formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(format);
    rhi::Texture::Descriptor descriptor;
    descriptor.width = dx;
    descriptor.height = dy;
    descriptor.autoGenMipmaps = false;
    descriptor.isRenderTarget = true;
    descriptor.type = requestedType;
    descriptor.format = formatDescriptor.format;
    DVASSERT(descriptor.format != -1);//unsupported format
    tx->handle = rhi::CreateTexture(descriptor);

    tx->isRenderTarget = true;
    tx->texDescriptor->pathname = Format("FBO texture %d", textureFboCounter);
    AddToMap(tx);

    textureFboCounter++;

    return tx;
    
}

	
void Texture::DumpTextures()
{
	uint32 allocSize = 0;
	int32 cnt = 0;
	Logger::FrameworkDebug("============================================================");
	Logger::FrameworkDebug("--------------- Currently allocated textures ---------------");

    textureMapMutex.Lock();
	for(TexturesMap::iterator it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		Texture *t = it->second;
		Logger::FrameworkDebug("%s with id %d (%dx%d) retainCount: %d debug: %s format: %s", t->texDescriptor->pathname.GetAbsolutePathname().c_str(), t->handle, t->width, t->height, 
								t->GetRetainCount(), t->debugInfo.c_str(), PixelFormatDescriptor::GetPixelFormatString(t->texDescriptor->format));
		cnt++;
        
        DVASSERT((0 <= t->texDescriptor->format) && (t->texDescriptor->format < FORMAT_COUNT));
        if(FORMAT_INVALID != t->texDescriptor->format)
        {
            allocSize += t->width * t->height * PixelFormatDescriptor::GetPixelFormatSizeInBits(t->texDescriptor->format);
        }
	}
    textureMapMutex.Unlock();

	Logger::FrameworkDebug("      Total allocated textures %d    memory size %d", cnt, allocSize/8);
	Logger::FrameworkDebug("============================================================");
}
	
void Texture::SetDebugInfo(const String & _debugInfo)
{
#if defined(__DAVAENGINE_DEBUG__)
	debugInfo = FastName(_debugInfo.c_str());
#endif
}
	
#if defined(__DAVAENGINE_ANDROID__)
	
void Texture::Lost()
{
	RenderResource::Lost();
    
    ReleaseTextureData();
}

void Texture::Invalidate()
{
	RenderResource::Invalidate();
	
	DVASSERT(id == 0 && "Texture always invalidated");
	if (id)
	{
		return;
	}

	if (invalidater)
    {
        invalidater->InvalidateTexture(this);
    }
    else
    {
        const FilePath& relativePathname = texDescriptor->GetSourceTexturePathname();
        if (relativePathname.GetType() == FilePath::PATH_IN_FILESYSTEM ||
            relativePathname.GetType() == FilePath::PATH_IN_RESOURCES ||
            relativePathname.GetType() == FilePath::PATH_IN_DOCUMENTS)
        {
            Reload();
        }
        else if (relativePathname.GetType() == FilePath::PATH_IN_MEMORY)
        {
            // Make it pink, to prevent craches
            Logger::Debug("[Texture::Invalidate] - invalidater is null");
            MakePink();
        }
        else if (isPink)
        {
            MakePink();
        }
    }
}
#endif //#if defined(__DAVAENGINE_ANDROID__)

Image * Texture::ReadDataToImage()
{
	const PixelFormatDescriptor & formatDescriptor = PixelFormatDescriptor::GetPixelFormatDescriptor(texDescriptor->format);

    Image *image = Image::Create(width, height, formatDescriptor.formatID);
    uint8 *imageData = image->GetData();
#if RHI_COMPLETE
#if defined(__DAVAENGINE_OPENGL__)
    
    int32 saveFBO = RenderManager::Instance()->HWglGetLastFBO();
    int32 saveId = RenderManager::Instance()->HWglGetLastTextureID(textureType);

    RenderManager::Instance()->HWglBindFBO(fboID);
	RenderManager::Instance()->HWglBindTexture(id, textureType);
    
    if(FORMAT_INVALID != formatDescriptor.formatID)
    {
		RENDER_VERIFY(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        RENDER_VERIFY(glReadPixels(0, 0, width, height, formatDescriptor.format, formatDescriptor.type, (GLvoid *)imageData));
    }

    RenderManager::Instance()->HWglBindFBO(saveFBO);
    RenderManager::Instance()->HWglBindTexture(saveId, textureType);
    
#endif //#if defined(__DAVAENGINE_OPENGL__)
#endif  // RHI_COMPLETE
    return image; 
}


Image * Texture::CreateImageFromMemory()
{
    Image *image = NULL;
#if RHI_COMPLETE
    if(isRenderTarget)
    {
        image = ReadDataToImage();
    }
    else
    {
        Texture * oldRenderTarget = RenderManager::Instance()->GetRenderTarget();

        Texture *renderTarget = Texture::CreateFBO(width, height, texDescriptor->format, DEPTH_NONE);
        RenderHelper::Instance()->Set2DRenderTarget(renderTarget);
        RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
        RenderHelper::Instance()->DrawTexture(this, renderState);

        RenderManager::Instance()->SetRenderTarget(oldRenderTarget);
        
        image = renderTarget->CreateImageFromMemory(renderState);

        SafeRelease(renderTarget);
    }
#endif //RHI_COMPLETE        
    
    return image;
}
	
const TexturesMap & Texture::GetTextureMap()
{
    return textureMap;
}

uint32 Texture::GetDataSize() const
{
    DVASSERT((0 <= texDescriptor->format) && (texDescriptor->format < FORMAT_COUNT));
    
    uint32 allocSize = width * height * PixelFormatDescriptor::GetPixelFormatSizeInBits(texDescriptor->format) / 8;
    return allocSize;
}

Texture * Texture::CreatePink(rhi::TextureType requestedType, bool checkers)
{
	//we need instances for pink textures for ResourceEditor. We use it for reloading for different GPUs
	//pink textures at game is invalid situation
	Texture *tex = new Texture();
	if(rhi::TEXTURE_TYPE_CUBE == requestedType)
	{
        tex->texDescriptor->Initialize(rhi::TEXADDR_CLAMP, true);
		tex->texDescriptor->dataSettings.cubefaceFlags = 0x000000FF;
	}
	else
	{
        tex->texDescriptor->Initialize(rhi::TEXADDR_CLAMP, false);
	}

	tex->MakePink(checkers);

	return tex;
}

void Texture::MakePink(bool checkers)
{
    Vector<Image *> *images = new Vector<Image *> ();
	if(texDescriptor->IsCubeMap())
	{
		for(uint32 i = 0; i < Texture::CUBE_FACE_COUNT; ++i)
		{
            Image *img = Image::CreatePinkPlaceholder(checkers);
			img->cubeFaceID = i;
			img->mipmapLevel = 0;

			images->push_back(img);
		}
	}
	else
	{
		images->push_back(Image::CreatePinkPlaceholder(checkers));
	}

	SetParamsFromImages(images);
    FlushDataToRenderer(images);

    isPink = true;

    SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);
}
    
bool Texture::IsPinkPlaceholder()
{
	return isPink;
}
    
void Texture::SetDefaultGPU(eGPUFamily gpuFamily)
{
    defaultGPU = gpuFamily;
}

eGPUFamily Texture::GetDefaultGPU()
{
    return defaultGPU;
}

    
eGPUFamily Texture::GetGPUForLoading(const eGPUFamily requestedGPU, const TextureDescriptor *descriptor)
{
    if(descriptor->IsCompressedFile())
        return (eGPUFamily)descriptor->exportedAsGpuFamily;
    
    return requestedGPU;
}

void Texture::SetInvalidater(TextureInvalidater* invalidater)
{
    if(this->invalidater)
    {
        this->invalidater->RemoveTexture(this);
    }
	this->invalidater = invalidater;
    if(invalidater != NULL)
    {
        invalidater->AddTexture(this);
    }
}

const FilePath & Texture::GetPathname() const
{
    return texDescriptor->pathname;
}
    
void Texture::SetPathname(const FilePath& path)
{
    textureMapMutex.Lock();
    textureMap.erase(FILEPATH_MAP_KEY(texDescriptor->pathname));
    texDescriptor->pathname = path;
    if (!texDescriptor->pathname.IsEmpty())
    {
        DVASSERT(textureMap.find(FILEPATH_MAP_KEY(texDescriptor->pathname)) == textureMap.end());
        textureMap[FILEPATH_MAP_KEY(texDescriptor->pathname)] = this;
    }
    textureMapMutex.Unlock();
}

PixelFormat Texture::GetFormat() const
{
	return texDescriptor->format;
}

void Texture::SetPixelization(bool value)
{
#if RHI_COMPLETE
    if (value == pixelizationFlag)
    {
        return;
    }

    pixelizationFlag = value;
    const TexturesMap& texturesMap = GetTextureMap();

    textureMapMutex.Lock();
    for (Map<FilePath, Texture *>::const_iterator iter = texturesMap.begin(); iter != texturesMap.end(); iter ++)
    {
        Texture* texture = iter->second;
        TextureFilter minFilter = pixelizationFlag ? FILTER_NEAREST : (TextureFilter)texture->GetDescriptor()->drawSettings.minFilter;
        TextureFilter magFilter = pixelizationFlag ? FILTER_NEAREST : (TextureFilter)texture->GetDescriptor()->drawSettings.magFilter;
        texture->SetMinMagFilter(minFilter, magFilter);
    }
    textureMapMutex.Unlock();
#endif //RHI_COMPLETE
}


int32 Texture::GetBaseMipMap() const
{
    if(texDescriptor->GetQualityGroup().IsValid())
    {
        const TextureQuality *curTxQuality = QualitySettingsSystem::Instance()->GetTxQuality(QualitySettingsSystem::Instance()->GetCurTextureQuality());
        if(NULL != curTxQuality)
        {
            return static_cast<int32>(curTxQuality->albedoBaseMipMapLevel);
        }
    }

    return 0;
}

};
