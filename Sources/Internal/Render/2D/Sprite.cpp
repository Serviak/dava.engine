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
=====================================================================================*/
#include "Render/2D/Sprite.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Render/RenderManager.h"
#include "Utils/StringFormat.h"
#include "Platform/SystemTimer.h"
#include "FileSystem/File.h"
#include "Core/Core.h"
#include "Render/Shader.h"
#include "Render/RenderManagerGL20.h"
#include "Render/RenderHelper.h"
#include "FileSystem/LocalizationSystem.h"

namespace DAVA 
{
	
Map<String, Sprite*> spriteMap;
static int32 fboCounter = 0;
Vector<Vector2> Sprite::clippedTexCoords;
Vector<Vector2> Sprite::clippedVertices;

Sprite::Sprite()
{
	textures = 0;
	frameTextureIndex = 0;
	textureCount = 0;
	
	frameVertices = 0;
	texCoords = 0;
	rectsAndOffsets = 0;
//	originalVertices = 0;
	
	size.dx = 24;
	size.dy = 24;
//	originalSize = size;
	frameCount = 0;
	frame = 0;
    
    isPreparedForTiling = false;
	
	modification = 0;
	flags = 0;
	resourceSizeIndex = 0;

	clipPolygon = 0;
	
	resourceToVirtualFactor = 1.0f;
    resourceToPhysicalFactor = 1.0f;
    
    spriteRenderObject = new RenderDataObject();
    vertexStream = spriteRenderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
    texCoordStream  = spriteRenderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);    
}

Sprite* Sprite::PureCreate(const String & spriteName, Sprite* forPointer)
{
//	Logger::Debug("pure create: %s", spriteName.c_str());
	bool usedForScale = false;//Думаю, после исправлений в конвертере, эта магия больше не нужна. Но переменную пока оставлю.
//	Logger::Info("Sprite pure creation");
	String pathName = spriteName + ".txt";
	
	Sprite * spr = forPointer;
	
	int pos = (int)spriteName.find(Core::Instance()->GetResourceFolder(Core::Instance()->GetBaseResourceIndex()));
	String scaledName = spriteName.substr(0, pos) + Core::Instance()->GetResourceFolder(Core::Instance()->GetDesirableResourceIndex()) + spriteName.substr(pos + Core::Instance()->GetResourceFolder(Core::Instance()->GetBaseResourceIndex()).length());
	String scaledPath = scaledName + ".txt";
	
	
	Map<String, Sprite*>::iterator it;
	it = spriteMap.find(scaledPath);
	if (it != spriteMap.end())
	{
		spr = it->second;
		spr->Retain();
		//		Logger::Instance()->Debug("Sprite::Create(%s) from map\n", pathName.c_str());
		return spr;
	}

    File * fp = 0;
    String texturePath;
    
	size_t slashPos = scaledPath.rfind("/");
    String fileName = scaledPath.substr(slashPos + 1);
    String localizedScaledPath = scaledPath.substr(0, slashPos + 1) + LocalizationSystem::Instance()->GetCurrentLocale() + "/" + fileName;

    fp = File::Create(localizedScaledPath, File::READ|File::OPEN);
    texturePath = localizedScaledPath.substr(0, localizedScaledPath.length() - 4);
    
    if(!fp)
    {
    	fp = File::Create(scaledPath, File::READ|File::OPEN);
        texturePath = scaledName;
    }
	
	uint64 timeSpriteRead = SystemTimer::Instance()->AbsoluteMS();


	if (!fp)
	{
		Map<String, Sprite*>::iterator it;
		it = spriteMap.find(pathName);
		if (it != spriteMap.end())
		{
			spr = it->second;
			spr->Retain();
			//		Logger::Instance()->Debug("Sprite::Create(%s) from map\n", pathName.c_str());
			return spr;
		}
	
        size_t pos = pathName.rfind("/");
        String fileName = pathName.substr(pos + 1);
        String localizedPathName = pathName.substr(0, pos + 1) + LocalizationSystem::Instance()->GetCurrentLocale() + "/" + fileName;
		texturePath = localizedPathName.substr(0, pathName.length() - 4);
	
		fp = File::Create(localizedPathName, File::READ|File::OPEN);
	
		if (!fp)
		{
            fp = File::Create(pathName, File::READ|File::OPEN);
            texturePath = pathName.substr(0, pathName.length() - 4);
            if (!fp)
            {    
                Logger::Instance()->Warning("Failed to open sprite file: %s", pathName.c_str());
                return 0;
            }
		}	
		if (!spr) 
		{
			spr = new Sprite();
		}
		spr->resourceSizeIndex = Core::Instance()->GetBaseResourceIndex();
	}
	else 
	{
		if (!spr) 
		{
			spr = new Sprite();
		}
		spr->resourceSizeIndex = Core::Instance()->GetDesirableResourceIndex();
//		texturePath = scaledName;
	}

	
	
	size_t tpos = texturePath.rfind("/");
	if(tpos != String::npos)
	{
		texturePath.erase(tpos + 1);
	}
	
	char tempBuf[1024];

	
	spr->type = SPRITE_FROM_FILE;
	spr->relativePathname = pathName;
	fp->ReadLine(tempBuf, 1024);
	sscanf(tempBuf, "%d", &spr->textureCount);
	spr->textures = new Texture*[spr->textureCount];
	timeSpriteRead = SystemTimer::Instance()->AbsoluteMS() - timeSpriteRead;
	
	char textureCharName[128];
	for (int k = 0; k < spr->textureCount; ++k)
	{
		fp->ReadLine(tempBuf, 1024);
		sscanf(tempBuf, "%s", textureCharName);
		String tp = texturePath + textureCharName;
//		Logger::Debug("Opening texture: %s", tp.c_str());
		Texture *texture = Texture::CreateFromFile(tp.c_str());
		
		spr->textures[k] = texture;
		DVASSERT_MSG(texture, "ERROR: Texture loading failed"/* + pathName*/);
	}	

	uint64 timeSpriteRead2 = SystemTimer::Instance()->AbsoluteMS();

	int32 width, height;
	fp->ReadLine(tempBuf, 1024);
	sscanf(tempBuf, "%d %d", &width, &height); 
	spr->size.dx = (float32)width;
	spr->size.dy = (float32)height;
//	spr->originalSize = spr->size;
	spr->size.dx *= Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
	spr->size.dy *= Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
	fp->ReadLine(tempBuf, 1024);
	sscanf(tempBuf, "%d", &spr->frameCount);
	
	spr->texCoords = new GLfloat*[spr->frameCount];
	spr->frameVertices = new GLfloat*[spr->frameCount];
//	spr->originalVertices = new float32*[spr->frameCount];
	spr->rectsAndOffsets = new float32*[spr->frameCount];
	spr->frameTextureIndex = new int32[spr->frameCount];
	
	
	for (int i = 0;	i < spr->frameCount; i++) 
	{
		spr->frameVertices[i] = new GLfloat[8];
//		spr->originalVertices[i] = new float32[4];
		spr->texCoords[i] = new GLfloat[8];
		spr->rectsAndOffsets[i] = new GLfloat[6];
		
		int32 x, y, dx,dy, xOff, yOff;
		
		fp->ReadLine(tempBuf, 1024);
		sscanf(tempBuf, "%d %d %d %d %d %d %d", &x, &y, &dx, &dy, &xOff, &yOff, &spr->frameTextureIndex[i]);
		
		spr->rectsAndOffsets[i][0] = (float32)x;
		spr->rectsAndOffsets[i][1] = (float32)y;
//		spr->rectsAndOffsets[i][2] = (float32)dx;
//		spr->rectsAndOffsets[i][3] = (float32)dy;
//		spr->rectsAndOffsets[i][4] = (float32)xOff;
//		spr->rectsAndOffsets[i][5] = (float32)yOff;
//		spr->originalVertices[i][0] = (float32)dx;
//		spr->originalVertices[i][1] = (float32)dy;
//		spr->originalVertices[i][2] = (float32)xOff;
//		spr->originalVertices[i][3] = (float32)yOff;
		
		spr->frameVertices[i][0] = (float32)xOff;
		spr->frameVertices[i][1] = (float32)yOff;
		spr->frameVertices[i][2] = (float32)(xOff + dx);
		spr->frameVertices[i][3] = (float32)yOff;
		spr->frameVertices[i][4] = (float32)xOff;
		spr->frameVertices[i][5] = (float32)(yOff + dy);
		spr->frameVertices[i][6] = (float32)(xOff + dx);
		spr->frameVertices[i][7] = (float32)(yOff + dy);
		
		float xof = 0;
		float yof = 0;
		if (usedForScale)
		{
			xof = 0.15f + (0.45f - 0.15f) * (dx * 0.01f);
			yof = 0.15f + (0.45f - 0.15f) * (dy * 0.01f);
			if(xof > 0.45f)
			{
				xof = 0.45f;
			}
			if(yof > 0.45f)
			{
				yof = 0.45f;
			}
		}
		

		spr->rectsAndOffsets[i][2] = dx * Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
		spr->rectsAndOffsets[i][3] = dy * Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
		spr->rectsAndOffsets[i][4] = xOff * Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
		spr->rectsAndOffsets[i][5] = yOff * Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);

		dx += x;
		dy += y;
		
		spr->texCoords[i][0] = ((GLfloat)x + xof) / spr->textures[spr->frameTextureIndex[i]]->width;
		spr->texCoords[i][1] = ((GLfloat)y + yof) / spr->textures[spr->frameTextureIndex[i]]->height;
		spr->texCoords[i][2] = ((GLfloat)dx - xof) / spr->textures[spr->frameTextureIndex[i]]->width;
		spr->texCoords[i][3] = ((GLfloat)y + yof) / spr->textures[spr->frameTextureIndex[i]]->height;
		spr->texCoords[i][4] = ((GLfloat)x + xof) / spr->textures[spr->frameTextureIndex[i]]->width;
		spr->texCoords[i][5] = ((GLfloat)dy - yof) / spr->textures[spr->frameTextureIndex[i]]->height;
		spr->texCoords[i][6] = ((GLfloat)dx - xof) / spr->textures[spr->frameTextureIndex[i]]->width;
		spr->texCoords[i][7] = ((GLfloat)dy - yof) / spr->textures[spr->frameTextureIndex[i]]->height;
		
		spr->frameVertices[i][0] *= Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
		spr->frameVertices[i][1] *= Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
		spr->frameVertices[i][2] *= Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
		spr->frameVertices[i][3] *= Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
		spr->frameVertices[i][4] *= Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
		spr->frameVertices[i][5] *= Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
		spr->frameVertices[i][6] *= Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
		spr->frameVertices[i][7] *= Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
		
		
	}
