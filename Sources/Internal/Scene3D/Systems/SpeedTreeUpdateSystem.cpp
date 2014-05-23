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


#include "SpeedTreeUpdateSystem.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Systems/WindSystem.h"
#include "Scene3D/Systems/WaveSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Scene.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Utils/Random.h"
#include "Math/Math2D.h"

namespace DAVA
{

SpeedTreeUpdateSystem::SpeedTreeUpdateSystem(Scene * scene)
    :	SceneSystem(scene)
{
    RenderOptions * options = RenderManager::Instance()->GetOptions();
    options->AddObserver(this);
    isAnimationEnabled = options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS);

    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::WORLD_TRANSFORM_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SPEED_TREE_MAX_ANIMATED_LOD_CHANGED);
}

SpeedTreeUpdateSystem::~SpeedTreeUpdateSystem()
{
    DVASSERT(allTrees.size() == 0);

    RenderOptions * options = RenderManager::Instance()->GetOptions();
    options->RemoveObserver(this);
}

void SpeedTreeUpdateSystem::ImmediateEvent(Entity * entity, uint32 event)
{
	if(event == EventSystem::WORLD_TRANSFORM_CHANGED)
	{
        SpeedTreeComponent * component = GetSpeedTreeComponent(entity);
        if(component)
        {
            Matrix4 * wtMxPrt = GetTransformComponent(entity)->GetWorldTransformPtr();
            component->wtPosition = wtMxPrt->GetTranslationVector();
            wtMxPrt->GetInverse(component->wtInvMx);
        }
	}
    if(event == EventSystem::SPEED_TREE_MAX_ANIMATED_LOD_CHANGED)
    {
        UpdateAnimationFlag(entity);
    }
}

void SpeedTreeUpdateSystem::AddEntity(Entity * entity)
{
    SpeedTreeComponent * component = GetSpeedTreeComponent(entity);
    DVASSERT(component);
    component->leafTime = (float32)Random::Instance()->RandFloat(1000.f);
    allTrees.push_back(component);
}

void SpeedTreeUpdateSystem::RemoveEntity(Entity * entity)
{
    uint32 treeCount = allTrees.size();
    for(uint32 i = 0; i < treeCount; ++i)
    {
        if(allTrees[i]->entity == entity)
        {
            allTrees[i] = allTrees[treeCount-1];
            allTrees.pop_back();
            break;
        }
    }
}

void SpeedTreeUpdateSystem::UpdateAnimationFlag(Entity * entity)
{
    SpeedTreeObject * treeObject = DynamicTypeCheck<SpeedTreeObject*>(GetRenderObject(entity));
    SpeedTreeComponent * component = GetSpeedTreeComponent(entity);

    int32 lodIndex = isAnimationEnabled ? component->GetMaxAnimatedLOD() : -1;
    treeObject->UpdateAnimationFlag(lodIndex);
}

void SpeedTreeUpdateSystem::Process(float32 timeElapsed)
{
    if(!isAnimationEnabled)
        return;
    
    WindSystem * windSystem = GetScene()->windSystem;
    WaveSystem * waveSystem = GetScene()->waveSystem;

    //Update trees
    uint32 treeCount = allTrees.size();
    for(uint32 i = 0; i < treeCount; ++i)
    {
		SpeedTreeComponent * component = allTrees[i];
		SpeedTreeObject * treeObject = DynamicTypeCheck<SpeedTreeObject*>(GetRenderObject(component->GetEntity()));

        const Vector3 & treePosition = component->wtPosition;
        Vector3 wind3D = windSystem->GetWind(treePosition) + waveSystem->GetWaveDisturbance(treePosition);
        float32 leafForce = wind3D.Length();
        Vector2 windVec(wind3D.x, wind3D.y);

        float32 trunkAmplitude = component->GetTrunkOscillationAmplitude();
        float32 trunkSpring = component->GetTrunkOscillationSpringSqrt();
        component->oscVelocity += (windVec - component->oscOffset * trunkSpring * trunkSpring - trunkSpring * component->oscVelocity.Length() * component->oscVelocity) * timeElapsed;
        component->oscOffset += component->oscVelocity * timeElapsed;
        
        component->leafTime += timeElapsed * sqrtf(leafForce) * component->GetLeafsOscillationSpeed();

        float32 sine, cosine;
        SinCosFast(component->leafTime, sine, cosine);
        float32 leafsOscillationAmplitude = component->GetLeafsOscillationApmlitude();
        Vector2 leafOscillationParams(leafsOscillationAmplitude * sine, leafsOscillationAmplitude * cosine);
        
		Vector2 localOffset = MultiplyVectorMat2x2XY(component->oscOffset * component->GetTrunkOscillationAmplitude(), component->wtInvMx);
        treeObject->SetTreeAnimationParams(localOffset, leafOscillationParams);
    }
}

void SpeedTreeUpdateSystem::HandleEvent(Observable * observable)
{
    RenderOptions * options = static_cast<RenderOptions *>(observable);
    
    if(isAnimationEnabled != options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS))
    {
        isAnimationEnabled = options->IsOptionEnabled(RenderOptions::SPEEDTREE_ANIMATIONS);
        
        if(!isAnimationEnabled)
        {
            uint32 treeCount = allTrees.size();
            for(uint32 i = 0; i < treeCount; ++i)
            {
                UpdateAnimationFlag(allTrees[i]->entity);
            }
        }
    }
}
    
};