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


#include "Render/TextureDescriptor.h"
#include "FileSystem/Logger.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "Render/Texture.h"
#include "Utils/Utils.h"
#include "Render/LibPVRHelper.h"
#include "Render/LibDxtHelper.h"

#include "Render/GPUFamilyDescriptor.h"

#include "Utils/CRC32.h"
#include "Utils/Utils.h"

namespace DAVA
{
    
//================   TextureDrawSettings  ===================
void TextureDescriptor::TextureDrawSettings::SetDefaultValues()
{
    wrapModeS = Texture::WRAP_REPEAT;
    wrapModeT = Texture::WRAP_REPEAT;

    minFilter = Texture::FILTER_LINEAR_MIPMAP_LINEAR;
    magFilter = Texture::FILTER_LINEAR;
}
    
//================   TextureDataSettings  ===================
void TextureDescriptor::TextureDataSettings::SetDefaultValues()
{
	textureFlags = FLAG_GENERATE_MIPMAPS;
}

void TextureDescriptor::TextureDataSettings::SetGenerateMipmaps(const bool & generateMipmaps)
{
	EnableFlag(generateMipmaps, FLAG_GENERATE_MIPMAPS);
}

bool TextureDescriptor::TextureDataSettings::GetGenerateMipMaps() const
{
	return IsFlagEnabled(FLAG_GENERATE_MIPMAPS);
}

void TextureDescriptor::TextureDataSettings::EnableFlag( bool enable, int8 flag )
{
	if(enable)
	{
		textureFlags |= flag;
	}
	else
	{
		textureFlags &= ~flag;
	}
}

bool TextureDescriptor::TextureDataSettings::IsFlagEnabled( int8 flag ) const
{
	return ((textureFlags & flag) == flag);
}


//================   Compression  ===================
void TextureDescriptor::Compression::Clear()
{
    format = FORMAT_INVALID;
	sourceFileCrc = 0;
    convertedFileCrc = 0;

    compressToWidth = 0;
    compressToHeight = 0;
}
    
//================   TextureDescriptor  ===================
const String TextureDescriptor::DESCRIPTOR_EXTENSION = ".tex";
const String TextureDescriptor::SOURCEFILE_EXTENSION = ".png";


TextureDescriptor::TextureDescriptor(bool needCompressionSettings /*= true*/)
	: isCompressedFile(!needCompressionSettings)
	, compression(NULL)
{
	SetDefaultValues();
}

TextureDescriptor::~TextureDescriptor()
{
	if(isCompressedFile == false)
		ReleaseCompressionData();
}

TextureDescriptor * TextureDescriptor::CreateFromFile(const FilePath &filePathname)
{
	if(filePathname.IsEmpty() || filePathname.GetType() == FilePath::PATH_IN_MEMORY)
		return NULL;

	TextureDescriptor *descriptor = new TextureDescriptor();
	bool initialized = descriptor->Initialize(filePathname);
	if(!initialized)
	{
		Logger::Error("[TextureDescriptor::CreateFromFile(]: there are no descriptor file (%s).", filePathname.GetAbsolutePathname().c_str());
		delete descriptor;

		return NULL;
	}

    return descriptor;
}
    
TextureDescriptor * TextureDescriptor::CreateDescriptor(Texture::TextureWrap wrap, bool generateMipmaps)
{
    TextureDescriptor *descriptor = new TextureDescriptor();
	descriptor->Initialize(wrap, generateMipmaps);

	return descriptor;
}
    
    
void TextureDescriptor::SetDefaultValues()
{
	pathname = FilePath();
	format = FORMAT_INVALID;

//	isCompressedFile = false; not need reset this flag

	drawSettings.SetDefaultValues();
	dataSettings.SetDefaultValues();
	if(compression)
	{
		for(int32 i = 0; i < GPU_FAMILY_COUNT; ++i)
		{
			compression[i]->Clear();
		}
	}

	exportedAsGpuFamily = GPU_UNKNOWN;
}

void TextureDescriptor::SetQualityGroup(const FastName &group)
{
    qualityGroup = group;
}

FastName TextureDescriptor::GetQualityGroup() const
{
    return qualityGroup;
}
    
bool TextureDescriptor::IsCompressedTextureActual(eGPUFamily forGPU) const
{
    const Compression *compression = GetCompressionParams(forGPU);
	uint32 sourceCRC = ReadSourceCRC();
    uint32 convertedCRC = GetConvertedCRC(forGPU);
    
	return ((compression->sourceFileCrc == sourceCRC) && (compression->convertedFileCrc == convertedCRC));
}
    
bool TextureDescriptor::UpdateCrcForFormat(eGPUFamily forGPU) const
{
    bool wasUpdated = false;
    const Compression *compression = GetCompressionParams(forGPU);

	uint32 sourceCRC = ReadSourceCRC();
	if(compression->sourceFileCrc != sourceCRC)
	{
		compression->sourceFileCrc = sourceCRC;
		wasUpdated = true;
	}
    
    uint32 convertedCRC = GetConvertedCRC(forGPU);
	if(compression->convertedFileCrc != convertedCRC)
	{
		compression->convertedFileCrc = convertedCRC;
		wasUpdated = true;
	}
    
    return wasUpdated;
}
    
bool TextureDescriptor::Load(const FilePath &filePathname)
{
	DVASSERT(compression == NULL);

    File *file = File::Create(filePathname, File::READ | File::OPEN);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Load] Can't open file: %s", filePathname.GetAbsolutePathname().c_str());
        return false;
    }
    
	pathname = filePathname;

    int32 signature;
	ReadFromFile(file, &signature);

	isCompressedFile = (COMPRESSED_FILE == signature);
	if(isCompressedFile == false)
	{
		AllocateCompressionData();
	}

    int8 version = 0;
	ReadFromFile(file, &version);
    if(version != CURRENT_VERSION)
    {
        ConvertToCurrentVersion(version, signature, file);
        SafeRelease(file);
        return true;
    }
    
    if(isCompressedFile)
    {
        LoadCompressed(file);
    }
    else if(NOTCOMPRESSED_FILE == signature)
    {
        LoadNotCompressed(file);
    }
    else
    {
        Logger::Error("[TextureDescriptor::Load] Wrong descriptor file: %s", filePathname.GetAbsolutePathname().c_str());
        SafeRelease(file);
        return false;
    }
    
	ReadFromFile(file, &dataSettings.faceDescription);
	
    SafeRelease(file);
    return true;
}

