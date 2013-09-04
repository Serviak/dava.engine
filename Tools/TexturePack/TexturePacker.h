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


#ifndef __TEXTURE_PACKER_H__
#define __TEXTURE_PACKER_H__

#include <string>
#include <list>
#include <vector>
#include "DefinitionFile.h"
#include "ImagePacker.h"


struct SizeSortItem
{
	int					imageSize;
	DefinitionFile *	defFile;
	int					frameIndex;
};

class TexturePacker 
{
public:
	TexturePacker();
	
	// pack textures to single texture
	void PackToTextures(const char * excludeFolder, const char* outputPath, std::list<DefinitionFile*> & defsList);
	// page each PSD file to separate texture
	void PackToTexturesSeparate(const char * excludeFolder, const char* outputPath, std::list<DefinitionFile*> & defsList);
	// pack one sprite and use several textures if more than one needed
	void PackToMultipleTextures(const char * excludeFolder, const char* outputPath, std::list<DefinitionFile*> & remainingList);

	bool TryToPack(const Rect2i & textureRect, std::list<DefinitionFile*> & defsList);
	bool WriteDefinition(const char * excludeFolder, const char * outputPath, const char * textureName, DefinitionFile * defFile);
	bool WriteMultipleDefinition(const char * excludeFolder, const char * outputPath, const char * _textureName, DefinitionFile * defFile);

	int TryToPackFromSortVector(ImagePacker * packer, std::vector<SizeSortItem> & tempSortVector);
	float TryToPackFromSortVectorWeight(ImagePacker * packer,std::vector<SizeSortItem> & tempSortVector);

	Rect2i ReduceRectToOriginalSize(const Rect2i & _input);
	
private:
	ImagePacker *			lastPackedPacker;
	std::vector<ImagePacker*> usedPackers;

	std::vector<SizeSortItem> sortVector;
	int32 maxTextureSize;
};

#endif __TEXTURE_PACKER_H__

