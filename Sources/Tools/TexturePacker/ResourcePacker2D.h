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


#ifndef __DAVAENGINE_RESOURCEPACKER2D_H__
#define __DAVAENGINE_RESOURCEPACKER2D_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"
#include "TextureCompression/TextureConverter.h"
#include <atomic>

namespace DAVA
{

class DefinitionFile;
class YamlNode;

class ResourcePacker2D
{
public:
    typedef std::map<String, String> FILESMAP;
    ResourcePacker2D();
    // Packing of resources section
    void InitFolders(const FilePath & inputPath,const FilePath & outputPath);
    void PackResources(eGPUFamily forGPU);
    void RecalculateMD5ForOutputDir();
    bool IsMD5ChangedDir(const FilePath & processDirectoryPath, const FilePath & pathname, const String & psdName, bool isRecursive) const;
    bool IsMD5ChangedFile(const FilePath & processDirectoryPath, const FilePath & pathname, const String & psdName) const;
    
    DefinitionFile * ProcessPSD(const FilePath & processDirectoryPath, const FilePath & psdPathname, const String & psdName, bool twoSideMargin, uint32 texturesMargin);
    Vector<String> FetchFlags(const FilePath & flagsPathname);

    static String GetProcessFolderName();
    void SetConvertQuality(const TextureConverter::eConvertQuality quality);
    void SetRunning(bool arg);
    bool IsRunning() const;
public:
    
    FilePath inputGfxDirectory;
    FilePath outputGfxDirectory;
    FilePath excludeDirectory;
    String gfxDirName;
    
    bool isGfxModified;
    
    bool isLightmapsPacking;
    bool clearProcessDirectory;
    eGPUFamily requestedGPUFamily;
    TextureConverter::eConvertQuality quality;
    FILESMAP spriteFiles;

    const Set<String>& GetErrors() const;
    std::atomic_bool running;
protected:
    Set<String> errors;

    void AddError(const String& errorMsg);
    void RecursiveTreeWalk(const FilePath & inputPath, const FilePath & outputPath, const Vector<String> & flags = Vector<String>());
};

inline bool ResourcePacker2D::IsRunning() const
{
    return running;
}

};


#endif // __DAVAENGINE_RESOURCEPACKER2D_H__