void TextureDescriptor::Save() const
{
    DVASSERT_MSG(!pathname.IsEmpty(), "Can use this method only after calling Load()");
    Save(pathname);
}
    
void TextureDescriptor::Save(const FilePath &filePathname) const
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Save] Can't open file: %s", filePathname.GetAbsolutePathname().c_str());
        return;
    }
    
    int32 signature = NOTCOMPRESSED_FILE;
	WriteToFile(file, &signature);
    
    int8 version = CURRENT_VERSION;
	WriteToFile(file, &version);
    
    WriteGeneralSettings(file);
    
    //Compression
	DVASSERT(compression);
	for(int32 i = 0; i < GPU_FAMILY_COUNT; ++i)
	{
		WriteCompression(file, compression[i]);
	}
	
	WriteToFile(file, &dataSettings.faceDescription);
    
    SafeRelease(file);
}
  
void TextureDescriptor::Export(const FilePath &filePathname) const
{
    File *file = File::Create(filePathname, File::WRITE | File::OPEN | File::CREATE);
    if(!file)
    {
        Logger::Error("[TextureDescriptor::Export] Can't open file: %s", filePathname.GetAbsolutePathname().c_str());
        return;
    }

    int32 signature = COMPRESSED_FILE;
	WriteToFile(file, &signature);
    
    int8 version = CURRENT_VERSION;
	WriteToFile(file, &version);

    WriteGeneralSettings(file);
	WriteToFile(file, &exportedAsGpuFamily);
	int8 exportedAsPixelFormat = format;
	WriteToFile(file, &exportedAsPixelFormat);
	
	WriteToFile(file, &dataSettings.faceDescription);

    SafeRelease(file);
}
    
void TextureDescriptor::ConvertToCurrentVersion(int8 version, int32 signature, DAVA::File *file)
{
	if(version == 5)
    {
        LoadVersion5(signature, file);
    }
	else if(version == 6)
	{
		LoadVersion6(signature, file);
	}
}
    

