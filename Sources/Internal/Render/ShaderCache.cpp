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
#include "Render/ShaderCache.h"
#include "FileSystem/FileSystem.h"
#include "Base/Data.h"
#include "Render/Shader.h"

namespace DAVA
{
    
    
ShaderAsset::ShaderAsset(Data * _vertexShaderData, Data * _fragmentShaderData)
{
    vertexShaderData = SafeRetain(_vertexShaderData);
    fragmentShaderData = SafeRetain(_fragmentShaderData);
}

ShaderAsset::~ShaderAsset()
{
    HashMap<FastNameSet, Shader *>::iterator end = compiledShaders.end();
    for (HashMap<FastNameSet, Shader *>::iterator it = compiledShaders.begin(); it != end; ++it)
    {
		Shader * shader = it->second;
        SafeRelease(shader);
    }
    
    SafeRelease(vertexShaderData);
    SafeRelease(fragmentShaderData);
}

Shader * ShaderAsset::Compile(const FastNameSet & defines)
{
    Shader * checkShader = compiledShaders.at(defines);
    if (checkShader)return checkShader;
    
    Shader * shader = Shader::CompileShader(vertexShaderData, fragmentShaderData, defines);

    compiledShaders.insert(defines, shader);
    return shader;
}
    
void ShaderAsset::Remove(const FastNameSet & defines)
{
    
}
    
Shader * ShaderAsset::Get(const FastNameSet & defines)
{
	/*String str;
	defines.ToString(str);
	
	if(dbgMap.find(str) == dbgMap.end())
	{
		dbgMap[str] = 0;
	}
	else
	{
		DVASSERT(compiledShaders.GetValue(defines));
	}*/
	
    Shader * shader = compiledShaders.at(defines);
    
    if (!shader)
    {
        shader = Compile(defines);
    }
    
    return shader;
}

    
ShaderCache::ShaderCache()
{
    
}
    
ShaderCache::~ShaderCache()
{
    FastNameMap<ShaderAsset*>::iterator end = shaderAssetMap.end();
    for (FastNameMap<ShaderAsset*>::iterator it = shaderAssetMap.begin(); it != end; ++it)
    {
		ShaderAsset * asset = it->second;
        SafeDelete(asset);
    }
}

    
ShaderAsset * ShaderCache::Load(const FastName & shaderFastName)
{
    ShaderAsset * checkAsset = shaderAssetMap.at(shaderFastName);
    DVASSERT(checkAsset == 0);

    String shader = shaderFastName.c_str();
    String vertexShaderPath = shader + ".vsh";
    String fragmentShaderPath = shader + ".fsh";
    
    uint32 vertexShaderSize = 0, fragmentShaderSize = 0;
    
    uint8 * vertexShaderBytes = FileSystem::Instance()->ReadFileContents(vertexShaderPath, vertexShaderSize);
    Data * vertexShaderData = new Data(vertexShaderBytes, vertexShaderSize);
    
    uint8 * fragmentShaderBytes = FileSystem::Instance()->ReadFileContents(fragmentShaderPath, fragmentShaderSize);
    Data * fragmentShaderData = new Data(fragmentShaderBytes, fragmentShaderSize);
    
    ShaderAsset * asset = new ShaderAsset(vertexShaderData, fragmentShaderData);

    SafeRelease(vertexShaderData);
    SafeRelease(fragmentShaderData);

    shaderAssetMap.Insert(shaderFastName, asset);
    return asset;
};


ShaderAsset * ShaderCache::Get(const FastName & shader)
{
    return shaderAssetMap.at(shader);
}
    
Shader * ShaderCache::Get(const FastName & shaderName, const FastNameSet & definesSet)
{
    ShaderAsset * asset = shaderAssetMap.at(shaderName);
    if (!asset)
    {
        asset = Load(shaderName);
    }
    Shader * shader = asset->Get(definesSet);
    return shader;
}

    

    
    
};