//	Logger::Debug("Frames created: %d", spr->frameCount);
	//	center.x = width / 2;
	//	center.y = height / 2;
	
	spr->defaultPivotPoint.x = 0;
	spr->defaultPivotPoint.y = 0;
	
	spr->resourceToVirtualFactor = Core::Instance()->GetResourceToVirtualFactor(spr->resourceSizeIndex);
	spr->resourceToPhysicalFactor = Core::Instance()->GetResourceToPhysicalFactor(spr->resourceSizeIndex);


	SafeRelease(fp);
	timeSpriteRead2 = SystemTimer::Instance()->AbsoluteMS() - timeSpriteRead2;
	
	//Logger::Debug("Sprite: %s time:%lld", spr->relativePathname.c_str(), timeSpriteRead2 + timeSpriteRead);
	
//	Logger::Debug("Adding to map for key: %s", spr->relativePathname.c_str());
	spriteMap[spr->relativePathname] = spr;
//	Logger::Debug("Resetting sprite");
	spr->Reset();
//	Logger::Debug("Returning pointer");
	return spr;
}
	
Sprite* Sprite::Create(const String &spriteName)
{
	
	Sprite * spr = PureCreate(spriteName,NULL);
	if (!spr)
	{
		spr = CreateFromTexture(Vector2(16.f, 16.f), Texture::GetPinkPlaceholder(), Vector2(0.f, 0.f), Vector2(16.f, 16.f));
	}
	return spr;
}

Sprite* Sprite::CreateAsRenderTarget(float32 sprWidth, float32 sprHeight, PixelFormat textureFormat, bool contentScaleIncluded)
{
	Sprite * sprite = new Sprite();
	sprite->InitAsRenderTarget(sprWidth, sprHeight, textureFormat, contentScaleIncluded);
	return sprite;
}

void Sprite::InitAsRenderTarget(float32 sprWidth, float32 sprHeight, PixelFormat textureFormat, bool contentScaleIncluded)
{
	if (!contentScaleIncluded)
	{
		sprWidth = sprWidth * Core::GetVirtualToPhysicalFactor();
		sprHeight = sprHeight * Core::GetVirtualToPhysicalFactor();
	}

	Texture *t = Texture::CreateFBO((int32)ceilf(sprWidth), (int32)ceilf(sprHeight), textureFormat, Texture::DEPTH_NONE);
	
	this->InitFromTexture(t, 0, 0, sprWidth, sprHeight, -1, -1, true);
	
	t->Release();
	
	this->type = SPRITE_RENDER_TARGET;

	// Clear created render target first 
	RenderManager::Instance()->LockNonMain();
	RenderManager::Instance()->SetRenderTarget(this);
	RenderManager::Instance()->ClearWithColor(0, 0, 0, 0);
	RenderManager::Instance()->RestoreRenderTarget();
	RenderManager::Instance()->UnlockNonMain();
}
	