void TextureDescriptor::LoadVersion5(int32 signature, DAVA::File *file)
{
	ReadFromFile(file, &drawSettings.wrapModeS);
	ReadFromFile(file, &drawSettings.wrapModeT);
	ReadFromFile(file, &dataSettings.textureFlags);
	ReadFromFile(file, &drawSettings.minFilter);
	ReadFromFile(file, &drawSettings.magFilter);

    if(signature == COMPRESSED_FILE)
	{
		exportedAsGpuFamily = GPU_UNKNOWN;
	}
	else if(signature == NOTCOMPRESSED_FILE)
	{
        int8 format;
		ReadFromFile(file, &format);

		if(format == FORMAT_ETC1)
		{
			Logger::Warning("[TextureDescriptor::LoadVersion5] format for pvr was ETC1");

			compression[GPU_POWERVR_IOS]->Clear();

			uint32 dummy32 = 0;
			ReadFromFile(file, &dummy32);
			ReadFromFile(file, &dummy32);
			ReadFromFile(file, &dummy32);
		}
		else
		{
			compression[GPU_POWERVR_IOS]->format = (PixelFormat)format;

			ReadFromFile(file, &compression[GPU_POWERVR_IOS]->compressToWidth);
			ReadFromFile(file, &compression[GPU_POWERVR_IOS]->compressToHeight);
			ReadFromFile(file, &compression[GPU_POWERVR_IOS]->sourceFileCrc);
		}

        file->Read(&format, sizeof(format));

		if(format == FORMAT_ATC_RGB || format == FORMAT_ATC_RGBA_EXPLICIT_ALPHA || format == FORMAT_ATC_RGBA_INTERPOLATED_ALPHA)
		{
			Logger::Warning("[TextureDescriptor::LoadVersion5] format for dds was ATC_...");

			compression[GPU_TEGRA]->Clear();

			uint32 dummy32 = 0;

			ReadFromFile(file, &dummy32);
			ReadFromFile(file, &dummy32);
			ReadFromFile(file, &dummy32);
		}
		else
		{
			compression[GPU_TEGRA]->format = (PixelFormat)format;

			ReadFromFile(file, &compression[GPU_TEGRA]->compressToWidth);
			ReadFromFile(file, &compression[GPU_TEGRA]->compressToHeight);
			ReadFromFile(file, &compression[GPU_TEGRA]->sourceFileCrc);
		}
	}
}

void TextureDescriptor::LoadVersion6(int32 signature, DAVA::File *file)
{
	ReadFromFile(file, &drawSettings.wrapModeS);
	ReadFromFile(file, &drawSettings.wrapModeT);
	ReadFromFile(file, &dataSettings.textureFlags);
	ReadFromFile(file, &drawSettings.minFilter);
	ReadFromFile(file, &drawSettings.magFilter);

    if(signature == COMPRESSED_FILE)
    {
		ReadFromFile(file, &exportedAsGpuFamily);
		int8 exportedAsPixelFormat = FORMAT_INVALID;
		ReadFromFile(file, &exportedAsPixelFormat);
		format = (PixelFormat)exportedAsPixelFormat;
    }
    else if(signature == NOTCOMPRESSED_FILE)
    {
        for(int32 i = 0; i < GPU_FAMILY_COUNT; ++i)
        {
            int8 format;
			ReadFromFile(file, &format);
            compression[i]->format = (PixelFormat)format;
            
			ReadFromFile(file, &compression[i]->compressToWidth);
			ReadFromFile(file, &compression[i]->compressToHeight);
			ReadFromFile(file, &compression[i]->sourceFileCrc);
        }
    }
}
    
void TextureDescriptor::LoadNotCompressed(File *file)
{
    ReadGeneralSettings(file);
    
	for(int32 i = 0; i < GPU_FAMILY_COUNT; ++i)
	{
		ReadCompression(file, compression[i]);
	}
}
    
void TextureDescriptor::LoadCompressed(File *file)
{
    ReadGeneralSettings(file);
	ReadFromFile(file, &exportedAsGpuFamily);
	int8 exportedAsPixelFormat = FORMAT_INVALID;
	ReadFromFile(file, &exportedAsPixelFormat);
	format = (PixelFormat)exportedAsPixelFormat;
}

void TextureDescriptor::ReadGeneralSettings(File *file)
{
	ReadFromFile(file, &drawSettings.wrapModeS);
	ReadFromFile(file, &drawSettings.wrapModeT);
	ReadFromFile(file, &dataSettings.textureFlags);
	ReadFromFile(file, &drawSettings.minFilter);
	ReadFromFile(file, &drawSettings.magFilter);
}
    
void TextureDescriptor::WriteGeneralSettings(File *file) const
{
	WriteToFile(file, &drawSettings.wrapModeS);
	WriteToFile(file, &drawSettings.wrapModeT);
	WriteToFile(file, &dataSettings.textureFlags);
	WriteToFile(file, &drawSettings.minFilter);
	WriteToFile(file, &drawSettings.magFilter);
}

    
void TextureDescriptor::ReadCompression(File *file, Compression *compression)
{
    int8 format;
	ReadFromFile(file, &format);
    compression->format = (PixelFormat)format;

	ReadFromFile(file, &compression->compressToWidth);
	ReadFromFile(file, &compression->compressToHeight);
	ReadFromFile(file, &compression->sourceFileCrc);
	ReadFromFile(file, &compression->convertedFileCrc);
}

