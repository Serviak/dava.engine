#include "Particles/ParticleEmitter3D.h"
#include "Particles/ParticleLayer3D.h"
#include "Particles/ParticleLayerLong.h"
#include "Render/Highlevel/Camera.h"
#include "Utils/Random.h"

namespace DAVA
{

REGISTER_CLASS(ParticleEmitter3D);

ParticleEmitter3D::ParticleEmitter3D()
{
	is3D = true;
}

void ParticleEmitter3D::AddLayer(ParticleLayer * layer)
{
	// Only ParticleLayer3Ds are allowed on this level.
	if (dynamic_cast<ParticleLayer3D*>(layer))
	{
		ParticleEmitter::AddLayer(layer);
	}
}
	
void ParticleEmitter3D::AddLayer(ParticleLayer * layer, ParticleLayer * layerToMoveAbove)
{
	// Only ParticleLayer3Ds are allowed on this level.
	if (dynamic_cast<ParticleLayer3D*>(layer))
	{
		ParticleEmitter::AddLayer(layer, layerToMoveAbove);
	}
}

void ParticleEmitter3D::RenderUpdate(Camera *camera, float32 timeElapsed)
{
	eBlendMode srcMode = RenderManager::Instance()->GetSrcBlend();
	eBlendMode destMode = RenderManager::Instance()->GetDestBlend();

	Draw(camera);
    
	RenderManager::Instance()->SetBlendMode(srcMode, destMode);
}

void ParticleEmitter3D::Draw(Camera * camera)
{
	//Dizz: now layer->Draw is called from ParticleLayerBatch
	//Vector<ParticleLayer*>::iterator it;
	//for(it = layers.begin(); it != layers.end(); ++it)
	//{
	//	ParticleLayer3D * layer = (ParticleLayer3D*)(*it);
	//	if(!layer->isDisabled)
	//	{
	//		layer->Draw(camera);
	//	}
	//}
}

void ParticleEmitter3D::PrepareEmitterParameters(Particle * particle, float32 velocity, int32 emitIndex)
{
	Vector3 tempPosition = Vector3();
	Matrix4 * worldTransformPtr = GetWorldTransformPtr();
	if(worldTransformPtr)
	{
		tempPosition = worldTransformPtr->GetTranslationVector();
	}

	//Vector3 tempPosition = particlesFollow ? Vector3() : position;
    if (emitterType == EMITTER_POINT)
    {
        particle->position = tempPosition;
    }
    else if (emitterType == EMITTER_LINE)
    {
        // TODO: add emitter angle support
        float32 rand05 = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        Vector3 lineDirection(0, 0, 0);
        if(size)
            lineDirection = size->GetValue(time)*rand05;
        particle->position = tempPosition + lineDirection;
    }
    else if (emitterType == EMITTER_RECT)
    {
        // TODO: add emitter angle support
        float32 rand05_x = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        float32 rand05_y = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        float32 rand05_z = (float32)Random::Instance()->RandFloat() - 0.5f; // [-0.5f, 0.5f]
        Vector3 lineDirection(0, 0, 0);
        if(size)
            lineDirection = Vector3(size->GetValue(time).x * rand05_x, size->GetValue(time).y * rand05_y, size->GetValue(time).z * rand05_z);
        particle->position = tempPosition + lineDirection;
    }
    else if (emitterType == EMITTER_ONCIRCLE)
    {
        // here just set particle position
        particle->position = tempPosition;
    }
	
    Vector3 vel = Vector3(1.0f, 0.0f, 0.0f);
    if(emissionVector)
	{
        vel = emissionVector->GetValue(0);
		vel = vel*rotationMatrix;
	}
	
    Vector3 rotVect(0, 0, 1);
    float32 phi = PI*2*(float32)Random::Instance()->RandFloat();
    if(vel.x != 0)
    {
        rotVect.y = sinf(phi);
        rotVect.z = cosf(phi);
        rotVect.x = - rotVect.y*vel.y/vel.x - rotVect.z*vel.z/vel.x;
    }
    else if(vel.y != 0)
    {
        rotVect.x = cosf(phi);
        rotVect.z = sinf(phi);
        rotVect.y = - rotVect.z*vel.z/vel.y;
    }
    else if(vel.z != 0)
    {
        rotVect.x = cosf(phi);
        rotVect.y = sinf(phi);
        rotVect.z = 0;
    }
    rotVect.Normalize();
	
    float32 range = 0;
    if(emissionRange)
        range = DegToRad(emissionRange->GetValue(time) + angle);
    float32 rand05 = (float32)Random::Instance()->RandFloat() - 0.5f;
	
    Vector3 q_v(rotVect*sinf(range*rand05/2));
    float32 q_w = cosf(range*rand05/2);
	
    Vector3 q1_v(q_v);
    float32 q1_w = -q_w;
    q1_v /= (q_v.SquareLength() + q_w*q_w);
    q1_w /= (q_v.SquareLength() + q_w*q_w);
	
    Vector3 v_v(vel);
	
    Vector3 qv_v = q_v.CrossProduct(v_v) + q_w*v_v;
    float32 qv_w = - q_v.DotProduct(v_v);
	
    Vector3 qvq1_v = qv_v.CrossProduct(q1_v) + qv_w*q1_v + q1_w*qv_v;
	
	Vector3 speed = qvq1_v * velocity;
	particle->speed = speed.Length();
    particle->direction = speed/particle->speed;
	if (particle->direction.x <= EPSILON && particle->direction.x >= -EPSILON)
		particle->direction.x = 0.f;
	if (particle->direction.y <= EPSILON && particle->direction.y >= -EPSILON)
		particle->direction.y = 0.f;
	if (particle->direction.z <= EPSILON && particle->direction.z >= -EPSILON)
		particle->direction.z = 0.f;
	
    if (emitterType == EMITTER_ONCIRCLE)
    {
        qvq1_v.Normalize();
        if(radius)
            particle->position += qvq1_v * radius->GetValue(time);
    }
	
	// TODO! ARCTANGENS 0 != ARCTANGENS -0, EPSILON HERE!!!
    particle->angle = atanf(particle->direction.z/particle->direction.x);

	if(worldTransformPtr)
	{
		Matrix4 newTransform = *worldTransformPtr;
		newTransform._30 = newTransform._31 = newTransform._32 = 0;
		particle->direction = particle->direction*newTransform;
	}
}

void ParticleEmitter3D::LoadParticleLayerFromYaml(YamlNode* yamlNode, bool isLong)
{
	ParticleLayer* layer = NULL;
	 if (isLong)
	 {
		 layer = new ParticleLayerLong();
	 }
	 else
	 {
		 layer = new ParticleLayer3D();
	 }

	AddLayer(layer);
	layer->LoadFromYaml(configPath, yamlNode);
	SafeRelease(layer);
}

bool ParticleEmitter3D::Is3DFlagCorrect()
{
	// For ParticleEmitter3D is3D flag must be set to TRUE.
	return (is3D == true);
}

RenderObject * ParticleEmitter3D::Clone(RenderObject *newObject)
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<ParticleEmitter3D>(this), "Can clone only ParticleEmitter3D");
		newObject = new ParticleEmitter3D();
	}

	((ParticleEmitter3D*)newObject)->LoadFromYaml(configPath);

	return newObject;
}

}