Sprite* Sprite::CreateFromTexture(Texture *fromTexture, int32 xOffset, int32 yOffset, float32 sprWidth, float32 sprHeight, bool contentScaleIncluded)
{
	DVASSERT(fromTexture);
	Sprite *spr = new Sprite();
	DVASSERT_MSG(spr, "Render Target Sprite Creation failed");
	spr->InitFromTexture(fromTexture, xOffset, yOffset, sprWidth, sprHeight, -1, -1, contentScaleIncluded);
	return spr;
}

Sprite * Sprite::CreateFromTexture(const Vector2 & spriteSize, Texture * fromTexture, const Vector2 & textureRegionOffset, const Vector2 & textureRegionSize)
{
	DVASSERT(fromTexture);
	Sprite *spr = new Sprite();
	DVASSERT_MSG(spr, "Render Target Sprite Creation failed");
	spr->InitFromTexture(fromTexture, (int32)textureRegionOffset.x, (int32)textureRegionOffset.y, textureRegionSize.x, textureRegionSize.y, (int32)spriteSize.x, (int32)spriteSize.y, false);
	return spr;
}

void Sprite::InitFromTexture(Texture *fromTexture, int32 xOffset, int32 yOffset, float32 sprWidth, float32 sprHeight, int32 targetWidth, int32 targetHeight, bool contentScaleIncluded)
{
	if (!contentScaleIncluded) 
	{
		xOffset = (int32)(Core::GetVirtualToPhysicalFactor() * xOffset);
		yOffset = (int32)(Core::GetVirtualToPhysicalFactor() * yOffset);
	}
	else 
	{
		sprWidth = Core::GetPhysicalToVirtualFactor() * sprWidth;
		sprHeight = Core::GetPhysicalToVirtualFactor() * sprHeight;
	}

    resourceToPhysicalFactor = Core::GetVirtualToPhysicalFactor();
	resourceToVirtualFactor = Core::GetPhysicalToVirtualFactor();
    resourceSizeIndex = Core::Instance()->GetDesirableResourceIndex();
	
	this->type = SPRITE_FROM_TEXTURE;
	this->textureCount = 1;
	this->textures = new Texture*[this->textureCount];
	
	this->textures[0] = SafeRetain(fromTexture);
	
//	int32 width = sprWidth;
	this->size.dx = (float32)sprWidth;

//	int32 height = sprHeight;
	this->size.dy = (float32)sprHeight;
	
//	Logger::Info("Init from texture: %.4fx%.4f", sprWidth, sprWidth);

//	this->originalSize = this->size;
	this->defaultPivotPoint.x = 0;
	this->defaultPivotPoint.y = 0;
	this->frameCount = 1;
	
	this->texCoords = new GLfloat*[this->frameCount];
	this->frameVertices = new GLfloat*[this->frameCount];
//	this->originalVertices = new GLfloat*[this->frameCount];
	this->rectsAndOffsets = new GLfloat*[this->frameCount];
	this->frameTextureIndex = new int32[this->frameCount];
	
	for (int i = 0;	i < this->frameCount; i++) 
	{
		this->frameVertices[i] = new GLfloat[8];
//		this->originalVertices[i] = new GLfloat[4];
		this->texCoords[i] = new GLfloat[8];
		this->rectsAndOffsets[i] = new GLfloat[6];
		this->frameTextureIndex[i] = 0;
		
		float32 x, y, dx,dy, xOff, yOff;
		x = (float32)xOffset;
		y = (float32)yOffset;
		dx = sprWidth * Core::GetVirtualToPhysicalFactor();
		dy = sprHeight * Core::GetVirtualToPhysicalFactor();
		xOff = 0;
		yOff = 0;
		
		this->rectsAndOffsets[i][0] = (float32)x;
		this->rectsAndOffsets[i][1] = (float32)y;
		this->rectsAndOffsets[i][2] = sprWidth;
		this->rectsAndOffsets[i][3] = sprHeight;
		this->rectsAndOffsets[i][4] = (float32)xOff;
		this->rectsAndOffsets[i][5] = (float32)yOff;

//		this->originalVertices[i][0] = (float32)dx;
//		this->originalVertices[i][1] = (float32)dy;
//		this->originalVertices[i][2] = (float32)xOff;
//		this->originalVertices[i][3] = (float32)yOff;
		
		this->frameVertices[i][0] = (float32)xOff;
		this->frameVertices[i][1] = (float32)yOff;
		this->frameVertices[i][2] = (float32)xOff + sprWidth;
		this->frameVertices[i][3] = (float32)yOff;
		this->frameVertices[i][4] = (float32)xOff;
		this->frameVertices[i][5] = (float32)(yOff + sprHeight);
		this->frameVertices[i][6] = (float32)(xOff + sprWidth);
		this->frameVertices[i][7] = (float32)(yOff + sprHeight);
		
		
		dx += x;
		dy += y;
		
		this->texCoords[i][0] = (GLfloat)x / this->textures[this->frameTextureIndex[i]]->width;
		this->texCoords[i][1] = (GLfloat)y / this->textures[this->frameTextureIndex[i]]->height;
		this->texCoords[i][2] = (GLfloat)dx / this->textures[this->frameTextureIndex[i]]->width;
		this->texCoords[i][3] = (GLfloat)y / this->textures[this->frameTextureIndex[i]]->height;
		this->texCoords[i][4] = (GLfloat)x / this->textures[this->frameTextureIndex[i]]->width;
		this->texCoords[i][5] = (GLfloat)dy / this->textures[this->frameTextureIndex[i]]->height;
		this->texCoords[i][6] = (GLfloat)dx / this->textures[this->frameTextureIndex[i]]->width;
		this->texCoords[i][7] = (GLfloat)dy / this->textures[this->frameTextureIndex[i]]->height;
		
	}
	
	
	this->relativePathname = Format("FBO sprite %d", fboCounter);
	spriteMap[this->relativePathname] = this;
	fboCounter++;
	this->Reset();
}
    