void TextureDescriptor::ReadCompressionWithDateOld( File *file, Compression *compression )
{
	int8 format;
	ReadFromFile(file, &format);
	compression->format = (PixelFormat)format;

	ReadFromFile(file, &compression->compressToWidth);
	ReadFromFile(file, &compression->compressToHeight);

	// skip modification date
	file->Seek(DATE_BUFFER_SIZE * sizeof(char8), File::SEEK_FROM_CURRENT);

	// skip old crc
	file->Seek((MD5::DIGEST_SIZE*2 + 1) * sizeof(char8), File::SEEK_FROM_CURRENT);
	compression->sourceFileCrc = 0;
}

void TextureDescriptor::ReadCompressionWith16CRCOld( File *file, Compression *compression )
{
	int8 format;
	ReadFromFile(file, &format);
	compression->format = (PixelFormat)format;

	ReadFromFile(file, &compression->compressToWidth);
	ReadFromFile(file, &compression->compressToHeight);

	// skip old crc
	file->Seek((MD5::DIGEST_SIZE*2 + 1) * sizeof(char8), File::SEEK_FROM_CURRENT);
	compression->sourceFileCrc = 0;
}

void TextureDescriptor::WriteCompression(File *file, const Compression *compression) const
{
    int8 format = compression->format;

	WriteToFile(file, &format);
	WriteToFile(file, &compression->compressToWidth);
	WriteToFile(file, &compression->compressToHeight);
	WriteToFile(file, &compression->sourceFileCrc);
	WriteToFile(file, &compression->convertedFileCrc);
}

bool TextureDescriptor::GetGenerateMipMaps() const
{
    return dataSettings.GetGenerateMipMaps();
}
    
    
FilePath TextureDescriptor::GetSourceTexturePathname() const
{
    if(pathname.IsEmpty())
    {
        return FilePath();
    }

    return FilePath::CreateWithNewExtension(pathname, GetSourceTextureExtension());
}

FilePath TextureDescriptor::GetDescriptorPathname(const FilePath &texturePathname)
{
    DVASSERT(!texturePathname.IsEmpty());
    
    if(0 == CompareCaseInsensitive(texturePathname.GetExtension(), GetDescriptorExtension()))
    {
        return texturePathname;
    }
    
    return FilePath::CreateWithNewExtension(texturePathname, GetDescriptorExtension());
}


const String & TextureDescriptor::GetDescriptorExtension()
{
    return DESCRIPTOR_EXTENSION;
}
    
const String & TextureDescriptor::GetSourceTextureExtension()
{
    return SOURCEFILE_EXTENSION;
}
    
    
const TextureDescriptor::Compression * TextureDescriptor::GetCompressionParams(eGPUFamily gpuFamily) const
{
	DVASSERT(compression);

    DVASSERT(gpuFamily < GPU_FAMILY_COUNT);
    if(gpuFamily != GPU_UNKNOWN)
    {
        return compression[gpuFamily];
    }

    return NULL;
}

String TextureDescriptor::GetSupportedTextureExtensions()
{
    return String(".png;.pvr;.dxt;") + TextureDescriptor::GetDescriptorExtension();
}

bool TextureDescriptor::IsCompressedFile() const
{
    return isCompressedFile;
}

bool TextureDescriptor::IsCubeMap() const
{
	return (dataSettings.faceDescription != 0);
}
	
uint32 TextureDescriptor::ReadSourceCRC() const
{
	uint32 crc = 0;

	DAVA::File *f = DAVA::File::Create(GetSourceTexturePathname(), DAVA::File::OPEN | DAVA::File::READ);
	if(NULL != f)
	{
		uint8 buffer[8];

		// Read PNG header
		f->Read(buffer, 8);

		// read chunk header
		while (0 != f->Read(buffer, 8))
		{
			int32 chunk_size = 0;
			chunk_size |= (buffer[0] << 24);
			chunk_size |= (buffer[1] << 16);
			chunk_size |= (buffer[2] << 8);
			chunk_size |= buffer[3];

			// jump thought data
			DVASSERT(chunk_size >= 0);
			f->Seek(chunk_size, File::SEEK_FROM_CURRENT);

			// read crc
			f->Read(buffer, 4);
			crc += ((uint32 *) buffer)[0];
		}

		f->Release();
	}

	return crc;
}
    
