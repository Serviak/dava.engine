/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "TextureDescriptorTool.h"
#include "TextureDescriptorUtils.h"

#include "TexturePacker/CommandLineParser.h"

using namespace DAVA;


void TextureDescriptorTool::PrintUsage()
{
    printf("\n");
    printf("-texdescriptor [-resave] [-copycompression] [-create] [-folder [folder for action with descriptors]]\n");
	printf("-texdescriptor [-setcompression] [-file [descriptors folderPathname]] [-folder [folder with descriptors]] [-gpuName gpuValue] [-f]\n");
    printf("\tDo different operations with *.tex files\n");
    printf("\t-resave - resave all *.tex with new format\n");
    printf("\t-copycompression - copy compressionParams parameters from PowerVR_iOS to other gpus\n");
    printf("\t-create - create *.tex for *.png if need\n");
	printf("\t-setcompression - set compressionParams parameters for *tex or for all *text in folder. -f enables force mode\n");
    
    printf("\n");
    printf("Samples:\n");
    printf("-texdescriptor -resave -folder /Users/User/Project/DataSource/3d/\n");
    printf("-texdescriptor -copycompression -folder /Users/User/Project/DataSource/3d/\n");
    printf("-texdescriptor -create -folder /Users/User/Project/DataSource/3d/\n");
	printf("-texdescriptor -setcompression -file /Users/User/Project/DataSource/3d/Tanks/images/a-20.tex -PowerVR_iOS PVR4 -tegra DXT1 -mali ETC1 -adreno RGBA4444\n");
	printf("-texdescriptor -setcompression -folder /Users/User/Project/DataSource/3d/Tanks/images/-PowerVR_iOS PVR4 -tegra DXT1 -mali ETC1 -adreno RGBA4444 -f\n");
}

DAVA::String TextureDescriptorTool::GetCommandLineKey()
{
    return "-texdescriptor";
}

bool TextureDescriptorTool::InitializeFromCommandLine()
{
    commandAction = ACTION_NONE;
    
	folderPathname = ReadPathnameFromParams();
    if(folderPathname.IsEmpty())
    {
        errors.insert(Format("[TextureDescriptorTool]: Incorrect path parameter", folderPathname.GetAbsolutePathname().c_str()));
        return false;
    }
    
    if(CommandLineParser::CommandIsFound("-resave"))
    {
        commandAction = ACTION_RESAVE_DESCRIPTORS;
    }
    else if(CommandLineParser::CommandIsFound("-copycompression"))
    {
        commandAction = ACTION_COPY_COMPRESSION;
    }
    else if(CommandLineParser::CommandIsFound("-create"))
    {
        commandAction = ACTION_CREATE_DESCRIPTORS;
    }
	else if(CommandLineParser::CommandIsFound("-setcompression"))
	{
		commandAction = ACTION_SET_COMPRESSION;

		compressionParams.clear();
		for(int32 i = GPU_POWERVR_IOS; i < GPU_FAMILY_COUNT; ++i)
		{
			eGPUFamily gpu = (eGPUFamily)i;
			String gpuFlag = "-" + GPUFamilyDescriptor::GetGPUName(gpu);
			if(CommandLineParser::CommandIsFound(gpuFlag))
			{
				String formatName = CommandLineParser::Instance()->GetParamForFlag(gpuFlag);

				TextureDescriptor::Compression compression;
				compression.format = Texture::GetPixelFormatByName(formatName);
				compression.compressToWidth = compression.compressToHeight = 0;

				compressionParams[gpu] = compression;
			}
		}
	}
    else
    {
        errors.insert("[TextureDescriptorTool]: action not set");
        return false;
    }
    
    return true;
}

void TextureDescriptorTool::Process()
{
    switch(commandAction)
    {
        case ACTION_RESAVE_DESCRIPTORS:
            TextureDescriptorUtils::ResaveDescriptors(folderPathname);
            break;
            
        case ACTION_COPY_COMPRESSION:
            TextureDescriptorUtils::CopyCompressionParams(folderPathname);
            break;
            
        case ACTION_CREATE_DESCRIPTORS:
            TextureDescriptorUtils::CreateDescriptors(folderPathname);
			break;

		case ACTION_SET_COMPRESSION:
			TextureDescriptorUtils::SetCompressionParams(folderPathname, compressionParams, false);
			break;

            
        default:
            Logger::Error("[TextureDescriptorTool::Process] Unhandled action!");
    }
}

DAVA::FilePath TextureDescriptorTool::ReadPathnameFromParams() const
{
	FilePath folder = CommandLineParser::GetCommandParam(String("-folder"));
	if(!folder.IsEmpty())
	{
		folder.MakeDirectoryPathname();
		return folder;
	}

	return CommandLineParser::GetCommandParam(String("-file"));
}


