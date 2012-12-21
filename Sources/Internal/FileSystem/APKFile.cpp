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
#include "FileSystem/APKFile.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/Android/CorePlatformAndroid.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/DynamicMemoryFile.h"
#include "FileSystem/ResourceArchive.h"

#include <android/asset_manager.h>

namespace DAVA
{

APKFile::APKFile()
{
    
}


File * APKFile::CreateFromAssets(const String &filePath, uint32 attributes)
{
    FileSystem * fileSystem = FileSystem::Instance();
	for (List<FileSystem::ResourceArchiveItem>::iterator ai = fileSystem->resourceArchiveList.begin();
         ai != fileSystem->resourceArchiveList.end(); ++ai)
	{
		FileSystem::ResourceArchiveItem & item = *ai;
        
		String filenamecpp = filePath;
        
		String::size_type pos = filenamecpp.find(item.attachPath);
		if (pos == 0)
		{
			String relfilename = filenamecpp.substr(item.attachPath.length());
			int32 size = item.archive->LoadResource(relfilename, 0);
			if ( size == -1 )
			{
				return 0;
			}
            
			uint8 * buffer = new uint8[size];
			item.archive->LoadResource(relfilename, buffer);
			DynamicMemoryFile * file =  DynamicMemoryFile::Create(buffer, size, attributes);
			return file;
		}
	}
    
    bool isDirectory = FileSystem::Instance()->IsDirectory(filePath);
    if(isDirectory)
    {
        Logger::Error("[APKFile::CreateFromAssets] Can't create file because of it is directory (%s)", filePath.c_str());
        return NULL;
    }
    
    
    CorePlatformAndroid *core = (CorePlatformAndroid *)Core::Instance();
    DVASSERT_MSG(core, "Need create core before loading of files");
    
    
    AAssetManager *assetManager = core->GetAssetManager();
    DVASSERT_MSG(assetManager, "Need setup assetManager on core creation");
    
    AAsset * asset = AAssetManager_open(assetManager, filePath.c_str(), AASSET_MODE_UNKNOWN);
    if(!asset)
    {
        Logger::Error("[APKFile::CreateFromAssets] Can't load asset for path %s", filePath.c_str());
        return NULL;
    }
    

    uint32 dataSize = AAsset_getLength(asset);
//    Logger::Debug("[APKFile::CreateFromAssets] fileSize is %d (%s)", dataSize, filePath.c_str());


    uint8 *data = new uint8[dataSize];
    
    uint32 readSize = AAsset_read(asset, data, dataSize * sizeof(uint8));
    AAsset_close(asset);

    DVASSERT_MSG(readSize == dataSize * sizeof(uint8), "Can't read full file");

    DynamicMemoryFile *fileInstance = DynamicMemoryFile::Create(data, readSize, File::READ);
    DVASSERT_MSG(fileInstance, "Can't create dynamic file from memory");
    return fileInstance;
}
    
};

#endif // __DAVAENGINE_ANDROID__



