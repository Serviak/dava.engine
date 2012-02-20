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
#include "Scene3D/Camera.h"
#include "Render/RenderBase.h"
#include "Core/Core.h"
#include "Render/RenderManager.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA 
{
    
REGISTER_CLASS(Camera);


Camera::Camera(Scene * scene) : SceneNode(scene)
{
	Setup(35.0f, 1.0f, 1.0f, 2500.f, false);
	up = Vector3(0.0f, 1.0f, 0.0f);
	left = Vector3(1.0f, 0.0f, 0.0f);
	flags = REQUIRE_REBUILD | REQUIRE_REBUILD_MODEL | REQUIRE_REBUILD_PROJECTION;
    
	cameraTransform.Identity();
    currentFrustum = new Frustum();
}
	
Camera::~Camera()
{
	SafeRelease(currentFrustum);
}
	
void Camera::RestoreOriginalSceneTransform()
{
	cameraTransform = GetLocalTransform();
	SceneNode * node = GetParent();
	while(node)
	{
		cameraTransform = node->GetLocalTransform() * cameraTransform;
		node = node->GetParent();
	}
	worldTransform = cameraTransform;
	cameraTransform.Inverse();
	ExtractCameraToValues();
}

void Camera::SetFOV(float32 fovyInDegrees)
{
    Setup(fovyInDegrees, aspect, znear, zfar, ortho);
}
    
void Camera::SetAspect(float32 _aspect)
{
    Setup(fovy, _aspect, znear, zfar, ortho);
}
    
float32 Camera::GetFOV() const
{
    return fovy;
}

float32 Camera::GetAspect() const
{
    return aspect;
}

float32 Camera::GetZNear() const
{
    return znear;
}

float32 Camera::GetZFar() const
{
    return zfar;
}
    
bool Camera::GetIsOrtho() const
{
    return ortho;
}


void Camera::Setup(float32 fovyInDegrees, float32 aspectYdivX, float32 zNear, float32 zFar, bool isOrtho)
{
    flags |= REQUIRE_REBUILD_PROJECTION;

    this->aspect = aspectYdivX;
    
    this->fovy = fovyInDegrees;
	this->znear = zNear;
	this->zfar = zFar;
	this->ortho = isOrtho;
	
	this->znear = 1;
	this->zfar = 5000;
	Recalc();
}

void Camera::Recalc()
{
	ymax = znear * tanf(fovy * PI / 360.0f);
	ymin = -ymax;
	
    float32 realAspect = aspect;
    if ((RenderManager::Instance()->GetRenderOrientation() == Core::SCREEN_ORIENTATION_PORTRAIT) || (RenderManager::Instance()->GetRenderOrientation() == Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN) || (RenderManager::Instance()->GetRenderOrientation() == Core::SCREEN_ORIENTATION_TEXTURE))
    {
        realAspect = 1.0f / realAspect;
	}
    
	xmin = ymin * realAspect;
	xmax = ymax * realAspect;
    CalculateZoomFactor();
}

Vector2 Camera::GetOnScreenPosition(const Vector3 &forPoint, const Rect & viewport)
{
    Vector4 pv(forPoint);
    pv = pv * GetUniformProjModelMatrix();
//    return Vector2((viewport.dx * 0.5f) * (1.f + pv.x/pv.w) + viewport.x
//                   , (viewport.dy * 0.5f) * (1.f + pv.y/pv.w) + viewport.y);


	switch(RenderManager::Instance()->GetRenderOrientation())
	{
		case Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
        {
            return Vector2((viewport.dx * 0.5f) * (1.f + pv.y/pv.w) + viewport.x
                            , (viewport.dy * 0.5f) * (1.f + pv.x/pv.w) + viewport.y);
        }
            break;
		case Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
        {
            DVASSERT(false);
        }
                //add code here
			break;
        default:
            return Vector2(((pv.x/pv.w)*0.5f+0.5f)*viewport.dx+viewport.x,
                       (1.0f - ((pv.y/pv.w)*0.5f+0.5f))*viewport.dy+viewport.y);
            break;
	}
    DVASSERT(false);
	return Vector2();
}

const Matrix4 &Camera::GetUniformProjModelMatrix()
{
    if (flags & REQUIRE_REBUILD)
    {
        RebuildCameraFromValues();
    }
    if (flags & REQUIRE_REBUILD_PROJECTION)
    {
        RecalcFrustum();
    }
    if (flags & REQUIRE_REBUILD_MODEL)
    {
        RecalcTransform();
    }
    if (flags & REQUIRE_REBUILD_UNIFORM_PROJ_MODEL)
    {
        uniformProjModelMatrix = modelMatrix * projMatrix;
        flags &= ~REQUIRE_REBUILD_UNIFORM_PROJ_MODEL;
    }
    
    return uniformProjModelMatrix;
}

void Camera::RecalcFrustum()
{
    flags &= ~REQUIRE_REBUILD_PROJECTION;
    flags |= REQUIRE_REBUILD_UNIFORM_PROJ_MODEL;
    if (!ortho) 
    {
        projMatrix.glFrustum(xmin, xmax, ymin, ymax, znear, zfar);
    }
    else
    {
        projMatrix.glOrtho(xmin, xmax, ymin, ymax, znear, zfar);
    }
}

void Camera::RecalcTransform()
{
    flags &= ~REQUIRE_REBUILD_MODEL;
    flags |= REQUIRE_REBUILD_UNIFORM_PROJ_MODEL;

//	Core::eScreenOrientation orientation = Core::Instance()->GetScreenOrientation();
	modelMatrix = Matrix4::IDENTITY;
    
	switch(RenderManager::Instance()->GetRenderOrientation())
	{
		case Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
                //glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
			modelMatrix.CreateRotation(Vector3(0.0f, 0.0f, 1.0f), DegToRad(-90.0f));
            break;
		case Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
                //glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
            modelMatrix.CreateRotation(Vector3(0.0f, 0.0f, 1.0f), DegToRad(90.0f));
			break;
		case Core::SCREEN_ORIENTATION_PORTRAIT_UPSIDE_DOWN:
		case Core::SCREEN_ORIENTATION_TEXTURE:
                //glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
            modelMatrix.CreateRotation(Vector3(0.0f, 0.0f, 1.0f), DegToRad(180.0f));
			break;
        default:
            break;
	}
    modelMatrix = cameraTransform * modelMatrix;
}

    
void Camera::ApplyFrustum()
{
    if (flags & REQUIRE_REBUILD_PROJECTION)
    {
        RecalcFrustum();
    }

    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_PROJECTION, projMatrix);
    
    /*  Boroda: Matrix Extract Snippet
     
     float32 proj[16];
     glGetFloatv(GL_PROJECTION_MATRIX, proj);
     
     Matrix4 frustumMatrix;
     frustumMatrix.glFrustum(xmin, xmax, ymin, ymax, znear, zfar);
     glLoadMatrixf(frustumMatrix.data);
     
     for (int32 k = 0; k < 16; ++k)
     {
        printf("k:%d - %0.3f = %0.3f\n", k, proj[k], frustumMatrix.data[k]);
     }

     */
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//#ifdef __DAVAENGINE_IPHONE__
//	if (!ortho)
//	{
//		glFrustumf(xmin, xmax, ymin, ymax, znear, zfar);
//	}
//	else 
//	{
//		glOrthof(xmin, xmax, ymin, ymax, znear, zfar);
//	}
//#else
//	if (!ortho)
//	{
//		glFrustum(xmin, xmax, ymin, ymax, znear, zfar);        
//    }
//	else 
//	{
//		glOrtho(xmin, xmax, ymin, ymax, znear, zfar);
//	}
//#endif
}

