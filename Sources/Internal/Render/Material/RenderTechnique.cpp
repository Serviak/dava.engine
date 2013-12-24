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


#include "Render/Material/RenderTechnique.h"
#include "Render/ShaderCache.h"
#include "FileSystem/YamlParser.h"
#include "Base/HashMap.h"

namespace DAVA
{
    
RenderTechniquePass::RenderTechniquePass(const FastName & _shaderName,
										 const FastNameSet & _uniqueDefines,
										 RenderState * _renderState,
										 const FastNameSet & layers)
{
    shaderName = _shaderName;
    uniqueDefines = _uniqueDefines;
    renderState = _renderState;
	layersSet = layers;
}

RenderTechniquePass::~RenderTechniquePass()
{
    SafeDelete(renderState);
}

Shader * RenderTechniquePass::RetainShader(const FastNameSet & materialDefines)
{
    FastNameSet combinedDefines = materialDefines;
    if(uniqueDefines.size() > 0)
    {
        combinedDefines.Combine(uniqueDefines);
    }
    Shader * shader = SafeRetain(ShaderCache::Instance()->Get(shaderName, combinedDefines));
    return shader;
};
    
    
RenderTechnique::RenderTechnique(const FastName & _name)
    : name(_name)
{
}
    
RenderTechnique::~RenderTechnique()
{
}
    
void RenderTechnique::AddRenderTechniquePass(const DAVA::FastName &_shaderName,
                                             const DAVA::FastNameSet & _uniqueDefines,
                                             DAVA::RenderState *_renderState,
											 const DAVA::FastNameSet& layers)
{
    RenderTechniquePass * technique = new RenderTechniquePass(_shaderName,
															  _uniqueDefines,
															  _renderState,
															  layers);
    nameIndexMap[_shaderName] = (uint32)renderTechniqueArray.size();
    renderTechniqueArray.push_back(technique);
}

bool RenderTechniqueSingleton::LoadRenderTechniqueFromYamlNode(const YamlNode * rootNode, RenderTechnique * targetTechnique)
{
    bool result = false;
/*
    Move to shader level uniforms;
    const YamlNode * uniformsNode = stateNode->Get("Uniforms");
    if (uniformsNode)
    {
        uint32 count = uniformsNode->GetCount();
        for (uint32 k = 0; k < count; ++k)
        {
            const YamlNode * uniformNode = uniformsNode->Get(k);
            if (uniformNode)
            {
                //AddMaterialProperty(uniformsNode->GetItemKeyName(k), uniformNode);
            }
        }
    }
 */
    const YamlNode * stateNode = rootNode->Get("RenderTechnique");
    if (!stateNode)return false;
    
    uint32 techniqueCount = 0;
    for (int32 k = 0; k < stateNode->GetCount(); ++k)
    {
        const YamlNode * renderStepNode = stateNode->Get(k);
        
        if (renderStepNode->AsString() == "RenderPass")
        {
            const YamlNode * renderPassNameNode = renderStepNode->Get("Name");
            FastName renderPassName;
            if (renderPassNameNode)
            {
                renderPassName = FastName(renderPassNameNode->AsString());
            }
            
            Logger::FrameworkDebug("- RenderPass: %s", renderPassName.c_str());
            const YamlNode * shaderNode = renderStepNode->Get("Shader");
            const YamlNode * shaderGraphNode = renderStepNode->Get("ShaderGraph");
            
            if (!shaderNode && !shaderGraphNode)
            {
                Logger::Error("RenderPass:%s does not have shader or shader graph", renderStepNode->AsString().c_str());
                break;
            }
            
            FastName shaderName;
            if (shaderNode)
            {
                shaderName = FastName(shaderNode->AsString().c_str());
            }
            
            FastNameSet definesSet;
            const YamlNode * definesNode = renderStepNode->Get("UniqueDefines");
            if (definesNode)
            {
                int32 count = definesNode->GetCount();
                for (int32 k = 0; k < count; ++k)
                {
                    const YamlNode * singleDefineNode = definesNode->Get(k);
                    definesSet.Insert(FastName(singleDefineNode->AsString().c_str()));
                }
            }
            
            
            RenderState * renderState = new RenderState();
            if (renderStepNode)
            {
                renderState->LoadFromYamlNode(renderStepNode);
            }
			
			FastNameSet layersSet;
			const YamlNode * layersNode = renderStepNode->Get("Layers");
			if (layersNode)
			{
				int32 count = layersNode->GetCount();
				for (int32 k = 0; k < count; ++k)
				{
					const YamlNode * singleLayerNode = layersNode->Get(k);
					layersSet.Insert(FastName(singleLayerNode->AsString().c_str()));
				}
			}

            
            targetTechnique->AddRenderTechniquePass(shaderName,
													definesSet,
													renderState,
													layersSet);
            techniqueCount++;
        }
    }
    result = true;
    return result;
}

    
RenderTechnique * RenderTechniqueSingleton::RetainRenderTechniqueByName(const FastName & renderTechniquePathInFastName)
{
    FilePath renderTechniquePathname(renderTechniquePathInFastName.c_str());
    FastName renderTechniqueFastName(renderTechniquePathname.GetRelativePathname().c_str());
    //Logger::Debug("Get render technique: %s %d", renderTechniquePathname.GetRelativePathname().c_str(), renderTechniqueFastName.Index());
    
    RenderTechnique * renderTechnique = renderTechniqueMap.at(renderTechniqueFastName);
    if (!renderTechnique)
    {
		YamlParser * parser = YamlParser::Create(renderTechniquePathname);
		if (!parser)
		{
			Logger::Error("Can't load requested material: %s", renderTechniquePathname.GetRelativePathname().c_str());
			return 0;
		}
        Logger::Debug("Load render technique: %s", renderTechniquePathname.GetRelativePathname().c_str());
		YamlNode * rootNode = parser->GetRootNode();
        
        renderTechnique = new RenderTechnique(renderTechniqueFastName);
        LoadRenderTechniqueFromYamlNode(rootNode, renderTechnique);
        renderTechniqueMap.insert(renderTechniqueFastName, renderTechnique);
        
		if (!rootNode)
		{
			SafeRelease(parser);
			return 0;
		}
    }else
    {
        Logger::Debug("Get render technique: %s", renderTechnique->GetName().c_str());
    }
    return SafeRetain(renderTechnique);
}
    
void RenderTechniqueSingleton::ReleaseRenderTechnique(RenderTechnique * renderTechnique)
{
    SafeRelease(renderTechnique);
    if (renderTechnique->GetRetainCount() == 1)
    {
        SafeRelease(renderTechnique);
        renderTechniqueMap.erase(renderTechnique->GetName());
    }
}

};
