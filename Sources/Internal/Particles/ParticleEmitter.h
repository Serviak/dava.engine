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
#ifndef __DAVAENGINE_PARTICLE_EMITTER_H__
#define __DAVAENGINE_PARTICLE_EMITTER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Base/DynamicObjectCache.h"
#include "Base/RefPtr.h"
#include "Particles/ParticlePropertyLine.h"
#include "Animation/Animation.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/IRenderUpdatable.h"
#include "Particles/ParticleLayer.h"

namespace DAVA 
{
/**	
	\defgroup particlesystem Particle Systems
 */
	
class Particle;
//class ParticleLayer;
	
/**
	\ingroup particlesystem
	\brief Main class to work with particle systems in DAVA SDK. 
	This class allow to load particle emitters with various properties from yaml files and use them in the game.
	ParticleEmitter is class you can use to handle your particles in various 
 
	Parameters that can be used inside yaml file:
	
	type - can be one of the following values: point, line, rect, oncircle
	
	width - width of emitter generation zone in pixels
	height - height of emitter generation zone in pixels 
 
	If emitter type is line, width define the lenght of the line. 
	If emitter position equal 0, and width equal to 20, particles will be generated on line from -10px to 10px.
	
	If emitter type is rect width and height define size of the rect. emitter position define center of this rect. 
 
	radius - valid only if type is oncircle and define circle radius
	
	emissionAngle - angle of emmision in degrees for the particles
	emissionRange - emission range in degrees for particle generation. 
 
	Emitter generates random particles with angle [emissionAngle-emissionRange/2; emissionAngle+emissionRange/2].
	
	emitAtPoints - this number means that particles will be generated evenly on circle. If it's not defined particles will be generated randomly.
	life - emitter life in seconds. When accumulated time in ParticleEmitter::Update exceeds this value, emitter restarts and delete all previous particles. 
 */
class ParticleEmitter : public RenderObject, public IRenderUpdatable
{
public:
	enum eType
	{
		EMITTER_POINT,
		EMITTER_LINE,
		EMITTER_RECT,
		EMITTER_ONCIRCLE,		// 
	};

	ParticleEmitter();
	virtual ~ParticleEmitter();
	
	/**
		 \brief Function loads emitter from yaml file.
		 Function do not cache emitters so you'll need to cache them yourself in your code.
		 \param[in] pathName path to resource you want to load
	 */
	void LoadFromYaml(const String & pathName);
	
	/**
     \brief Function saves emitter to yaml file.
     \param[in] pathName path to resource you want to load
	 */
    void SaveToYaml(const String & pathName);
    
	/**
		\brief Function sets the position of emitter.
		This function is needed if you want to move emitter. You should understand that this function changes
		the virtual position of emitter, but not particle positions. So when you change position particle generation
		starts from this position, but all particles will remain at the same positions.
		
		\param[in] position position to be set
	 */
	inline void SetPosition(const Vector2 &position);
	inline void SetPosition(const Vector3 &position);
	
	/**
		\brief Function sets the angle of emitter.
		This function is needed if you want to rotate emitter. You should understand that this function changes
		the virtual angle of emitter, but not particle angles. So when you change angle particle generation
		start using this angle, but all particles will remain on previous positions with previous properties.

		\param[in] position position to be set
	 */
	inline void SetAngle(float32 angle);
	/** 
		\brief Function to get current position of emitter.
		\returns current position of emitter
	 */
	inline Vector3 & GetPosition();

	/** 
		\brief Function to get current particle count in all emitter layers.
		\returns number of particles
	 */
	int32 GetParticleCount();

	/**
		\brief This function restarts this emitter.
		You can delete particles or not. The idea that you can reuse emitter in different place, but at the same
		moment you can leave particles that was generated before. 
	 
		\param[in] isDeleteAllParticles if it's set to true emitter deletes all previous particles that was generated
	 */
	void Restart(bool isDeleteAllParticles = true);
	
    /**
     \brief Function to start generation from this emitter
	 */
	void Play();

    /**
     \brief Function to stop generation from this emitter
	 */
	void Stop();