void Camera::ApplyTransform()
{
    if (flags & REQUIRE_REBUILD_MODEL)
    {
        RecalcTransform();
    }
        //glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
    
    //RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, Matrix4::IDENTITY);

	
	// Xpen poymesh chto eto napisano
	//glLoadMatrixf(localTransform.data);
	// Matrix4 m = worldTransform;
	// m.Inverse();
	// cameraTransform =
	
    // Correct code from boroda // commented during refactoring
    //glMultMatrixf(cameraTransform.data);
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, modelMatrix);
}

void Camera::SetPosition(const Vector3 & _position)
{
	position = _position;
    flags |= REQUIRE_REBUILD;
}

void Camera::SetDirection(const Vector3 & _direction)
{
	target = position + _direction;
    flags |= REQUIRE_REBUILD;
}

void Camera::SetTarget(const Vector3 & _target)
{
	target = _target;
    flags |= REQUIRE_REBUILD;
}
    
void Camera::SetUp(const Vector3 & _up)
{
    up = _up;
    flags |= REQUIRE_REBUILD;
}
    
void Camera::SetLeft(const Vector3 & _left)
{
    left = _left;
    flags |= REQUIRE_REBUILD;
}

	
const Vector3 & Camera::GetTarget() const
{
	return target;
}

