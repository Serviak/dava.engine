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


#include "DXTConverter.h"

#include "FileSystem/FilePath.h"
#include "Render/TextureDescriptor.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/LibDdsHelper.h"
#include "Render/GPUFamilyDescriptor.h"

namespace DAVA
{
    
FilePath DXTConverter::ConvertPngToDxt(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
    FilePath fileToConvert = FilePath::CreateWithNewExtension(descriptor.pathname, ".png");
    
    Vector<Image*> inputImages;
    ImageSystem::Instance()->Load(fileToConvert, inputImages, 0);
    if(inputImages.size() ==1 )
    {
        Image* image = inputImages[0];
        
        FilePath outputName = GetDXTOutput(descriptor, gpuFamily);
        
		DVASSERT(descriptor.compression);
		const TextureDescriptor::Compression * compression = descriptor.compression[gpuFamily];

        if((compression->compressToWidth != 0) && (compression->compressToHeight != 0))
        {
            Logger::Warning("[DXTConverter::ConvertPngToDxt] convert to compression size");
            image->ResizeImage(compression->compressToWidth, compression->compressToHeight);
        }
        
        Vector<Image*> imagesToSave;
        if(descriptor.dataSettings.GetGenerateMipMaps())
        {
            imagesToSave = image->CreateMipMapsImages();
        }
        else
        {
            imagesToSave.push_back(SafeRetain(image));
        }
        eErrorCode retCode = ImageSystem::Instance()->Save(outputName, imagesToSave, (PixelFormat) descriptor.compression[gpuFamily]->format);
        for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
        for_each(imagesToSave.begin(), imagesToSave.end(), SafeRelease<Image>);
        if(SUCCESS == retCode)
        {
            LibDdsHelper helper;
			helper.AddCRCIntoMetaData(outputName);
            return outputName;
        }
    }
    
    Logger::Error("[DXTConverter::ConvertPngToDxt] can't convert %s to DXT", fileToConvert.GetAbsolutePathname().c_str());
    return FilePath();
}
	
FilePath DXTConverter::ConvertCubemapPngToDxt(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
	FilePath fileToConvert = FilePath::CreateWithNewExtension(descriptor.pathname, ".png");
	
	Vector<Image*> inputImages;
	Vector<FilePath> faceNames;
	Texture::GenerateCubeFaceNames(descriptor.pathname, faceNames);
	for(size_t i = 0; i < faceNames.size(); ++i)
	{
        Vector<Image*> tempImages;
        ImageSystem::Instance()->Load(faceNames[i], tempImages);
		if(tempImages.size() == 1)
		{
			inputImages.push_back(tempImages[0]);
		}
		
		if(tempImages.size() != 1 ||
		   ((inputImages.size() > 0 && tempImages.size() > 0) &&
			(inputImages[0]->width != tempImages[0]->width || inputImages[0]->height != tempImages[0]->height)))
		{
			for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
			for_each(tempImages.begin(), tempImages.end(), SafeRelease<Image>);
			
			Logger::Error("[DXTConverter::ConvertCubemapPngToDxt] can't convert %s to cubemap DXT", fileToConvert.GetAbsolutePathname().c_str());
			return FilePath();
		}
	}
    
    
    if(inputImages.size() == DAVA::Texture::CUBE_FACE_MAX_COUNT)
    {
        FilePath outputName = GetDXTOutput(descriptor, gpuFamily);
        
		DVASSERT(descriptor.compression);
		const TextureDescriptor::Compression * compression = descriptor.compression[gpuFamily];


        if((compression->compressToWidth != 0) && (compression->compressToHeight != 0))
        {
            Logger::Warning("[DXTConverter::ConvertPngToDxt] convert to compression size");
			
			for(size_t i = 0; i < inputImages.size(); ++i)
			{
				inputImages[i]->ResizeImage(compression->compressToWidth, compression->compressToHeight);
			}
        }
		
        eErrorCode retCode = ImageSystem::Instance()->SaveAsCubeMap(outputName, inputImages, (PixelFormat) descriptor.compression[gpuFamily]->format);
        if(SUCCESS == retCode)
        {
            for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
            LibDdsHelper helper;
            helper.AddCRCIntoMetaData(outputName);
            return outputName;
        }
    }
    
    Logger::Error("[DXTConverter::ConvertCubemapPngToDxt] can't convert %s to cubemap DXT", fileToConvert.GetAbsolutePathname().c_str());
    
    for_each(inputImages.begin(), inputImages.end(), SafeRelease<Image>);
    return FilePath();	
}

FilePath DXTConverter::GetDXTOutput(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
    return GPUFamilyDescriptor::CreatePathnameForGPU(&descriptor, gpuFamily);
}

};