void Sprite::PrepareForTiling()
{
    if(!isPreparedForTiling)
    {
        for (int i = 0;	i < this->frameCount; i++) 
        {
            this->texCoords[i][0] += (1.0f/this->textures[this->frameTextureIndex[i]]->width); // x
            this->texCoords[i][1] += (1.0f/this->textures[this->frameTextureIndex[i]]->height); // y
            this->texCoords[i][2] -= (2.0f/this->textures[this->frameTextureIndex[i]]->width); // x+dx
            this->texCoords[i][3] += (1.0f/this->textures[this->frameTextureIndex[i]]->height); // y
            this->texCoords[i][4] += (1.0f/this->textures[this->frameTextureIndex[i]]->width); // x
            this->texCoords[i][5] -= (2.0f/this->textures[this->frameTextureIndex[i]]->height); // y+dy
            this->texCoords[i][6] -= (2.0f/this->textures[this->frameTextureIndex[i]]->width); // x+dx
            this->texCoords[i][7] -= (2.0f/this->textures[this->frameTextureIndex[i]]->height); // y+dy
        }
        isPreparedForTiling = true;
    }
}

void Sprite::SetOffsetsForFrame(int frame, float32 xOff, float32 yOff)
{
	DVASSERT(frame < frameCount);

	rectsAndOffsets[frame][4] = xOff;
	rectsAndOffsets[frame][5] = yOff;
	
//	originalVertices[frame][2] = xOff / Core::Instance()->GetResourceToVirtualFactor(resourceSizeIndex);
//	originalVertices[frame][3] = yOff / Core::Instance()->GetResourceToVirtualFactor(resourceSizeIndex);
	
	frameVertices[frame][0] = xOff;
	frameVertices[frame][1] = yOff;
	frameVertices[frame][2] = xOff + rectsAndOffsets[frame][2];
	frameVertices[frame][3] = yOff;
	frameVertices[frame][4] = xOff;
	frameVertices[frame][5] = yOff + rectsAndOffsets[frame][3];
	frameVertices[frame][6] = xOff + rectsAndOffsets[frame][2];
	frameVertices[frame][7] = yOff + rectsAndOffsets[frame][3];
}
	
int32 Sprite::Release()
{
	if(GetRetainCount() == 1)
	{
        SafeRelease(spriteRenderObject);
		spriteMap.erase(relativePathname);
	}
		
	return BaseObject::Release();
}
	
void Sprite::Clear()
{
	for (int k = 0; k < textureCount; ++k)
	{
		SafeRelease(textures[k]);
	}
	SafeDeleteArray(textures);
	
	if (frameVertices != 0)
	{
		for (int i = 0;	i < frameCount; i++) 
		{
			SafeDeleteArray(frameVertices[i]);
			//			SafeDeleteArray(originalVertices[i]);
			SafeDeleteArray(texCoords[i]);
			SafeDeleteArray(rectsAndOffsets[i]);
		}
	}
	//	if(maxCollisionPoints != 0)
	//	{
	//		for (int i = 0;	i < frameCount; i++) 
	//		{
	//			[collision[i] release];
	//		}
	//		SAFE_DELETE_ARRAY(collision);
	//	}
	SafeDeleteArray(frameVertices);
	//	SafeDeleteArray(originalVertices);
	SafeDeleteArray(texCoords);
	SafeDeleteArray(rectsAndOffsets);
	SafeDeleteArray(frameTextureIndex);
}

Sprite::~Sprite()
{
//	Logger::Info("Removing sprite");
	Clear();
		
}
	
Texture* Sprite::GetTexture()
{
	return textures[0];
}

Texture* Sprite::GetTexture(int32 frameNumber)
{
	DVASSERT(frameNumber > -1 && frameNumber < frameCount);
	return textures[frameTextureIndex[frame]];
}
	
float32 *Sprite::GetTextureVerts(int32 frame)
{
    return texCoords[frame];
}
    
int32 Sprite::GetFrameCount()
{
	return frameCount;	
}
	
float32 Sprite::GetWidth()
{
	return size.dx;
}
	
float32 Sprite::GetHeight()
{
	return size.dy;
}
	
const Vector2 &Sprite::GetSize()
{
	return size;
}
	
const Vector2 &Sprite::GetDefaultPivotPoint()
{
	return defaultPivotPoint;
}
	
void Sprite::SetFrame(int32 frm)
{
	frame = Max(0, Min(frm, frameCount - 1));	
}
	
void Sprite::SetDefaultPivotPoint(float32 x, float32 y)
{
	defaultPivotPoint.x = x;
	defaultPivotPoint.y = y;
	pivotPoint = defaultPivotPoint;
}
	
void Sprite::SetDefaultPivotPoint(const Vector2 &newPivotPoint)
{
	defaultPivotPoint = newPivotPoint;
	pivotPoint = defaultPivotPoint;
}
	
void Sprite::SetPivotPoint(float32 x, float32 y)
{
	pivotPoint.x = x;
	pivotPoint.y = y;
}
	
void Sprite::SetPivotPoint(const Vector2 &newPivotPoint)
{
	pivotPoint = newPivotPoint;
}
	
void Sprite::SetPosition(float32 x, float32 y)
{
	drawCoord.x = x;
	drawCoord.y = y;
}
	
void Sprite::SetPosition(const Vector2 &drawPos)
{
	drawCoord = drawPos;
}
		
	
void Sprite::SetAngle(float32 angleInRadians)
{
	rotateAngle = angleInRadians;
	if(angleInRadians != 0)
	{
		flags = flags | EST_ROTATE;
	}
	else
	{
		ResetAngle();
	}
}
	
void Sprite::SetScale(float32 xScale, float32 yScale)
{
	if(xScale != 1.f || yScale != 1.f)
	{
        scale.x = xScale;
        scale.y = yScale;

		flags = flags | EST_SCALE;
	}
	else
	{
		ResetScale();
	}
}
	
void Sprite::SetScale(const Vector2 &newScale)
{
	if(newScale.x != 1.f || newScale.y != 1.f)
	{
        scale = newScale;
		flags = flags | EST_SCALE;
	}
	else
	{
		ResetScale();
	}
}
	
void Sprite::SetScaleSize(float32 width, float32 height)
{
	if(width != size.dx || height != size.dy)
	{
		scale.x = width / size.dx;
		scale.y = height / size.dy;
		flags = flags | EST_SCALE;
	}
	else
	{
		ResetScale();
	}
}

void Sprite::SetScaleSize(const Vector2 &drawSize)
{
	SetScaleSize(drawSize.x, drawSize.y);
}