const Vector3 & Camera::GetPosition() const
{
	return position;
}

const Vector3 & Camera::GetDirection()
{
    direction = target - position;
    direction.Normalize();
    return direction;
}

const Vector3 & Camera::GetUp() const
{
    return up;
}

const Vector3 & Camera::GetLeft() const
{
    return left;
}
    
const Matrix4 & Camera::GetMatrix() const 
{
    return modelMatrix;
}

void Camera::RebuildCameraFromValues()
{
//    Logger::Debug("camera rebuild: pos(%0.2f %0.2f %0.2f) target(%0.2f %0.2f %0.2f) up(%0.2f %0.2f %0.2f)",
//                  position.x, position.y, position.z, target.x, target.y, target.z, up.x, up.y, up.z);
    
    flags &= ~REQUIRE_REBUILD; 
    flags |= REQUIRE_REBUILD_MODEL;
	cameraTransform.BuildLookAtMatrixRH(position, target, up);
    
    // update left vector after rebuild
	left.x = cameraTransform._00;
	left.y = cameraTransform._10;
	left.z = cameraTransform._20;
}
	
void Camera::ExtractCameraToValues()
{
	position.x = worldTransform._30;
	position.y = worldTransform._31;
	position.z = worldTransform._32;
	left.x = cameraTransform._00;
	left.y = cameraTransform._10;
	left.z = cameraTransform._20;
	up.x = cameraTransform._01;
	up.y = cameraTransform._11;
	up.z = cameraTransform._21;
	target.x = position.x - cameraTransform._02;
	target.y = position.y - cameraTransform._12;
	target.z = position.z - cameraTransform._22;
}


/*
void Camera::LookAt(Vector3	position, Vector3 view, Vector3 up)
{
	// compute matrix
	localTransform.BuildLookAtMatrixLH(position, view, up);
}
 */

void Camera::Set()
{
	flags = REQUIRE_REBUILD | REQUIRE_REBUILD_MODEL | REQUIRE_REBUILD_PROJECTION;
    if (flags & REQUIRE_REBUILD)
    {
        RebuildCameraFromValues();
    }
	ApplyFrustum();
	ApplyTransform();
    
    if (currentFrustum)
    {
        currentFrustum->Set();
    }
}

SceneNode* Camera::Clone(SceneNode *dstNode)
{
    if (!dstNode) 
    {
        dstNode = new Camera(scene);
    }
    SceneNode::Clone(dstNode);
    Camera *cnd = (Camera*)dstNode;
    cnd->xmin = xmin;
    cnd->xmax = xmax;
    cnd->ymin = ymin;
    cnd->ymax = ymax;
    cnd->znear = znear;
    cnd->zfar = zfar;
    cnd->aspect = aspect;
    cnd->fovy = fovy;
    cnd->ortho = ortho;
    
    cnd->position = position;
    cnd->target = target;
    cnd->up = up;
    cnd->left = left;
    //cnd->rotation = rotation;
    cnd->cameraTransform = cameraTransform;
    cnd->flags = flags;
    
    cnd->zoomFactor = zoomFactor;
    return dstNode;
}
    
Frustum * Camera::GetFrustum() const
{
    return currentFrustum;
}
    
void Camera::CalculateZoomFactor()
{
    zoomFactor = tanf(DegToRad(fovy * 0.5f));
}

float32 Camera::GetZoomFactor() const
{
    return zoomFactor;
}


    
void Camera::Draw()
{
    if (debugFlags & DEBUG_DRAW_ALL)
    {
        Camera * prevCamera = scene->GetCurrentCamera();

        // Build this camera matrixes & it's frustum
        this->Set();
        
        // Restore original camera
        // Do that only if exists because potentially it can be scene without camera set
        if (prevCamera)
            prevCamera->Set();
        
        RenderManager::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
        
        // If this is clip camera - show it as red camera
        if (this == scene->GetClipCamera())
            RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);

        // Draw frustum of this camera
        if (currentFrustum)
        {
            currentFrustum->DebugDraw();
        }
        // reset color
        RenderManager::Instance()->ResetColor();
    }
}