uint32 TextureDescriptor::GetConvertedCRC(eGPUFamily forGPU) const
{
	FilePath filePath = GPUFamilyDescriptor::CreatePathnameForGPU(this, forGPU);
	if(filePath.IsEqualToExtension(".pvr"))
	{
		return LibPVRHelper::GetCRCFromFile(filePath) + GenerateDescriptorCRC();
	}
	else if(filePath.IsEqualToExtension(".dds"))
	{
		return LibDxtHelper::GetCRCFromFile(filePath) + GenerateDescriptorCRC();
	}
    DVASSERT(0);//converted means only pvr or dds
    return 0;
}

    
PixelFormat TextureDescriptor::GetPixelFormatForCompression(eGPUFamily forGPU) const
{
	DVASSERT(compression);

	if(forGPU == GPU_UNKNOWN)	
		return FORMAT_INVALID;

    DVASSERT(0 <= forGPU && forGPU < GPU_FAMILY_COUNT);
    return (PixelFormat) compression[forGPU]->format;
}

void TextureDescriptor::Initialize( Texture::TextureWrap wrap, bool generateMipmaps )
{
	SetDefaultValues();

	drawSettings.wrapModeS = wrap;
	drawSettings.wrapModeT = wrap;

	dataSettings.SetGenerateMipmaps(generateMipmaps);
	if(generateMipmaps)
	{
		drawSettings.minFilter = Texture::FILTER_LINEAR_MIPMAP_LINEAR;
		drawSettings.magFilter = Texture::FILTER_LINEAR;
	}
	else
	{
		drawSettings.minFilter = Texture::FILTER_LINEAR;
		drawSettings.magFilter = Texture::FILTER_LINEAR;
	}
}

void TextureDescriptor::Initialize( const TextureDescriptor *descriptor )
{
	if(!descriptor)
	{
		SetDefaultValues();
		return;
	}

	pathname = descriptor->pathname;

	drawSettings = descriptor->drawSettings;

	isCompressedFile = descriptor->isCompressedFile;
	if(descriptor->compression && !compression)
		AllocateCompressionData();
	else if(!descriptor->compression && compression)
	{
		ReleaseCompressionData();
	}

	if(compression && descriptor->compression)
	{
		for(uint32 i = 0; i < GPU_FAMILY_COUNT; ++i)
		{
			*compression[i] = *descriptor->compression[i];
		}
	}

    qualityGroup = descriptor->qualityGroup;
	exportedAsGpuFamily = descriptor->exportedAsGpuFamily;

	dataSettings.faceDescription = descriptor->dataSettings.faceDescription;
	format = descriptor->format;
}

bool TextureDescriptor::Initialize(const FilePath &filePathname)
{
	SetDefaultValues();

	FilePath descriptorPathname = GetDescriptorPathname(filePathname);
	return Load(descriptorPathname);
}

void TextureDescriptor::SetGenerateMipmaps( bool generateMipmaps )
{
	dataSettings.SetGenerateMipmaps(generateMipmaps);
}

void TextureDescriptor::AllocateCompressionData()
{
	DVASSERT(isCompressedFile == false);
	DVASSERT(compression == NULL);

	compression = new Compression *[GPU_FAMILY_COUNT];
	for(uint32 i = 0; i < GPU_FAMILY_COUNT; ++i)
	{
		compression[i] = new Compression();
	}
}

void TextureDescriptor::ReleaseCompressionData()
{
	DVASSERT(compression);

	if(compression)
	{
		for(uint32 i = 0; i < GPU_FAMILY_COUNT; ++i)
		{
			SafeDelete(compression[i]);
		}

		SafeDeleteArray(compression);
	}
}

void TextureDescriptor::Reload()
{
	if((pathname.IsEmpty() == false) && pathname.Exists())
	{
		if((isCompressedFile == false) && (compression))
			ReleaseCompressionData();

		FilePath savedPath = pathname;
		Initialize(savedPath);
	}
}

uint32 TextureDescriptor::GenerateDescriptorCRC() const
{
	static const uint32 CRC_BUFFER_SIZE = 16;
	static const uint8 CRC_BUFFER_VALUE = 0x3F; //to create nonzero buffer

	uint8 crcBuffer[CRC_BUFFER_SIZE];
	Memset(crcBuffer, CRC_BUFFER_VALUE, CRC_BUFFER_SIZE); 

	crcBuffer[0] = dataSettings.textureFlags;

	return CRC32::ForBuffer((const char8 *)crcBuffer, CRC_BUFFER_SIZE);
}


};