void Sprite::SetModification(int32 modif)
{
	modification = modif;
	if(modif != 0)
	{
		flags = flags | EST_MODIFICATION;
	}
	else 
	{
		ResetModification();
	}
}
	
void Sprite::Reset()
{
	drawCoord.x = 0;
	drawCoord.y = 0;
	frame = 0;
	flags = 0;
	rotateAngle = 0.0f;
	modification = 0;
	SetScale(1.0f, 1.0f);
	ResetPivotPoint();
	clipPolygon = 0;
}

void Sprite::ResetPivotPoint()
{
	pivotPoint = defaultPivotPoint;	
}
	
void Sprite::ResetAngle()
{
	flags = flags & ~EST_ROTATE;
}

void Sprite::ResetModification()
{
	flags = flags & ~EST_MODIFICATION;
}
	
void Sprite::ResetScale()
{
	scale.x = 1.f;
	scale.y = 1.f;
	flags = flags & ~EST_SCALE;
}
    
inline void Sprite::PrepareSpriteRenderData(Sprite::DrawState * state)
{
    float32 x, y;
    
    if(state)
    {
        flags = 0;
        if (state->flags != 0)
        {
            flags |= EST_MODIFICATION;
        }

        if(state->scale.x != 1.f || state->scale.y != 1.f)
        {
            flags |= EST_SCALE;
            scale.x = state->scale.x;
            scale.y = state->scale.y;
        }

        if(state->angle != 0.f) flags |= EST_ROTATE; 
            
        frame = Max(0, Min(state->frame, frameCount - 1));	
        
        x = state->position.x - state->pivotPoint.x * state->scale.x;
        y = state->position.y - state->pivotPoint.y * state->scale.y;
    }
    else
    {
       	x = drawCoord.x - pivotPoint.x * scale.x;
        y = drawCoord.y - pivotPoint.y * scale.y;
    }
        
    if(flags & EST_MODIFICATION)
	{
		if((modification & (ESM_HFLIP | ESM_VFLIP)) == (ESM_HFLIP | ESM_VFLIP))
		{
			if(flags & EST_SCALE)
			{
				x += (size.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2) * scale.x;
				y += (size.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2) * scale.y;
				tempVertices[0] = frameVertices[frame][6] * scale.x + x;
				tempVertices[1] = frameVertices[frame][7] * scale.y + y;
				tempVertices[2] = frameVertices[frame][4] * scale.x + x;
				tempVertices[3] = frameVertices[frame][5] * scale.y + y;
				tempVertices[4] = frameVertices[frame][2] * scale.x + x;
				tempVertices[5] = frameVertices[frame][3] * scale.y + y;
				tempVertices[6] = frameVertices[frame][0] * scale.x + x;
				tempVertices[7] = frameVertices[frame][1] * scale.y + y;
			}
			else 
			{
				x += (size.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2);
				y += (size.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2);
				tempVertices[0] = frameVertices[frame][6] + x;
				tempVertices[1] = frameVertices[frame][7] + y;
				tempVertices[2] = frameVertices[frame][4] + x;
				tempVertices[3] = frameVertices[frame][5] + y;
				tempVertices[4] = frameVertices[frame][2] + x;
				tempVertices[5] = frameVertices[frame][3] + y;
				tempVertices[6] = frameVertices[frame][0] + x;
				tempVertices[7] = frameVertices[frame][1] + y;
			}
		}
		else 
		{
			if(modification & ESM_HFLIP)
			{
				if(flags & EST_SCALE)
				{
					x += (size.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2) * scale.x;
					tempVertices[0] = frameVertices[frame][2] * scale.x + x;
					tempVertices[1] = frameVertices[frame][3] * scale.y + y;
					tempVertices[2] = frameVertices[frame][0] * scale.x + x;
					tempVertices[3] = frameVertices[frame][1] * scale.y + y;
					tempVertices[4] = frameVertices[frame][6] * scale.x + x;
					tempVertices[5] = frameVertices[frame][7] * scale.y + y;
					tempVertices[6] = frameVertices[frame][4] * scale.x + x;
					tempVertices[7] = frameVertices[frame][5] * scale.y + y;
				}
				else 
				{
					x += (size.dx - rectsAndOffsets[frame][2] - rectsAndOffsets[frame][4] * 2);
					tempVertices[0] = frameVertices[frame][2] + x;
					tempVertices[1] = frameVertices[frame][3] + y;
					tempVertices[2] = frameVertices[frame][0] + x;
					tempVertices[3] = frameVertices[frame][1] + y;
					tempVertices[4] = frameVertices[frame][6] + x;
					tempVertices[5] = frameVertices[frame][7] + y;
					tempVertices[6] = frameVertices[frame][4] + x;
					tempVertices[7] = frameVertices[frame][5] + y;
				}
			}
			else
			{
				if(flags & EST_SCALE)
				{
					y += (size.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2) * scale.y;
					tempVertices[0] = frameVertices[frame][4] * scale.x + x;
					tempVertices[1] = frameVertices[frame][5] * scale.y + y;
					tempVertices[2] = frameVertices[frame][6] * scale.x + x;
					tempVertices[3] = frameVertices[frame][7] * scale.y + y;
					tempVertices[4] = frameVertices[frame][0] * scale.x + x;
					tempVertices[5] = frameVertices[frame][1] * scale.y + y;
					tempVertices[6] = frameVertices[frame][2] * scale.x + x;
					tempVertices[7] = frameVertices[frame][3] * scale.y + y;
				}
				else 
				{
					y += (size.dy - rectsAndOffsets[frame][3] - rectsAndOffsets[frame][5] * 2);
					tempVertices[0] = frameVertices[frame][4] + x;
					tempVertices[1] = frameVertices[frame][5] + y;
					tempVertices[2] = frameVertices[frame][6] + x;
					tempVertices[3] = frameVertices[frame][7] + y;
					tempVertices[4] = frameVertices[frame][0] + x;
					tempVertices[5] = frameVertices[frame][1] + y;
					tempVertices[6] = frameVertices[frame][2] + x;
					tempVertices[7] = frameVertices[frame][3] + y;
				}
			}
		}
        
	}
	else 
	{
		if(flags & EST_SCALE)
		{
			tempVertices[0] = frameVertices[frame][0] * scale.x + x;
			tempVertices[1] = frameVertices[frame][1] * scale.y + y;
			tempVertices[2] = frameVertices[frame][2] * scale.x + x;
			tempVertices[3] = frameVertices[frame][3] * scale.y + y;
			tempVertices[4] = frameVertices[frame][4] * scale.x + x;
			tempVertices[5] = frameVertices[frame][5] * scale.y + y;
			tempVertices[6] = frameVertices[frame][6] * scale.x + x;
			tempVertices[7] = frameVertices[frame][7] * scale.y + y;
		}
		else 
		{
			if(state && state->usePerPixelAccuracy && !(flags & EST_ROTATE))
			{
				tempVertices[0] = tempVertices[4] = floorf((frameVertices[frame][0] + x) * Core::GetVirtualToPhysicalFactor() + 0.5f);//x1
				tempVertices[1] = tempVertices[3] = floorf((frameVertices[frame][1] + y) * Core::GetVirtualToPhysicalFactor() + 0.5f);//y1
				tempVertices[2] = tempVertices[6] = (frameVertices[frame][2] - frameVertices[frame][0]) * Core::GetVirtualToPhysicalFactor() + tempVertices[0];//x2
				tempVertices[5] = tempVertices[7] = (frameVertices[frame][5] - frameVertices[frame][1]) * Core::GetVirtualToPhysicalFactor() + tempVertices[1];//y2
			
				RenderManager::Instance()->SetPhysicalViewScale();
			}
			else
			{
				//135
				//			tempVertices[0] = frameVertices[frame][0] + x;//x1
				//			tempVertices[1] = frameVertices[frame][1] + y;//y1
				//			tempVertices[2] = frameVertices[frame][2] + x;//x2
				//			tempVertices[3] = frameVertices[frame][3] + y;//y1
				//			tempVertices[4] = frameVertices[frame][4] + x;//x1
				//			tempVertices[5] = frameVertices[frame][5] + y;//y2
				//			tempVertices[6] = frameVertices[frame][6] + x;//x2
				//			tempVertices[7] = frameVertices[frame][7] + y;//y2

				//134
				tempVertices[0] = tempVertices[4] = frameVertices[frame][0] + x;//x1
				tempVertices[5] = tempVertices[7] = frameVertices[frame][5] + y;//y2
				tempVertices[1] = tempVertices[3] = frameVertices[frame][1] + y;//y1
				tempVertices[2] = tempVertices[6] = frameVertices[frame][2] + x;//x2

				//136
				//			float x1 = frameVertices[frame][0] + x;//x1
				//			float y2 = frameVertices[frame][5] + y;//y2
				//			float y1 = frameVertices[frame][1] + y;//y2
				//			float x2 = frameVertices[frame][2] + x;//x2
				//			tempVertices[0] = x1;
				//			tempVertices[7] = y2;
				//			tempVertices[1] = y1;
				//			tempVertices[2] = x2;
				//			tempVertices[3] = y1;
				//			tempVertices[4] = x1;
				//			tempVertices[5] = y2;
				//			tempVertices[6] = x2;
			}
			
		}
        
	}
    
    if(!clipPolygon)
	{
        if(flags & EST_ROTATE)
        {
            //SLOW CODE
            //			glPushMatrix();
            //			glTranslatef(drawCoord.x, drawCoord.y, 0);
            //			glRotatef(RadToDeg(rotateAngle), 0.0f, 0.0f, 1.0f);
            //			glTranslatef(-drawCoord.x, -drawCoord.y, 0);
            //			RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
            //			glPopMatrix();
            
            if (state)
            {
                rotateAngle = state->angle;
                drawCoord.x = state->position.x;
                drawCoord.y = state->position.y;
            }
            
            // Optimized code
            float32 sinA = sinf(rotateAngle);
            float32 cosA = cosf(rotateAngle);
            for(int32 k = 0; k < 4; ++k)
            {
                float32 x = tempVertices[(k << 1)] - drawCoord.x;
                float32 y = tempVertices[(k << 1) + 1] - drawCoord.y;
                
                float32 nx = (x) * cosA  - (y) * sinA + drawCoord.x;
                float32 ny = (x) * sinA  + (y) * cosA + drawCoord.y;
                
                tempVertices[(k << 1)] = nx;
                tempVertices[(k << 1) + 1] = ny;
            }
        }
        
        vertexStream->Set(TYPE_FLOAT, 2, 0, tempVertices);
        texCoordStream->Set(TYPE_FLOAT, 2, 0, texCoords[frame]);
        primitiveToDraw = PRIMITIVETYPE_TRIANGLESTRIP;
        vertexCount = 4;
	}else 
    {	
        clippedVertices.clear();
        clippedTexCoords.clear();
        Texture * t = GetTexture(frame);
		float32 adjWidth = 1.f / t->width / resourceToVirtualFactor;
		float32 adjHeight = 1.f / t->height / resourceToVirtualFactor;
        
		for(int32 i = 0; i < clipPolygon->pointCount; ++i)
		{
            const Vector2 & pos = clipPolygon->points[i];
			clippedVertices.push_back(pos);
			clippedTexCoords.push_back(Vector2(texCoords[frame][0] + (pos.x - x) * adjWidth,
                                               texCoords[frame][1] + (pos.y - y) * adjHeight));
		}
        
        vertexStream->Set(TYPE_FLOAT, 2, 0, &clippedVertices.front());
        texCoordStream->Set(TYPE_FLOAT, 2, 0, &clippedTexCoords.front());      
        primitiveToDraw = PRIMITIVETYPE_TRIANGLEFAN;
        vertexCount = clipPolygon->pointCount;
	}

	DVASSERT(vertexStream->pointer != 0);
	DVASSERT(texCoordStream->pointer != 0);
}

