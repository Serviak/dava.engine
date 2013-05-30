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

#ifndef __DAVAENGINE_SCENE3D_LIGHT_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_LIGHT_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/Light.h"

namespace DAVA 
{

class LightComponent : public Component
{
public:
    LightComponent(Light * _light = 0);
    ~LightComponent();
    
    IMPLEMENT_COMPONENT_TYPE(LIGHT_COMPONENT);
    virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);

    void SetLightObject(Light * _light);
    Light * GetLightObject();
    
private:
    Light * light;
    
public:
    
    INTROSPECTION_EXTEND(LightComponent, Component,
        MEMBER(light, "Light", I_SAVE | I_VIEW | I_EDIT)
    );
};


};

#endif //__DAVAENGINE_SCENE3D_LIGHT_COMPONENT_H__
