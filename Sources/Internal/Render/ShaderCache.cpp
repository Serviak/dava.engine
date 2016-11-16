#include "Render/ShaderCache.h"
#include "Render/RHI/rhi_ShaderCache.h"
#include "FileSystem/FileSystem.h"
#include "Concurrency/LockGuard.h"

namespace DAVA
{
namespace ShaderDescriptorCache
{
struct ShaderSourceCode
{
    char8* vertexProgText = nullptr;
    char8* fragmentProgText = nullptr;

    FilePath vertexProgSourcePath;
    FilePath fragmentProgSourcePath;
};

namespace
{
Map<Vector<int32>, ShaderDescriptor*> shaderDescriptors;
Map<FastName, ShaderSourceCode> shaderSourceCodes;
Mutex shaderCacheMutex;
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

    LockGuard<Mutex> guard(shaderCacheMutex);

    for (auto& it : shaderSourceCodes)
    {
        SafeDeleteArray(it.second.vertexProgText);
        SafeDeleteArray(it.second.fragmentProgText);
    }
    shaderSourceCodes.clear();
}

void ClearDynamicBindigs()
{
    DVASSERT(initialized);

    LockGuard<Mutex> guard(shaderCacheMutex);

    for (auto& it : shaderDescriptors)
    {
        it.second->ClearDynamicBindings();
    }
}

Vector<int32> BuildFlagsKey(const FastName& name, const HashMap<FastName, int32>& defines)
{
    Vector<int32> key;

    key.clear();
    key.reserve(defines.size() * 2 + 1);
    for (auto& define : defines)
    {
        key.push_back(define.first.Index());
        key.push_back(define.second);
    }
    key.push_back(name.Index());

    return key;
}

ShaderSourceCode LoadFromSource(const String& source)
{
    ShaderSourceCode sourceCode;
    sourceCode.vertexProgSourcePath = FilePath(source + "-vp.sl");
    sourceCode.fragmentProgSourcePath = FilePath(source + "-fp.sl");

    //later move it into FileSystem

    //vertex
    bool loaded = true;
    File* fp = File::Create(sourceCode.vertexProgSourcePath, File::OPEN | File::READ);
    if (fp)
    {
        uint32 fileSize = static_cast<uint32>(fp->GetSize());
        sourceCode.vertexProgText = new char8[fileSize + 1];
        sourceCode.vertexProgText[fileSize] = 0;
        uint32 dataRead = fp->Read(reinterpret_cast<uint8*>(sourceCode.vertexProgText), fileSize);
        if (dataRead != fileSize)
        {
            SafeDeleteArray(sourceCode.vertexProgText);

            loaded = false;
            Logger::Error("Failed to open vertex shader source file: %s", sourceCode.vertexProgSourcePath.GetAbsolutePathname().c_str());
        }
    }
    else
    {
        loaded = false;
        Logger::Error("Failed to open vertex shader source file: %s", sourceCode.vertexProgSourcePath.GetAbsolutePathname().c_str());
    }
    SafeRelease(fp);

    if (!loaded)
    {
        sourceCode.vertexProgText = new char[1];
        sourceCode.vertexProgText[0] = 0;
    }

    //fragment
    loaded = true;
    fp = File::Create(sourceCode.fragmentProgSourcePath, File::OPEN | File::READ);
    if (fp)
    {
        uint32 fileSize = static_cast<uint32>(fp->GetSize());
        sourceCode.fragmentProgText = new char8[fileSize + 1];
        sourceCode.fragmentProgText[fileSize] = 0;
        uint32 dataRead = fp->Read(reinterpret_cast<uint8*>(sourceCode.fragmentProgText), fileSize);
        if (dataRead != fileSize)
        {
            SafeDeleteArray(sourceCode.fragmentProgText);
            loaded = false;
            Logger::Error("Failed to open fragment shader source file: %s", sourceCode.fragmentProgSourcePath.GetAbsolutePathname().c_str());
        }
    }
    else
    {
        loaded = false;
        Logger::Error("Failed to open fragment shader source file: %s", sourceCode.fragmentProgSourcePath.GetAbsolutePathname().c_str());
    }
    SafeRelease(fp);

    if (!loaded)
    {
        sourceCode.fragmentProgText = new char[1];
        sourceCode.fragmentProgText[0] = 0;
    }

    return sourceCode;
}

ShaderSourceCode GetSourceCode(const FastName& name)
{
    auto sourceIt = shaderSourceCodes.find(name);
    if (sourceIt != shaderSourceCodes.end()) //source found
    {
        return sourceIt->second;
    }
    else
    {
        ShaderSourceCode sourceCode = LoadFromSource(name.c_str());
        shaderSourceCodes[name] = sourceCode;
        return sourceCode;
    }
}

ShaderDescriptor* GetShaderDescriptor(const FastName& name, const HashMap<FastName, int32>& defines)
{
    DVASSERT(initialized);

    LockGuard<Mutex> guard(shaderCacheMutex);

    /*key*/
    Vector<int32> key = BuildFlagsKey(name, defines);

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
        bool doAdd = true;

        for (size_t i = 0; i != progDefines.size(); i += 2)
        {
            if (strcmp(it.first.c_str(), progDefines[i].c_str()) < 0)
            {
                progDefines.insert(progDefines.begin() + i, String(it.first.c_str()));
                progDefines.insert(progDefines.begin() + i + 1, DAVA::Format("%d", it.second));
                doAdd = false;
                break;
            }
        }

        if (doAdd)
        {
            progDefines.push_back(String(it.first.c_str()));
            progDefines.push_back(DAVA::Format("%d", it.second));
        }
    }