void Sprite::Draw()
{
	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
	{
		return;
	}

    PrepareSpriteRenderData(0);
    
	RenderManager::Instance()->SetTexture(textures[frameTextureIndex[frame]]);
    RenderManager::Instance()->SetRenderData(spriteRenderObject);
    
    RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
        
    RenderManager::Instance()->DrawArrays(primitiveToDraw, 0, vertexCount);

	Reset();
}
	
void Sprite::Draw(DrawState * state)
{
	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
	{
		return;
	}

	if (state->usePerPixelAccuracy) 
		RenderManager::Instance()->PushMappingMatrix();

	PrepareSpriteRenderData(state);
	RenderManager::Instance()->SetTexture(textures[frameTextureIndex[frame]]);

	RenderManager::Instance()->SetRenderData(spriteRenderObject);
	RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
	RenderManager::Instance()->DrawArrays(primitiveToDraw, 0, vertexCount);

	if (state->usePerPixelAccuracy) 
		RenderManager::Instance()->PopMappingMatrix();

}
    
void Sprite::BeginBatching()
{
    
    
}

void Sprite::EndBatching()
{
    
    
}


void Sprite::DrawPoints(Vector2 *verticies, Vector2 *textureCoordinates)
{
    GLfloat tempCoord[8];
    Memcpy(tempCoord, texCoords[frame], sizeof(float32)*8);

    
    texCoords[frame][0] = textureCoordinates[0].x;
    texCoords[frame][1] = textureCoordinates[0].y;
    texCoords[frame][2] = textureCoordinates[1].x;
    texCoords[frame][3] = textureCoordinates[1].y;
    texCoords[frame][4] = textureCoordinates[2].x;
    texCoords[frame][5] = textureCoordinates[2].y;
    texCoords[frame][6] = textureCoordinates[3].x;
    texCoords[frame][7] = textureCoordinates[3].y;
    DrawPoints(verticies);
    
    Memcpy(texCoords[frame], tempCoord, sizeof(float32)*8);
}


