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

#ifndef __DAVAENGINE_LIGHT_NODE_H__
#define __DAVAENGINE_LIGHT_NODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"

//default direction (with identity matrix) is -y
namespace DAVA 
{
class SceneFileV2;
class LightNode : public BaseObject
{
public:
    enum eType
    {
        TYPE_DIRECTIONAL = 0,
        TYPE_SPOT,
        TYPE_POINT,
		TYPE_SKY,

		TYPE_COUNT
    };
    
    LightNode();
    virtual ~LightNode();
    
    virtual BaseObject * Clone(BaseObject * dstNode = NULL);

    void SetType(eType _type);
    void SetAmbientColor(const Color & _color);
    void SetDiffuseColor(const Color & _color);
    void SetSpecularColor(const Color & _color);
    void SetIntensity(float32 intensity);
    
    eType GetType() const;
    const Color & GetAmbientColor() const;
    const Color & GetDiffuseColor() const;    
    const Color & GetSpecularColor() const;    
    float32 GetIntensity() const;
    
    const Vector3 & GetPosition() const;
    const Vector3 & GetDirection() const;
    
    //virtual void Update(float32 timeElapsed);
    //virtual void Draw();
    
	virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFile);
	virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFile);

	bool IsDynamic();
	void SetDynamic(bool isDynamic);
    
protected:
    Vector3 position;
    Vector3 direction;
    
//    eType type; //TODO: waiting for enums at introspection
    uint8 type;
    
    Color ambientColor;
    Color diffuseColor;
    Color specularColor;
    float32 intensity;

	bool isDynamic;
    
public:
    
    INTROSPECTION_EXTEND(LightNode, BaseObject,
        MEMBER(position, "Position", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(direction, "Direction", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
                         
        MEMBER(type, "Type", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
                         
        MEMBER(ambientColor, "Ambient Color", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(diffuseColor, "Diffuse Color", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(specularColor, "Specular Color", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(intensity, "Intensity", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
        MEMBER(isDynamic, "Is Dynamic", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};

};

#endif //__DAVAENGINE_LIGHT_NODE_H__