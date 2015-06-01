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
=====================================================================================*/
#include "Render/ShaderCache.h"
#include "Render/RHI/rhi_ShaderCache.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
namespace ShaderDescriptorCache
{
struct ShaderSourceCode
{
    char8* vertexProgText;
    char8* fragmentProgText;    

    FilePath vertexProgSourcePath;
    FilePath fragmentProgSourcePath;
};
    
namespace
{
    Map<Vector<int32>, ShaderDescriptor *> shaderDescriptors;
    Map<FastName, ShaderSourceCode> shaderSourceCodes;
    bool initialized = false;
}
        

void Initialize()
{
    DVASSERT(!initialized);
    initialized = true;
}

void Uninitialize()
{
    DVASSERT(initialized);
    Clear();
    initialized = false;
}

void Clear()
{
    //RHI_COMPLETE - clear shader descriptors here too?
    DVASSERT(initialized);
    for (auto &it : shaderSourceCodes)
    {        
        SafeDelete(it.second.vertexProgText);
        SafeDelete(it.second.fragmentProgText);
    }
    shaderSourceCodes.clear();

}

void ClearDynamicBindigs()
{
    DVASSERT(initialized);
    for (auto &it : shaderDescriptors)
    {
        it.second->ClearDynamicBindings();
    }
}

void BuildFlagsKey(const FastName& name,const HashMap<FastName, int32>& defines, Vector<int32>& key)
{    
    key.clear();
    key.reserve(defines.size() * 2 + 1);
    for (auto& define : defines)
    {       
        key.push_back(define.first.Index());
        key.push_back(define.second);
    }
    key.push_back(name.Index());
}

ShaderSourceCode LoadFromSource(const String& source)
{
    ShaderSourceCode sourceCode;
    sourceCode.vertexProgSourcePath = FilePath(source + "-vp.cg");
    sourceCode.fragmentProgSourcePath = FilePath(source + "-fp.cg");
    
    //later move it into FileSystem

    //vertex
    File * fp = File::Create(sourceCode.vertexProgSourcePath, File::OPEN | File::READ);
    if (fp)
    {
        uint32 fileSize = fp->GetSize();
        sourceCode.vertexProgText = new char8[fileSize + 1];
        sourceCode.vertexProgText[fileSize] = 0;
        uint32 dataRead = fp->Read((uint8*)sourceCode.vertexProgText, fileSize);
        if (dataRead != fileSize)
        {
            Logger::Error("Failed to open vertex shader source file: %s", sourceCode.vertexProgSourcePath.GetAbsolutePathname().c_str());
        }
    }
    else
    {
        Logger::Error("Failed to open vertex shader source file: %s", sourceCode.vertexProgSourcePath.GetAbsolutePathname().c_str());
    }
    SafeRelease(fp);

    //fragment
    fp = File::Create(sourceCode.fragmentProgSourcePath, File::OPEN | File::READ);
    if (fp)
    {
        uint32 fileSize = fp->GetSize();
        sourceCode.fragmentProgText = new char8[fileSize + 1];
        sourceCode.fragmentProgText[fileSize] = 0;
        uint32 dataRead = fp->Read((uint8*)sourceCode.fragmentProgText, fileSize);
        if (dataRead != fileSize)
        {
            Logger::Error("Failed to open fragment shader source file: %s", sourceCode.fragmentProgSourcePath.GetAbsolutePathname().c_str());
        }
    }
    else
    {
        Logger::Error("Failed to open fragment shader source file: %s", sourceCode.fragmentProgSourcePath.GetAbsolutePathname().c_str());
    }
    SafeRelease(fp);

    return sourceCode;
}

ShaderDescriptor* GetShaderDescriptor(const FastName& name, const HashMap<FastName, int32>& defines)
{    
    DVASSERT(initialized);
 
    Vector<int32> key;
    BuildFlagsKey(name, defines, key);    

    auto descriptorIt = shaderDescriptors.find(key);
    if (descriptorIt != shaderDescriptors.end())
        return descriptorIt->second;

    //not found - create new shader
    Vector<String> progDefines;
    progDefines.reserve(defines.size() * 2);
    String resName(name.c_str());
    resName += "  defines: ";
    for (auto& it : defines)
    {
        progDefines.push_back(String(it.first.c_str()));
        progDefines.push_back(DAVA::Format("%d", it.second));                
        resName += Format("%s = %d, ", it.first.c_str(), it.second);
    }


    ShaderSourceCode sourceCode;
    auto sourceIt = shaderSourceCodes.find(name);
    if (sourceIt != shaderSourceCodes.end()) //source found
    {
        sourceCode = sourceIt->second;
    }
    else
    {
        sourceCode = LoadFromSource(name.c_str());
        //sourceCode = LoadFromSource("~res:/Materials/Shaders/Default/materials");
        shaderSourceCodes[name] = sourceCode;
    }

    rhi::ShaderSource vSource(sourceCode.vertexProgSourcePath.GetAbsolutePathname().c_str());
    rhi::ShaderSource fSource(sourceCode.fragmentProgSourcePath.GetAbsolutePathname().c_str());
    vSource.Construct(rhi::PROG_VERTEX, sourceCode.vertexProgText, progDefines);
    fSource.Construct(rhi::PROG_FRAGMENT, sourceCode.fragmentProgText, progDefines);    
    //vSource.Dump();
    //fSource.Dump();    
    
    FastName vProgUid, fProgUid;    
    vProgUid = FastName(String("vSource: ") + resName);
    fProgUid = FastName(String("fSource: ") + resName);

    rhi::ShaderCache::UpdateProg(rhi::HostApi(), rhi::PROG_VERTEX, vProgUid, vSource.SourceCode());
    rhi::ShaderCache::UpdateProg(rhi::HostApi(), rhi::PROG_FRAGMENT, fProgUid, fSource.SourceCode());

    rhi::PipelineState::Descriptor  psDesc;
    psDesc.vprogUid = vProgUid;
    psDesc.fprogUid = fProgUid;
    psDesc.vertexLayout = vSource.ShaderVertexLayout();
    psDesc.blending = fSource.Blending();
    rhi::HPipelineState piplineState = rhi::AcquireRenderPipelineState(psDesc);
    
    ShaderDescriptor *res = new ShaderDescriptor(&vSource, &fSource, piplineState);    
    res->sourceName = name;
    res->defines = defines;
    shaderDescriptors[key] = res;

    return res;
}

}
};