    for (size_t i = 0; i != progDefines.size(); i += 2)
        resName += Format("%s = %s, ", progDefines[i + 0].c_str(), progDefines[i + 1].c_str());

    FastName vProgUid, fProgUid;
    vProgUid = FastName(String("vSource: ") + resName);
    fProgUid = FastName(String("fSource: ") + resName);

    ShaderSourceCode sourceCode = GetSourceCode(name);
    const uint32 vSrcHash = HashValue_N(sourceCode.vertexProgText, static_cast<uint32>(strlen(sourceCode.vertexProgText)));
    const uint32 fSrcHash = HashValue_N(sourceCode.fragmentProgText, static_cast<uint32>(strlen(sourceCode.fragmentProgText)));
    const rhi::ShaderSource* vSource = rhi::ShaderSourceCache::Get(vProgUid, vSrcHash);
    const rhi::ShaderSource* fSource = rhi::ShaderSourceCache::Get(fProgUid, fSrcHash);

#define TRACE_CACHE_USAGE 0

    if (vSource)
    {
#if TRACE_CACHE_USAGE
        Logger::Info("using cached \"%s\"", vProgUid.c_str());
#endif
    }
    else
    {
#if TRACE_CACHE_USAGE
        Logger::Info("building \"%s\"", vProgUid.c_str());
#endif
        vSource = rhi::ShaderSourceCache::Add(sourceCode.vertexProgSourcePath.GetFrameworkPath().c_str(), vProgUid, rhi::PROG_VERTEX, sourceCode.vertexProgText, progDefines);

        if (!vSource)
        {
            Logger::Error("failed to construct \"%s\"", vProgUid.c_str());
            vSource = nullptr;
        }
    }

    if (fSource)
    {
#if TRACE_CACHE_USAGE
        Logger::Info("using cached \"%s\"", fProgUid.c_str());
#endif
    }
    else
    {
#if TRACE_CACHE_USAGE
        Logger::Info("building \"%s\"", fProgUid.c_str());
#endif
        fSource = rhi::ShaderSourceCache::Add(sourceCode.fragmentProgSourcePath.GetFrameworkPath().c_str(), fProgUid, rhi::PROG_FRAGMENT, sourceCode.fragmentProgText, progDefines);

        if (!fSource)
        {
            Logger::Error("failed to construct \"%s\"", fProgUid.c_str());
        }
    }

#if 0
    Logger::Info("\n\n%s", vProgUid.c_str());
    vSource->Dump();
#endif
    const std::string& vpBin = vSource->GetSourceCode(rhi::HostApi());
    rhi::ShaderCache::UpdateProgBinary(rhi::HostApi(), rhi::PROG_VERTEX, vProgUid, vpBin.c_str(), unsigned(vpBin.length()));
#if 0
    Logger::Info("\n\n%s", fProgUid.c_str());
    fSource->Dump();
#endif
    const std::string& fpBin = fSource->GetSourceCode(rhi::HostApi());
    rhi::ShaderCache::UpdateProgBinary(rhi::HostApi(), rhi::PROG_FRAGMENT, fProgUid, fpBin.c_str(), unsigned(fpBin.length()));