Vector3 Camera::UnProject(float32 winx, float32 winy, float32 winz, const Rect & viewport)
{
	Matrix4 finalMatrix = modelMatrix * projMatrix;//RenderManager::Instance()->GetUniformMatrix(RenderManager::UNIFORM_MATRIX_MODELVIEWPROJECTION);
	finalMatrix.Inverse();		

	Vector4 in(winx, winy, winz, 1.0f);

	/* Map x and y from window coordinates */

	in.x = (in.x - viewport.x) / viewport.dx;
	in.y = 1.0f - (in.y - viewport.y) / viewport.dy;

	/* Map to range -1 to 1 */
	in.x = in.x * 2 - 1;
	in.y = in.y * 2 - 1;
	in.z = in.z * 2 - 1;

	Vector4 out = in * finalMatrix;
	
	Vector3 result(0,0,0);
	if (out.w == 0.0) return result;
	
	
	result.x = out.x / out.w;
	result.y = out.y / out.w;
	result.z = out.z / out.w;
	return result;
}
    
/*
     float32 xmin, xmax, ymin, ymax, znear, zfar, aspect, fovy;
     bool ortho;
     
     
     Vector3 position;		//
     Vector3 target;		//	
     Vector3 up;
     Vector3 left;
     
     Vector3 direction;  // right now this variable updated only when you call GetDirection.
     
     //Quaternion rotation;	// 
     Matrix4 cameraTransform;
     
     Matrix4 modelMatrix;
     Matrix4 projMatrix;
     Matrix4 uniformProjModelMatrix;
     
     uint32 flags;
*/
    
void Camera::Save(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
    SceneNode::Save(archive, sceneFile);
    
    archive->SetFloat("cam.xmin", xmin);
    archive->SetFloat("cam.xmax", xmax);
    archive->SetFloat("cam.ymin", ymin);
    archive->SetFloat("cam.ymax", ymax);
    archive->SetFloat("cam.znear", znear);
    archive->SetFloat("cam.zfar", zfar);
    archive->SetFloat("cam.aspect", aspect);
    archive->SetFloat("cam.fov", fovy);
    archive->SetBool("cam.isOrtho", ortho);
    archive->SetInt32("cam.flags", flags);
    
    archive->SetByteArrayAsType("cam.position", position);
    archive->SetByteArrayAsType("cam.target", target);
    archive->SetByteArrayAsType("cam.up", up);
    archive->SetByteArrayAsType("cam.left", left);
    archive->SetByteArrayAsType("cam.direction", direction);

    archive->SetByteArrayAsType("cam.cameraTransform", cameraTransform);

    archive->SetByteArrayAsType("cam.modelMatrix", modelMatrix);
    archive->SetByteArrayAsType("cam.projMatrix", projMatrix);
}

void Camera::Load(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
    SceneNode::Load(archive, sceneFile);
    
    // todo add default values
    xmin = archive->GetFloat("cam.xmin");
    xmax = archive->GetFloat("cam.xmax");
    ymin = archive->GetFloat("cam.ymin");
    ymax = archive->GetFloat("cam.ymax");
    znear = archive->GetFloat("cam.znear");
    zfar = archive->GetFloat("cam.zfar");
    aspect = archive->GetFloat("cam.aspect");
    fovy = archive->GetFloat("cam.fov");
    ortho = archive->GetBool("cam.isOrtho");
    flags = archive->GetInt32("cam.flags");
    
    position = archive->GetByteArrayAsType("cam.position", position);
    target = archive->GetByteArrayAsType("cam.target", target);
    up = archive->GetByteArrayAsType("cam.up", up);
    left = archive->GetByteArrayAsType("cam.left", left);
    direction = archive->GetByteArrayAsType("cam.direction", direction);

    cameraTransform = archive->GetByteArrayAsType("cam.cameraTransform", cameraTransform);
    modelMatrix = archive->GetByteArrayAsType("cam.modelMatrix", modelMatrix);
    projMatrix = archive->GetByteArrayAsType("cam.projMatrix", projMatrix);
}

	
//SceneNode* Camera::Clone()
//{
//    return CopyDataTo(new Camera(scene));
//}
//
//SceneNode* Camera::CopyDataTo(SceneNode *dstNode)
//{
//    SceneNode::CopyDataTo(dstNode);
//    dstNode->xmin = xmin;
//    dstNode->xmax = xmax;
//    dstNode->ymin = ymin;
//    dstNode->ymax = ymax;
//    dstNode->znear = znear;
//    dstNode->zfar = zfar;
//    dstNode->aspect = aspect;
//    dstNode->fovy = fovy;
//	dstNode->ortho = ortho;
//    
//	dstNode->position = position;
//	dstNode->target = target;
//	dstNode->up = up;
//	dstNode->left = left;
//	dstNode->rotation = rotation;
//	dstNode->cameraTransform = cameraTransform;
//    return dstNode;
//}


	
} // ns