	/**
     \brief Function returns is emitter stopped
     \returns is emitter stopped
	 */
	bool IsStopped();

	/**
		\brief Function to pause generation from this emitter
		\param[in] isPaused true if you want to pause the generation, false if you want to resume it
	 */
	void Pause(bool isPaused = true);
	
	/**
		\brief Function returns is emitter paused
		\returns is emitter paused 
	 */
	bool IsPaused();
	
	/**
		\brief Function adds layer to emitter.
		You can use this function if you create emitters on the fly manually. It's not often case, but probably sometimes
		it can be required. 
		\param[in] layer layer to be added
	 */
	void AddLayer(ParticleLayer * layer);

	/**
	 \brief Function adds layer to emitter to the particular position.
	 You can use this function if you create emitters on the fly manually. It's not often case, but probably sometimes
	 it can be required.
	 \param[in] layer layer to be added
  	 \param[in] layerToMoveAbove the position above which the layer will be inserted
	 */
	void AddLayer(ParticleLayer * layer, ParticleLayer * layerToMoveAbove);

	/**
	 \brief Function removes layer to emitter.
	 You can use this function if you create emitters on the fly manually. It's not often case, but probably sometimes
	 it can be required.
	 \param[in] layer layer to be removed
	 */
	void RemoveLayer(ParticleLayer * layer);

	/**
	 \brief Function change the layer's order inside the same emitter.
	 \param[in] layer layer to be moved
 	 \param[in] layerToMoveAbove the position above which the layer will be moved
	 */
	void MoveLayer(ParticleLayer * layer, ParticleLayer * layerToMoveAbove);

	/**
		\brief Function to clone emitter.
		This function is needed then you do not want to reload emitter every time from disk.
		It can fastly clone current emitter and create another copy.
		
		\returns absolutelly identical object with same properties
	 */
	//cloned in ParticleEmitterComponent::Clone
	//ParticleEmitter * Clone();

	virtual RenderObject * Clone();
	
	/**
		\brief Function to get number of repeats for current particle emitter.
	 */
	int32 GetRepeatCount();
	
	
	/**
		\brief Get life time of emitter
		\returns value of the emitter life time. 
	 */
	float32 GetLifeTime();
	
	/**
		\brief Function to get emit point count for this emitter
	 */
	int32 GetEmitPointsCount();
	
	/**
		\brief Function to get layers of this emitter
	 */
	Vector<ParticleLayer*> & GetLayers();
	/**
		\brief Function to update particle emitter
		If you using ParticleEmitter directly you should call this function and pass time elapsed fromp previous frame to it.
		\param[in] timeElapsed time in seconds elapsed from previous update
	 */
	void Update(float32 timeElapsed);
	/**	
		\brief function to draw particle emitter
		If you using ParticleEmitter directly you should call this function to draw emitter.
		Instead of use it directly check ParticleEmitterObject class, that allow you to use ParticleEmitters inside GameObject hierarchy.
	 */
	virtual void RenderUpdate(float32 timeElapsed);

	/**
	 \brief Enable/disable autorestart.
	 If autorestart is enabled, emitter will automatically start it's work from beginning after it's lifeTime ends. 
	 Option is enabled by default.
	 \param[in] autoRestart enable autorestart if true
	 */
	void SetAutorestart(bool isAutorestart);

	/**
	 \brief Get autorestart state.
	 \returns current autorestart state.
	 */
	bool GetAutorestart();

	/**
	 \brief Get emitter's size.
	 Valid only for emitter with type 'rect' and 'line'.
	 \returns emitter's width, height and depth in Vector3 form.
	 */
	Vector3 GetSize();
	Vector3 GetSize(float32 time);

	/**
	 \brief Set width and height.
	 Valid only for emitter with type 'rect' and 'line'.
	 \param[in] size emitter's width, height and depth in Vector3 form.
	 */
	void SetSize(const Vector3& size);

	/**
	 \brief Enables/disables particles following.
	 If enabled and ParticleEmitter::SetPosition() is called, particles will change their position respectively.
	 \param[in] follow enable particles following if true.
	 */
	inline void SetParticlesFollow(bool follow);

	/// Particles' color is multiplied by ambientColor before drawing.
	Color ambientColor;