void Sprite::DrawPoints(Vector2 *verticies)
{
	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
	{
		return;
	}

	float32 x = drawCoord.x;
	float32 y = drawCoord.y;
	if(flags & EST_SCALE)
	{
		x -= pivotPoint.x * scale.x;
		y -= pivotPoint.y * scale.y;
	}
	else 
	{
		x -= pivotPoint.x;
		y -= pivotPoint.y;
	}
	
	if (!textures)
	{
		RenderManager::Instance()->SetColor(1.0f, 0.0f, 1.0f, 1.0f);
		RenderHelper::Instance()->FillRect(Rect(drawCoord.x - 12, drawCoord.y - 12, 24.0f, 24.0f));		
		RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		return;
	}
	
	if(flags & EST_MODIFICATION)
	{
		if((modification & (ESM_HFLIP | ESM_VFLIP)) == (ESM_HFLIP | ESM_VFLIP))
		{
			if(flags & EST_SCALE)
			{
				tempVertices[0] = verticies[3].x * scale.x + x;
				tempVertices[1] = verticies[3].y * scale.y + y;
				tempVertices[2] = verticies[2].x * scale.x + x;
				tempVertices[3] = verticies[2].y * scale.y + y;
				tempVertices[4] = verticies[1].x * scale.x + x;
				tempVertices[5] = verticies[1].y * scale.y + y;
				tempVertices[6] = verticies[0].x * scale.x + x;
				tempVertices[7] = verticies[0].y * scale.y + y;
			}
			else 
			{
				tempVertices[0] = verticies[3].x + x;
				tempVertices[1] = verticies[3].y + y;
				tempVertices[2] = verticies[2].x + x;
				tempVertices[3] = verticies[2].y + y;
				tempVertices[4] = verticies[1].x + x;
				tempVertices[5] = verticies[1].y + y;
				tempVertices[6] = verticies[0].x + x;
				tempVertices[7] = verticies[0].y + y;
			}
		}
		else 
		{
			if(modification & ESM_HFLIP)
			{
				if(flags & EST_SCALE)
				{
					tempVertices[0] = verticies[1].x * scale.x + x;
					tempVertices[1] = verticies[1].y * scale.y + y;
					tempVertices[2] = verticies[0].x * scale.x + x;
					tempVertices[3] = verticies[0].y * scale.y + y;
					tempVertices[4] = verticies[3].x * scale.x + x;
					tempVertices[5] = verticies[3].y * scale.y + y;
					tempVertices[6] = verticies[2].x * scale.x + x;
					tempVertices[7] = verticies[2].y * scale.y + y;
				}
				else 
				{
					tempVertices[0] = verticies[1].x + x;
					tempVertices[1] = verticies[1].y + y;
					tempVertices[2] = verticies[0].x + x;
					tempVertices[3] = verticies[0].y + y;
					tempVertices[4] = verticies[3].x + x;
					tempVertices[5] = verticies[3].y + y;
					tempVertices[6] = verticies[2].x + x;
					tempVertices[7] = verticies[2].y + y;
				}
			}
			else
			{
				if(flags & EST_SCALE)
				{
					tempVertices[0] = verticies[2].x * scale.x + x;
					tempVertices[1] = verticies[2].y * scale.y + y;
					tempVertices[2] = verticies[3].x * scale.x + x;
					tempVertices[3] = verticies[3].y * scale.y + y;
					tempVertices[4] = verticies[0].x * scale.x + x;
					tempVertices[5] = verticies[0].y * scale.y + y;
					tempVertices[6] = verticies[1].x * scale.x + x;
					tempVertices[7] = verticies[1].y * scale.y + y;
				}
				else 
				{
					tempVertices[0] = verticies[2].x + x;
					tempVertices[1] = verticies[2].y + y;
					tempVertices[2] = verticies[3].x + x;
					tempVertices[3] = verticies[3].y + y;
					tempVertices[4] = verticies[0].x + x;
					tempVertices[5] = verticies[0].y + y;
					tempVertices[6] = verticies[1].x + x;
					tempVertices[7] = verticies[1].y + y;
				}
			}
		}
	}
	else 
	{
		if(flags & EST_SCALE)
		{
			tempVertices[0] = verticies[0].x * scale.x + x;
			tempVertices[1] = verticies[0].y * scale.y + y;
			tempVertices[2] = verticies[1].x * scale.x + x;
			tempVertices[3] = verticies[1].y * scale.y + y;
			tempVertices[4] = verticies[2].x * scale.x + x;
			tempVertices[5] = verticies[2].y * scale.y + y;
			tempVertices[6] = verticies[3].x * scale.x + x;
			tempVertices[7] = verticies[3].y * scale.y + y;
		}
		else 
		{
			tempVertices[0] = verticies[0].x + x;
			tempVertices[1] = verticies[0].y + y;
			tempVertices[2] = verticies[1].x + x;
			tempVertices[3] = verticies[1].y + y;
			tempVertices[4] = verticies[2].x + x;
			tempVertices[5] = verticies[2].y + y;
			tempVertices[6] = verticies[3].x + x;
			tempVertices[7] = verticies[3].y + y;
		}
		
	}
	
    vertexStream->Set(TYPE_FLOAT, 2, 0, tempVertices);
    texCoordStream->Set(TYPE_FLOAT, 2, 0, texCoords[frame]);
    
    primitiveToDraw = PRIMITIVETYPE_TRIANGLESTRIP;
    vertexCount = 4;
	
    if(flags & EST_ROTATE)
	{
        // Optimized code
        float32 sinA = sinf(rotateAngle);
        float32 cosA = cosf(rotateAngle);
        for(int32 k = 0; k < 4; ++k)
        {
            float32 x = tempVertices[(k << 1)] - drawCoord.x;
            float32 y = tempVertices[(k << 1) + 1] - drawCoord.y;
            
            float32 nx = (x) * cosA  - (y) * sinA + drawCoord.x;
            float32 ny = (x) * sinA  + (y) * cosA + drawCoord.y;
            
            tempVertices[(k << 1)] = nx;
            tempVertices[(k << 1) + 1] = ny;
        }
    }	

    RenderManager::Instance()->SetTexture(textures[frameTextureIndex[frame]]);
	RenderManager::Instance()->SetRenderData(spriteRenderObject);
	RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
	RenderManager::Instance()->DrawArrays(primitiveToDraw, 0, vertexCount);
}
	
