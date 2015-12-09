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


#ifndef __SCENE_COLLISION_BASE_OBJECT_H__
#define __SCENE_COLLISION_BASE_OBJECT_H__

#include "bullet/btBulletCollisionCommon.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
class Entity;
}

class CollisionBaseObject
{
public:
    enum class ClassifyPointResult
    {
        InFront,
        Behind
    };

    enum class ClassifyPlaneResult
    {
        InFront,
        Intersects,
        Behind
    };

public:
    CollisionBaseObject(DAVA::Entity* ent, btCollisionWorld* word)
        : entity(ent)
        , btWord(word)
    { }

	virtual ~CollisionBaseObject()
	{ }

    virtual ClassifyPlaneResult ClassifyToPlane(const DAVA::Plane& plane) = 0;

    inline CollisionBaseObject::ClassifyPlaneResult ClassifyBoundingBoxToPlane(const DAVA::AABBox3& bbox, const DAVA::Plane& plane) const;
    inline CollisionBaseObject::ClassifyPointResult ClassifyPointToPlane(const DAVA::Vector3& point, const DAVA::Plane& plane) const;

    DAVA::Entity *entity;
	DAVA::AABBox3 boundingBox;
    btCollisionObject* btObject = nullptr;
    btCollisionWorld *btWord;
};

CollisionBaseObject::ClassifyPlaneResult CollisionBaseObject::ClassifyBoundingBoxToPlane(const DAVA::AABBox3& bbox, const DAVA::Plane& plane) const
{
    char cornersData[8 * sizeof(DAVA::Vector3)];
    DAVA::Vector3* corners = reinterpret_cast<DAVA::Vector3*>(cornersData);
    bbox.GetCorners(corners);

    DAVA::float32 minDistance = std::numeric_limits<float>::max();
    DAVA::float32 maxDistance = -minDistance;
    for (DAVA::uint32 i = 0; i < 8; ++i)
    {
        float d = plane.DistanceToPoint(corners[i] * entity->GetWorldTransform());
        minDistance = std::min(minDistance, d);
        maxDistance = std::max(maxDistance, d);
    }

    if ((minDistance > 0.0f) && (maxDistance > 0.0f))
        return ClassifyPlaneResult::InFront;

    if ((minDistance < 0.0f) && (maxDistance < 0.0f))
        return ClassifyPlaneResult::Behind;

    return ClassifyPlaneResult::Intersects;
}

inline CollisionBaseObject::ClassifyPointResult CollisionBaseObject::ClassifyPointToPlane(const DAVA::Vector3& point, const DAVA::Plane& plane) const
{
    float d = plane.DistanceToPoint(point * entity->GetWorldTransform());
    return (d >= 0.0f) ? ClassifyPointResult::InFront : ClassifyPointResult::Behind;
}

#endif // __SCENE_COLLISION_BASE_OBJECT_H__