	/**
	 \brief Starts size animation for the emitter's rect.
	 \param[in] newSize New size.
	 \param[in] time animation time.
	 \param[in] interpolationFunc time interpolation method.
	 \param[in] track animation track. 0 by default.
	 \returns Animation object
	 */
	Animation * SizeAnimation(const Vector3 & newSize, float32 time, Interpolation::FuncType interpolationFunc = Interpolation::LINEAR, int32 track = 0);
    
    float32 GetTime();
    
	void SetLifeTime(float32 time);
    
    inline void Set3D(bool is3D);
    inline bool GetIs3D();

	const String & GetConfigPath() { return configPath; }
	void Cleanup(bool needCleanupLayers = true);

	void UpdateEmptyLayerNames();
	void UpdateLayerNameIfEmpty(ParticleLayer* layer, int32 index);

	void ReloadLayerSprites();

protected:
	void PrepareEmitterParameters(Particle * particle, float32 velocity, int32 emitIndex);
    String GetEmitterTypeName();

	void CleanupLayers();

	String configPath;
	
	Vector<ParticleLayer*> layers;
	Vector3 position;
	float32 angle;
	
	float32	lifeTime;
	int32	repeatCount;
	float32 time;
	int32	emitPointsCount;
	bool	isPaused;
	bool	isAutorestart;
	bool	particlesFollow;
    bool    is3D;

public:
	RefPtr< PropertyLine<Vector3> > emissionVector;
	Matrix3 rotationMatrix;
    RefPtr< PropertyLine<float32> > emissionAngle;
	RefPtr< PropertyLine<float32> > emissionRange;
	RefPtr< PropertyLine<float32> > radius;
	RefPtr< PropertyLine<Color> > colorOverLife;
	RefPtr< PropertyLine<Vector3> > size;
    
	eType	type;
	Color currentColor;

	bool GetCurrentColor(Color * currentColor);
	// RefPtr< PropertyLine<float32> > number;
	
	friend class ParticleLayer;
    
public:
    
    INTROSPECTION_EXTEND(ParticleEmitter, RenderObject,
                         NULL
//        MEMBER(ambientColor, "Ambient Color", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
//        MEMBER(configPath, "Config Path", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//
//        COLLECTION(layers, "Layers", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
//        MEMBER(position, "Position", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(angle, "Angle", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                     
//        MEMBER(lifeTime, "Life Time", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(repeatCount, "Repeat Count", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(time, "Time", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(emitPointsCount, "Emit Points Count", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
//        MEMBER(isAutorestart, "Is Auto Restart", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(particlesFollow, "Particles Follow", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(is3D, "Is 3D", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
////        MEMBER(emissionVector, "Emission Vector", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(rotationMatrix, "Rotation Matrix", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
////        MEMBER(emissionAngle, "Emission Angle", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
////        MEMBER(emissionRange, "Emission Range", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
////        MEMBER(radius, "Radius", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
////        MEMBER(colorOverLife, "Color Over Life", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
////        MEMBER(size, "Size", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//                         
////        MEMBER(type, "Type", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
//        MEMBER(currentColor, "Current Color", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};

inline void ParticleEmitter::SetPosition(const Vector2 &_position)
{
	position = _position;
}

inline void ParticleEmitter::SetPosition(const Vector3 &_position)
{
    position = _position;
}
inline Vector3 & ParticleEmitter::GetPosition()
{
	return position;
}
    
inline void ParticleEmitter::Set3D(bool _is3D)
{
    is3D = _is3D;
}
    
inline bool ParticleEmitter::GetIs3D()
{
    return is3D;
}
    
inline void ParticleEmitter::SetAngle(float32 _angle)
{
	angle = _angle;
}

inline bool ParticleEmitter::GetCurrentColor(Color * _currentColor)
{
	if(colorOverLife)
	{
		*_currentColor = currentColor;
		return true;
	}
	else
	{
		return false;
	}
}

inline void ParticleEmitter::SetParticlesFollow(bool follow)
{
	particlesFollow = follow;
}
};

#endif // __DAVAENGINE_PARTICLE_EMITTER_H__
