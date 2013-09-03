#include "PVRConverter.h"
#include "Render/TextureDescriptor.h"
#include "FileSystem/FileSystem.h"
#include "Utils/StringFormat.h"

#include "Render/GPUFamilyDescriptor.h"

namespace DAVA
{

static String CUBEMAP_TMP_DIR = "~doc:/ResourceEditor_Cubemap_Tmp/";
	
static String PVRTOOL_INPUT_NAMES[] =
{
	String("1"), //pz
	String("2"), //nz
	String("3"), //px
	String("4"), //nx
	String("5"), //pz
	String("6"), //nz
};
	
static DAVA::String PVRTOOL_MAP_NAMES[] =
{
	String("_pz"), //1
	String("_nz"), //2
	String("_px"), //3
	String("_nx"), //4
	String("_ny"), //5
	String("_py"), //6
};

PVRConverter::PVRConverter()
{
	// pvr map
	pixelFormatToPVRFormat[FORMAT_RGBA8888] = "OGL8888";
	pixelFormatToPVRFormat[FORMAT_RGBA4444] = "OGL4444";
	pixelFormatToPVRFormat[FORMAT_RGBA5551] = "OGL5551";
	pixelFormatToPVRFormat[FORMAT_RGB565] = "OGL565";
	pixelFormatToPVRFormat[FORMAT_RGB888] = "OGL888";
	pixelFormatToPVRFormat[FORMAT_PVR2] = "OGLPVRTC2";
	pixelFormatToPVRFormat[FORMAT_PVR4] = "OGLPVRTC4";
	pixelFormatToPVRFormat[FORMAT_A8] = "OGL8";
	pixelFormatToPVRFormat[FORMAT_ETC1] = "ETC";
	
	InitFileSuffixes();
}

PVRConverter::~PVRConverter()
{

}

FilePath PVRConverter::ConvertPngToPvr(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
	FilePath outputName = (descriptor.IsCubeMap()) ? PrepareCubeMapForPvrConvert(descriptor) : FilePath::CreateWithNewExtension(descriptor.pathname, ".png");
	String command = GetCommandLinePVR(descriptor, outputName, gpuFamily);
    Logger::Info("[PVRConverter::ConvertPngToPvr] (%s)", command.c_str());
    
	if(!command.empty())
	{
		FileSystem::Instance()->Spawn(command);		
		outputName = GetPVRToolOutput(descriptor, gpuFamily);
	}
	
	if(descriptor.IsCubeMap())
	{
		CleanupCubemapAfterConversion(descriptor);
	}

	return outputName;
}

String PVRConverter::GetCommandLinePVR(const TextureDescriptor &descriptor, FilePath fileToConvert, eGPUFamily gpuFamily)
{
	String command = "\"" + pvrTexToolPathname.GetAbsolutePathname() + "\"";
	String format = pixelFormatToPVRFormat[(PixelFormat) descriptor.compression[gpuFamily].format];

	if(command != "" && format != "")
	{
		FilePath outputFile = GetPVRToolOutput(descriptor, gpuFamily);

		// assemble command
		command += Format(" -i \"%s\"", fileToConvert.GetAbsolutePathname().c_str());

		if(descriptor.IsCubeMap())
		{
			command += " -s";
		}

		// output format
		command += Format(" -f%s", format.c_str());

		// pvr should be always flipped-y
		command += " -yflip0";

		// mipmaps
		if(descriptor.settings.generateMipMaps)
		{
			command += " -m";
		}

		// base mipmap level (base resize)
		if(0 != descriptor.compression[gpuFamily].compressToWidth && descriptor.compression[gpuFamily].compressToHeight != 0)
		{
			command += Format(" -x %d -y %d", descriptor.compression[gpuFamily].compressToWidth, descriptor.compression[gpuFamily].compressToHeight);
		}

		// output file
		command += Format(" -o \"%s\"", outputFile.GetAbsolutePathname().c_str());
	}
    else
    {
        Logger::Error("[PVRConverter::GetCommandLinePVR] Can't create command line for file with descriptor (%s)", descriptor.pathname.GetAbsolutePathname().c_str());
        command = "";
    }

	return command;
}

FilePath PVRConverter::GetPVRToolOutput(const TextureDescriptor &descriptor, eGPUFamily gpuFamily)
{
    return GPUFamilyDescriptor::CreatePathnameForGPU(&descriptor, gpuFamily);
}

void PVRConverter::SetPVRTexTool(const FilePath &textToolPathname)
{
	pvrTexToolPathname = textToolPathname;

	if(!FileSystem::Instance()->IsFile(pvrTexToolPathname))
	{
		Logger::Error("PVRTexTool doesn't found in %s\n", pvrTexToolPathname.GetAbsolutePathname().c_str());
		pvrTexToolPathname = FilePath();
	}
}

FilePath PVRConverter::PrepareCubeMapForPvrConvert(const TextureDescriptor& descriptor)
{
	DAVA::Vector<DAVA::String> pvrToolFaceNames;
	DAVA::Vector<DAVA::String> cubemapFaceNames;
	DAVA::Texture::GenerateCubeFaceNames(CUBEMAP_TMP_DIR, pvrToolSuffixes, pvrToolFaceNames);
	DAVA::Texture::GenerateCubeFaceNames(descriptor.pathname.GetAbsolutePathname(), cubemapSuffixes, cubemapFaceNames);
		
	DVASSERT(pvrToolSuffixes.size() == cubemapSuffixes.size());
		
	if(!FileSystem::Instance()->IsDirectory(CUBEMAP_TMP_DIR))
	{
		int createResult = FileSystem::Instance()->CreateDirectory(CUBEMAP_TMP_DIR);
		if(FileSystem::DIRECTORY_CREATED != createResult)
		{
			DAVA::Logger::Error("Failed to create temp dir for cubemap generation!");
			return FilePath();
		}
	}
		
	for(size_t i = 0; i < pvrToolFaceNames.size(); ++i)
	{
		//cleanup in case previous cleanup failed
		if(FileSystem::Instance()->IsFile(pvrToolFaceNames[i]))
		{
			FileSystem::Instance()->DeleteFile(pvrToolFaceNames[i]);
		}
			
		bool result = FileSystem::Instance()->CopyFile(cubemapFaceNames[i], pvrToolFaceNames[i]);
		if(!result)
		{
			DAVA::Logger::Error("Failed to copy tmp files for cubemap generation!");
			return FilePath();
		}
	}
		
	return FilePath(pvrToolFaceNames[0]);
}

void PVRConverter::CleanupCubemapAfterConversion(const TextureDescriptor& descriptor)
{
	Vector<String> pvrToolFaceNames;
	Texture::GenerateCubeFaceNames(CUBEMAP_TMP_DIR, pvrToolSuffixes, pvrToolFaceNames);
		
	for(size_t i = 0; i < pvrToolFaceNames.size(); ++i)
	{
		if(FileSystem::Instance()->IsFile(pvrToolFaceNames[i]))
		{
			FileSystem::Instance()->DeleteFile(pvrToolFaceNames[i]);
		}
	}
}
	
void PVRConverter::InitFileSuffixes()
{
	if(pvrToolSuffixes.empty())
	{
		for(int i = 0; i < DAVA::Texture::CUBE_FACE_MAX_COUNT; ++i)
		{
			pvrToolSuffixes.push_back(PVRTOOL_INPUT_NAMES[i]);
			cubemapSuffixes.push_back(PVRTOOL_MAP_NAMES[i]);
		}
	}
}	
};