    //ShaderDescr
    rhi::PipelineState::Descriptor psDesc;
    psDesc.vprogUid = vProgUid;
    psDesc.fprogUid = fProgUid;
    psDesc.vertexLayout = vSource->ShaderVertexLayout();
    psDesc.blending = fSource->Blending();
    rhi::HPipelineState piplineState = rhi::AcquireRenderPipelineState(psDesc);
    ShaderDescriptor* res = new ShaderDescriptor(piplineState, vProgUid, fProgUid);
    res->sourceName = name;
    res->defines = defines;
    res->valid = piplineState.IsValid(); //later add another conditions
    if (res->valid)
    {
        res->UpdateConfigFromSource(const_cast<rhi::ShaderSource*>(vSource), const_cast<rhi::ShaderSource*>(fSource));
        res->requiredVertexFormat = GetVertexLayoutRequiredFormat(psDesc.vertexLayout);
    }
    else
    {
        DAVA::Logger::Error("failed to get pipeline-state");
        DAVA::Logger::Info("  vprog-uid = %s", vProgUid.c_str());
        DAVA::Logger::Info("  fprog-uid = %s", fProgUid.c_str());
    }

    shaderDescriptors[key] = res;
    return res;
}

void ReloadShaders()
{
    DVASSERT(initialized);

    LockGuard<Mutex> guard(shaderCacheMutex);

    //clear cached source files
    for (auto& it : shaderSourceCodes)
    {
        SafeDeleteArray(it.second.vertexProgText);
        SafeDeleteArray(it.second.fragmentProgText);
    }
    shaderSourceCodes.clear();

    //reload shaders
    for (auto& shaderDescr : shaderDescriptors)
    {
        ShaderDescriptor* shader = shaderDescr.second;

        /*Sources*/
        ShaderSourceCode sourceCode = GetSourceCode(shader->sourceName);
        rhi::ShaderSource vSource(sourceCode.vertexProgSourcePath.GetFrameworkPath().c_str());
        rhi::ShaderSource fSource(sourceCode.fragmentProgSourcePath.GetFrameworkPath().c_str());
        Vector<String> progDefines;
        progDefines.reserve(shader->defines.size() * 2);
        for (auto& it : shader->defines)
        {
            progDefines.push_back(String(it.first.c_str()));
            progDefines.push_back(DAVA::Format("%d", it.second));
        }
        vSource.Construct(rhi::PROG_VERTEX, sourceCode.vertexProgText, progDefines);
        fSource.Construct(rhi::PROG_FRAGMENT, sourceCode.fragmentProgText, progDefines);

        const std::string& vpBin = vSource.GetSourceCode(rhi::HostApi());
        rhi::ShaderCache::UpdateProgBinary(rhi::HostApi(), rhi::PROG_VERTEX, shader->vProgUid, vpBin.c_str(), unsigned(vpBin.length()));
        const std::string& fpBin = fSource.GetSourceCode(rhi::HostApi());
        rhi::ShaderCache::UpdateProgBinary(rhi::HostApi(), rhi::PROG_FRAGMENT, shader->fProgUid, fpBin.c_str(), unsigned(fpBin.length()));

        //ShaderDescr
        rhi::PipelineState::Descriptor psDesc;
        psDesc.vprogUid = shader->vProgUid;
        psDesc.fprogUid = shader->fProgUid;
        psDesc.vertexLayout = vSource.ShaderVertexLayout();
        psDesc.blending = fSource.Blending();
        rhi::ReleaseRenderPipelineState(shader->piplineState);
        shader->piplineState = rhi::AcquireRenderPipelineState(psDesc);
        shader->valid = shader->piplineState.IsValid(); //later add another conditions
        if (shader->valid)
        {
            shader->UpdateConfigFromSource(&vSource, &fSource);
            shader->requiredVertexFormat = GetVertexLayoutRequiredFormat(psDesc.vertexLayout);
        }
        else
        {
            shader->requiredVertexFormat = 0;
        }
    }
}
}
};
