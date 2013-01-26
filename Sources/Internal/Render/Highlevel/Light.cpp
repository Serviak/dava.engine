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

#include "Render/Highlevel/Light.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"


namespace DAVA 
{
    
REGISTER_CLASS(LightNode);

LightNode::LightNode()
:	BaseObject(),
	type(TYPE_DIRECTIONAL),
    ambientColor(0.0f, 0.0f, 0.0f, 1.0f),
	diffuseColor(1.0f, 1.0f, 1.0f, 1.0f),
    specularColor(1.0f, 1.0f, 1.0f, 1.0f),
    intensity(300.0f),
	isDynamic(true)
{
}
    
LightNode::~LightNode()
{
    
}
    
void LightNode::SetType(DAVA::LightNode::eType _type)
{
    type = _type;
}
    
void LightNode::SetAmbientColor(const Color & _color)
{
    ambientColor = _color;
}

void LightNode::SetDiffuseColor(const Color & _color)
{
    diffuseColor = _color;
}

void LightNode::SetSpecularColor(const Color & _color)
{
    specularColor = _color;
}

void LightNode::SetIntensity(float32 _intensity)
{
    intensity = _intensity;
}

    
BaseObject * LightNode::Clone(BaseObject *dstNode)
{
    if(!dstNode)
    {
		DVASSERT_MSG(IsPointerToExactClass<LightNode>(this), "Can clone only LightNode");
        dstNode = new LightNode();
    }
    
    //BaseObject::Clone(dstNode);
    
    LightNode *lightNode = (LightNode *)dstNode;
    lightNode->type = type;
    lightNode->ambientColor = ambientColor;
    lightNode->diffuseColor = diffuseColor;
    lightNode->specularColor = specularColor;
    
    return dstNode;
}

void LightNode::SetPositionDirectionFromMatrix(const Matrix4 & worldTransform)
{
    position = Vector3(0.0f, 0.0f, 0.0f) * worldTransform;
    direction = MultiplyVectorMat3x3(Vector3(0.0, -1.0f, 0.0f), worldTransform);
    direction.Normalize();
    
    RenderSystem::Instance()->MarkForUpdate(this);
}

LightNode::eType LightNode::GetType() const
{
    return type;
}
const Vector3 & LightNode::GetPosition() const
{
    return position; 
}

const Vector3 & LightNode::GetDirection() const
{
    return direction;
}
   
const Color & LightNode::GetAmbientColor() const
{
    return ambientColor;
}
    
const Color & LightNode::GetDiffuseColor() const
{
    return diffuseColor;
}
    
const Color & LightNode::GetSpecularColor() const
{
    return specularColor;
}
    
float32 LightNode::GetIntensity() const
{
    return intensity;
}

void LightNode::Save(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
	BaseObject::Save(archive);
	
	archive->SetInt32("type", type);
	archive->SetFloat("ambColor.r", ambientColor.r);
	archive->SetFloat("ambColor.g", ambientColor.g);
	archive->SetFloat("ambColor.b", ambientColor.b);
	archive->SetFloat("ambColor.a", ambientColor.a);

	archive->SetFloat("color.r", diffuseColor.r);
	archive->SetFloat("color.g", diffuseColor.g);
	archive->SetFloat("color.b", diffuseColor.b);
	archive->SetFloat("color.a", diffuseColor.a);

	archive->SetFloat("specColor.r", specularColor.r);
	archive->SetFloat("specColor.g", specularColor.g);
	archive->SetFloat("specColor.b", specularColor.b);
	archive->SetFloat("specColor.a", specularColor.a);
    
    archive->SetFloat("intensity", intensity);

}

void LightNode::Load(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
    BaseObject::Load(archive);

    type = (eType)archive->GetInt32("type");
    
    ambientColor.r = archive->GetFloat("ambColor.r", ambientColor.r);
    ambientColor.g = archive->GetFloat("ambColor.g", ambientColor.g);
    ambientColor.b = archive->GetFloat("ambColor.b", ambientColor.b);
    ambientColor.a = archive->GetFloat("ambColor.a", ambientColor.a);
    
    diffuseColor.r = archive->GetFloat("color.r", diffuseColor.r);
    diffuseColor.g = archive->GetFloat("color.g", diffuseColor.g);
    diffuseColor.b = archive->GetFloat("color.b", diffuseColor.b);
    diffuseColor.a = archive->GetFloat("color.a", diffuseColor.a);
    
    specularColor.r = archive->GetFloat("specColor.r", specularColor.r);
    specularColor.g = archive->GetFloat("specColor.g", specularColor.g);
    specularColor.b = archive->GetFloat("specColor.b", specularColor.b);
    specularColor.a = archive->GetFloat("specColor.a", specularColor.a);
    
    intensity = archive->GetFloat("intensity", intensity);

    
	//isDynamic = GetCustomProperties()->GetBool("editor.dynamiclight.enable", true);
}

// LIGHT
//void LightNode::Draw()
//{
//    SceneNode::Draw();
//}

bool LightNode::IsDynamic()
{
	return isDynamic;
}

void LightNode::SetDynamic(bool _isDynamic)
{
	isDynamic = _isDynamic;
}

};
