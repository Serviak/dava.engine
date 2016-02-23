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

#include "Scene/System/CollisionSystem/CollisionBox.h"

CollisionBox::CollisionBox(DAVA::Entity* entity, btCollisionWorld* word, DAVA::Vector3 position, DAVA::float32 boxSize)
    : CollisionBaseObject(entity, word)
{
    if (word != nullptr)
    {
        btTransform trans;
        trans.setIdentity();
        trans.setOrigin(btVector3(position.x, position.y, position.z));

        btShape = new btBoxShape(btVector3(boxSize / 2, boxSize / 2, boxSize / 2));
        btObject = new btCollisionObject();
        btObject->setCollisionShape(btShape);
        btObject->setWorldTransform(trans);
        btWord->addCollisionObject(btObject);

        boundingBox = DAVA::AABBox3(DAVA::Vector3(), boxSize);
    }
}

CollisionBox::~CollisionBox()
{
    if (btObject != nullptr)
    {
        btWord->removeCollisionObject(btObject);
        DAVA::SafeDelete(btObject);
        DAVA::SafeDelete(btShape);
    }
}

CollisionBaseObject::ClassifyPlaneResult CollisionBox::ClassifyToPlane(const DAVA::Plane& plane)
{
    return ClassifyBoundingBoxToPlane(boundingBox, TransformPlaneToLocalSpace(plane));
}

CollisionBaseObject::ClassifyPlanesResult CollisionBox::ClassifyToPlanes(DAVA::Plane* plane, size_t numPlanes)
{
    for (size_t i = 0; i < numPlanes; ++i)
    {
        if (ClassifyToPlane(plane[i]) == CollisionBaseObject::ClassifyPlaneResult::Behind)
        {
            return CollisionBaseObject::ClassifyPlanesResult::Outside;
        }
    }
    return CollisionBaseObject::ClassifyPlanesResult::ContainsOrIntersects;
}