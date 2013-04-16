#ifndef __DAVAENGINE_PARTICLE_LAYER_3D_H__
#define __DAVAENGINE_PARTICLE_LAYER_3D_H__

#include "Base/BaseTypes.h"
#include "Particles/ParticleLayer.h"
#include "Math/Matrix4.h"

namespace DAVA
{

class RenderDataObject;
class Material;
class Camera;
class ParticleLayer3D : public ParticleLayer
{
public:
	ParticleLayer3D(ParticleEmitter* parent);
	virtual ~ParticleLayer3D();

	virtual void LoadFromYaml(const String & configPath, YamlNode * node);
	virtual ParticleLayer * Clone(ParticleLayer * dstLayer = 0);

	virtual void Draw(Camera * camera);

	Material * GetMaterial();
	
	virtual void SetAdditive(bool additive);

	// Whether this layer should be drawn as "long" one?
	virtual bool IsLong();
	virtual void SetLong(bool value);

	// Access to parent.
	ParticleEmitter* GetParent() const {return parent;};

protected:
	void CalcNonLong(Particle* current,
							Vector3& topLeft,
							Vector3& topRight,
							Vector3& botLeft,
							Vector3& botRight);

	void CalcLong(Particle* current,
						 Vector3& topLeft,
						 Vector3& topRight,
						 Vector3& botLeft,
						 Vector3& botRight);

	// Draw method for generic and long emitters.
	void DrawLayer(Camera* camera);

	bool isLong;
	ParticleEmitter* parent;

	RenderDataObject * renderData;
	Vector<float32> verts;
	Vector<float32> textures;
	Vector<uint32> colors;

	Vector3 _up;
	Vector3 _left;
	Vector3 direction;
public:
    //INTROSPECTION_EXTEND(ParticleLayer3D, ParticleLayer,
    //    MEMBER(material, "Material", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
   // );
};

};

#endif //__DAVAENGINE_PARTICLE_LAYER_3D_H__