float32 Sprite::GetRectOffsetValueForFrame(int32 frame, eRectsAndOffsets valueType)
{
	return rectsAndOffsets[frame][valueType];
}

void Sprite::PrepareForNewSize()
{
	int pos = (int)relativePathname.find(Core::Instance()->GetResourceFolder(Core::Instance()->GetBaseResourceIndex()));
	String scaledName = relativePathname.substr(0, pos) + Core::Instance()->GetResourceFolder(Core::Instance()->GetDesirableResourceIndex()) + relativePathname.substr(pos + Core::Instance()->GetResourceFolder(Core::Instance()->GetBaseResourceIndex()).length());
	
	Logger::Instance()->Debug("Seraching for file: %s", scaledName.c_str());
	
	
	File *fp = File::Create(scaledName, File::READ|File::OPEN);
	
	if (!fp)
	{
		Logger::Instance()->Debug("Can't find file: %s", scaledName.c_str());
		return;
	}
	SafeRelease(fp);

	Vector2 tempPivotPoint = defaultPivotPoint;
		
	Clear();
	Logger::Debug("erasing from sprite from map");
	spriteMap.erase(relativePathname);
	textures = 0;
	frameTextureIndex = 0;
	textureCount = 0;
	
	frameVertices = 0;
	texCoords = 0;
	rectsAndOffsets = 0;
	
	size.dx = 24;
	size.dy = 24;
	frameCount = 0;
	frame = 0;
	
	modification = 0;
	flags = 0;
	resourceSizeIndex = 0;
	
	clipPolygon = 0;
	
	resourceToVirtualFactor = 1.0f;
    resourceToPhysicalFactor = 1.0f;
    
	PureCreate(relativePathname.substr(0, relativePathname.length() - 4), this);
//TODO: следующая строка кода написада здесь только до тех времен 
//		пока defaultPivotPoint не начнет задаваться прямо в спрайте,
//		но возможно это навсегда.
	defaultPivotPoint = tempPivotPoint;
}

void Sprite::ValidateForSize()
{
	Logger::Debug("--------------- Sprites validation for new resolution ----------------");
	List<Sprite*> spritesToReload;
	for(Map<String, Sprite*>::iterator it = spriteMap.begin(); it != spriteMap.end(); ++it)
	{
		Sprite *sp = it->second;
		if (sp->type == SPRITE_FROM_FILE && Core::Instance()->GetDesirableResourceIndex() != sp->GetResourceSizeIndex())
		{
			spritesToReload.push_back(sp);
		}
	}
	for(List<Sprite*>::iterator it = spritesToReload.begin(); it != spritesToReload.end(); ++it)
	{
		(*it)->PrepareForNewSize();
	}
	Logger::Debug("----------- Sprites validation for new resolution DONE  --------------");
//	Texture::DumpTextures();
}

	
void Sprite::DumpSprites()
{
	Logger::Info("============================================================");
	Logger::Info("--------------- Currently allocated sprites ----------------");
	for(Map<String, Sprite*>::iterator it = spriteMap.begin(); it != spriteMap.end(); ++it)
	{
		Sprite *sp = it->second; //[spriteDict objectForKey:[txKeys objectAtIndex:i]];
		Logger::Debug("name:%s count:%d size(%.0f x %.0f)", sp->relativePathname.c_str(), sp->GetRetainCount(), sp->size.dx, sp->size.dy);
	}
	Logger::Info("============================================================");
}

void Sprite::SetClipPolygon(Polygon2 * _clipPolygon)
{
	clipPolygon = _clipPolygon;
}

void Sprite::ConvertToVirtualSize()
{
    float32 virtualToPhysicalFactor = Core::Instance()->GetVirtualToPhysicalFactor();
    float32 resourceToVirtualFactor = Core::Instance()->GetResourceToVirtualFactor(GetResourceSizeIndex());
    
	frameVertices[0][0] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][1] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][2] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][3] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][4] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][5] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][6] *= virtualToPhysicalFactor * resourceToVirtualFactor;
	frameVertices[0][7] *= virtualToPhysicalFactor * resourceToVirtualFactor;
    
    texCoords[0][0] *= resourceToVirtualFactor;
    texCoords[0][1] *= resourceToVirtualFactor;
    texCoords[0][2] *= resourceToVirtualFactor;
    texCoords[0][3] *= resourceToVirtualFactor;
    texCoords[0][4] *= resourceToVirtualFactor;
    texCoords[0][5] *= resourceToVirtualFactor;
    texCoords[0][6] *= resourceToVirtualFactor;
    texCoords[0][7] *= resourceToVirtualFactor;
}

void Sprite::DrawState::BuildStateFromParentAndLocal(const Sprite::DrawState &parentState, const Sprite::DrawState &localState)
{
	position.x = parentState.position.x + localState.position.x * parentState.scale.x;
	position.y = parentState.position.y + localState.position.y * parentState.scale.y;
	if(parentState.angle != 0)
	{
		float tmpX = position.x;
		position.x = (tmpX - parentState.position.x) * parentState.cosA  + (parentState.position.y - position.y) * parentState.sinA + parentState.position.x;
		position.y = (tmpX - parentState.position.x) * parentState.sinA  + (position.y - parentState.position.y) * parentState.cosA + parentState.position.y;
	}
	scale.x = localState.scale.x * parentState.scale.x;
	scale.y = localState.scale.y * parentState.scale.y;
	angle = localState.angle + parentState.angle;
	if(angle != precomputedAngle)	// compute precomputed angle and store values
	{
		precomputedAngle = angle;
		if(precomputedAngle != parentState.angle)
		{
			cosA = cosf(precomputedAngle);
			sinA = sinf(precomputedAngle);
		}
		else 
		{
			cosA = parentState.cosA;
			sinA = parentState.sinA;
		}
	}
	pivotPoint.x = localState.pivotPoint.x;
	pivotPoint.y = localState.pivotPoint.y;
	
	frame = localState.frame;
}


};