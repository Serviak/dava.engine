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


#include "Render/RenderHelper.h"
#include "Render/Texture.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/Renderer.h"

namespace DAVA
{
	/*
	static Vector3 DodecVertexes[20] = {
		Vector3( 0.607f,  0.000f,  0.795f),
		Vector3( 0.188f,  0.577f,  0.795f),
		Vector3(-0.491f,  0.357f,  0.795f),
		Vector3(-0.491f, -0.357f,  0.795f),
		Vector3( 0.188f, -0.577f,  0.795f),
		Vector3( 0.982f,  0.000f,  0.188f),
		Vector3( 0.304f,  0.934f,  0.188f),
		Vector3(-0.795f,  0.577f,  0.188f),
		Vector3(-0.795f, -0.577f,  0.188f),
		Vector3( 0.304f, -0.934f,  0.188f),
		Vector3( 0.795f,  0.577f, -0.188f),
		Vector3(-0.304f,  0.934f, -0.188f),
		Vector3(-0.982f,  0.000f, -0.188f),
		Vector3(-0.304f, -0.934f, -0.188f),
		Vector3( 0.795f, -0.577f, -0.188f),
		Vector3( 0.491f,  0.357f, -0.795f),
		Vector3(-0.188f,  0.577f, -0.795f),
		Vector3(-0.607f,  0.000f, -0.795f),
		Vector3(-0.188f, -0.577f, -0.795f),
		Vector3( 0.491f, -0.357f, -0.795f)
	};

	static int DodecIndexes[12][5] = { 
		0, 1, 2, 3, 4,
		0, 1, 6, 10, 5,
		1, 2, 7, 11, 6,
		2, 3, 8, 12, 7,
		3, 4, 9, 13, 8,
		4, 0, 5, 14, 9,
		15, 16, 11, 6, 10,
		16, 17, 12, 7, 11,
		17, 18, 13, 8, 12,
		18, 19, 14, 9, 13,
		19, 15, 10, 5, 14,
		15, 16, 17, 18, 19
	};
	*/

	#define isoX 0.525731f 
	#define isoZ 0.850650f

	static Vector3 gDodecVertexes[12] = {
		Vector3(-isoX, 0.0, isoZ),
		Vector3(isoX, 0.0, isoZ),
		Vector3(-isoX, 0.0, -isoZ),
		Vector3(isoX, 0.0, -isoZ),
		Vector3(0.0, isoZ, isoX),
		Vector3(0.0, isoZ, -isoX),
		Vector3(0.0, -isoZ, isoX),
		Vector3(0.0, -isoZ, -isoX),
		Vector3(isoZ, isoX, 0.0),
		Vector3(-isoZ, isoX, 0.0),
		Vector3(isoZ, -isoX, 0.0),
		Vector3(-isoZ, -isoX, 0.0)
	};

	static DAVA::uint16 gDodecIndexes[60] = {
		0, 4, 1,
		0, 9, 4,
		9, 5, 4,
		4, 5, 8,
		4, 8, 1,
		8, 10, 1,
		8, 3, 10,
		5, 3, 8,
		5, 2, 3,
		2, 7, 3,
		7, 10, 3,
		7, 6, 10,
		7, 11, 6,
		11, 0, 6,
		0, 1, 6,
		6, 1, 10,
		9, 0, 11,
		9, 11, 2,
		9, 2, 5,
		7, 2, 11
	};

#if RHI_COMPLETE
    static RenderDataObject *gDodecObject;
#endif
	
	const float32 SEGMENT_LENGTH = 15.0f;
	
RenderHelper::RenderHelper()
{
#if RHI_COMPLETE
    renderDataObject = new RenderDataObject();
    vertexStream = renderDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 2, 0, 0);
    texCoordStream = renderDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, 0);

    gDodecObject = new RenderDataObject();
    gDodecObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, gDodecVertexes);
    gDodecObject->SetIndices(EIF_16, (DAVA::uint8 *) gDodecIndexes, sizeof(gDodecIndexes) / sizeof(gDodecIndexes[0]));
#endif //RHI_COMPLETE
}
RenderHelper::~RenderHelper()
{
#if RHI_COMPLETE
    SafeRelease(renderDataObject);
    SafeRelease(gDodecObject);
#endif // RHI_COMPLETE

}
    
void RenderHelper::DrawLine(const Vector3 & start, const Vector3 & end, float32 lineWidth, NMaterial *material)
{
    #ifdef RHI_COMPLETE
RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    vertices[0] = start.x;						
    vertices[1] = start.y;
    vertices[2] = start.z;
    
    vertices[3] = end.x;
    vertices[4] = end.y;
    vertices[5] = end.z;

    
    vertexStream->Set(TYPE_FLOAT, 3, 0, vertices);
    
    RenderManager::Instance()->SetRenderState(renderState);
    RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
    RenderManager::Instance()->SetRenderData(renderDataObject);

#ifdef __DAVAENGINE_OPENGL__
	glLineWidth(lineWidth);
#endif
    RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINESTRIP, 0, 2);
#ifdef __DAVAENGINE_OPENGL__
	glLineWidth(1.f);
#endif
#endif // RHI_COMPLETE
}

void RenderHelper::DrawCircle(const Vector3 & center, float32 radius, NMaterial *material)
{
    Polygon3 pts;
    float32 angle = Min(PI/6.0f, SEGMENT_LENGTH / radius);// maximum angle 30 degrees
	int ptsCount = (int)(2 * PI / (DegToRad(angle))) + 1;

    pts.points.reserve(ptsCount);
	for (int k = 0; k < ptsCount; ++k)
	{
		angle = ((float)k / (ptsCount - 1)) * 2 * PI;
		float32 sinA = sinf(angle);
		float32 cosA = cosf(angle);
		Vector3 pos = center - Vector3(sinA * radius, cosA * radius, 0);

		pts.AddPoint(pos);
	}
    DrawPolygon(pts, false, material);
}

void RenderHelper::DrawCircle3D(const Vector3 & center, const Vector3 &emissionVector, float32 radius, bool useFilling, NMaterial *material)
{
#ifdef RHI_COMPLETE
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

	Polygon3 pts;
    float32 angle = Min(PI/6.0f, SEGMENT_LENGTH / radius);// maximum angle 30 degrees
	int ptsCount = (int)(PI_2 / (DegToRad(angle))) + 1;

    pts.points.reserve(ptsCount);
	for (int k = 0; k < ptsCount; ++k)
	{
		float32 angleA = ((float)k / (ptsCount - 1)) * PI_2;
		float sinAngle = 0.0f;
		float cosAngle = 0.0f;
		SinCosFast(angleA, sinAngle, cosAngle);

		Vector3 directionVector(radius * cosAngle,
								radius * sinAngle,
								0.0f);
		
		// Rotate the direction vector according to the current emission vector value.
		Vector3 zNormalVector(0.0f, 0.0f, 1.0f);
		Vector3 curEmissionVector = emissionVector;
        if (FLOAT_EQUAL(curEmissionVector.Length(), 0.f) == false)
        {
            curEmissionVector.Normalize();
        }

		// This code rotates the (XY) plane with the particles to the direction vector.
		// Taking into account that a normal vector to the (XY) plane is (0,0,1) this
		// code is very simplified version of the generic "plane rotation" code.
		float32 length = curEmissionVector.Length();
		if (FLOAT_EQUAL(length, 0.0f) == false)
		{
			float32 cosAngleRot = curEmissionVector.z / length;
			float32 angleRot = acos(cosAngleRot);
			Vector3 axisRot(curEmissionVector.y, -curEmissionVector.x, 0);
            if (FLOAT_EQUAL(axisRot.Length(), 0.f) == false)
            {
                axisRot.Normalize();
            }
			Matrix3 planeRotMatrix;
			planeRotMatrix.CreateRotation(axisRot, angleRot);
			Vector3 rotatedVector = directionVector * planeRotMatrix;
			directionVector = rotatedVector;
		}
		
		Vector3 pos = center - directionVector;
		pts.AddPoint(pos);
	}
	
	if (useFilling)
	{
		FillPolygon(pts, renderState);
	}
	else
	{
    	DrawPolygon(pts, false, renderState);
	}
#endif // RHI_COMPLETE
}

void RenderHelper::DrawCylinder(const Vector3 & center, float32 radius, bool useFilling, NMaterial *material)
{
#ifdef RHI_COMPLETE
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

	Polygon3 pts;
    float32 angle = Min(PI/6.0f, SEGMENT_LENGTH / radius);// maximum angle 30 degrees
	int32 ptsCount = (int32)(PI_2 / (DegToRad(angle))) + 1;

	Vector<Vector2> vertexes;
    vertexes.reserve(ptsCount + 1);
	for(int32 i = 0; i <= ptsCount; i++)
 	{
		float32 seta = i * 360.0f / (float32)ptsCount;
  		float32 x = sin(DegToRad(seta)) * radius;
  		float32 y = cos(DegToRad(seta)) * radius;

		vertexes.push_back(Vector2(x, y));
	}
	
    pts.points.reserve(ptsCount * 6);
	for(int32 i = 0; i < ptsCount; ++i)
	{
		pts.AddPoint((Vector3(vertexes[i].x,  vertexes[i].y,  1) * radius) + center);
		pts.AddPoint((Vector3(vertexes[i].x,  vertexes[i].y,  -1) * radius) + center);
		pts.AddPoint((Vector3(vertexes[i+1].x, vertexes[i+1].y,  -1) * radius) + center);
		pts.AddPoint((Vector3(vertexes[i].x,  vertexes[i].y,  1) * radius) + center);
		pts.AddPoint((Vector3(vertexes[i+1].x, vertexes[i+1].y,  1) * radius) + center);
		pts.AddPoint((Vector3(vertexes[i+1].x, vertexes[i+1].y,  -1) * radius) + center);
	}
	
	if (useFilling)
	{
		FillPolygon(pts, renderState);
	}
	else
	{
		DrawPolygon(pts, true, renderState);
	}
#endif // RHI_COMPLETE
}

void RenderHelper::DrawPolygonPoints(const Polygon3 & polygon, NMaterial *material)
{
#if RHI_COMPLETE
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    int ptCount = polygon.pointCount;
    if (ptCount >= 1)
    {
#if defined (__DAVAENGINE_OPENGL__) && !defined(__DAVAENGINE_OPENGL_ES__)
        glPointSize(3.0f);
#endif 
        vertexStream->Set(TYPE_FLOAT, 3, 0, polygon.GetPoints());
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
        RenderManager::Instance()->SetRenderData(renderDataObject);
        RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_POINTLIST, 0, ptCount);
#if defined (__DAVAENGINE_OPENGL__) && !defined(__DAVAENGINE_OPENGL_ES__)
        glPointSize(1.0f);
#endif		
    }
#endif
}

void RenderHelper::DrawPolygon(const Polygon3 & polygon, bool closed, NMaterial *material)
{
#ifdef RHI_COMPLETE
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    int ptCount = polygon.pointCount;
    if (ptCount >= 2)
    {        
        vertexStream->Set(TYPE_FLOAT, 3, 0, polygon.GetPoints());
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
        RenderManager::Instance()->SetRenderData(renderDataObject);
        RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINESTRIP, 0, ptCount);

        if (closed)
        {
            Vector3 line[2] = { Vector3(polygon.GetPoints()[0]), Vector3(polygon.GetPoints()[ptCount - 1]) };
            vertexStream->Set(TYPE_FLOAT, 3, 0, line);
            RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINESTRIP, 0, 2);
        }
}
#endif // RHI_COMPLETE
}

void RenderHelper::FillPolygon(const Polygon3 & polygon, NMaterial *material)
{
#ifdef RHI_COMPLETE
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    int ptCount = polygon.pointCount;
	if (ptCount >= 3)
	{		
		vertexStream->Set(TYPE_FLOAT, 3, 0, polygon.GetPoints());
        RenderManager::Instance()->SetRenderState(renderState);
        RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
		RenderManager::Instance()->SetRenderData(renderDataObject);
		RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLEFAN, 0, ptCount);
    }
#endif // RHI_COMPLETE
}

void RenderHelper::DrawBSpline(BezierSpline3 * bSpline, int segments, float ts, float te, NMaterial *material)
{
	Polygon3 pts;
    pts.points.reserve(segments);
	for (int k = 0; k < segments; ++k)
	{
		pts.AddPoint(bSpline->Evaluate(0, ts + (te - ts) * ((float)k / (float)(segments - 1))));
	}
    DrawPolygon(pts, false, material);
}
	
void RenderHelper::DrawInterpolationFunc(Interpolation::Func func, const Rect & destRect, NMaterial *material)
{
	Polygon3 pts;
	int segmentsCount = 20;
    pts.points.reserve(segmentsCount);
	for (int k = 0; k < segmentsCount; ++k)
	{
		Vector3 v;
		v.x = destRect.x + ((float)k / (float)(segmentsCount - 1)) * destRect.dx;
		v.y = destRect.y + func(((float)k / (float)(segmentsCount - 1))) * destRect.dy;
		v.z = 0.0f;
		pts.AddPoint(v);
	}
	DrawPolygon(pts, false, material);
}
	
void RenderHelper::DrawBox(const AABBox2 & box, float32 lineWidth, NMaterial *material)
{
    RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.min.y, 0), Vector3(box.max.x, box.min.y, 0), lineWidth, material);
    RenderHelper::Instance()->DrawLine(Vector3(box.max.x, box.min.y, 0), Vector3(box.max.x, box.max.y, 0), lineWidth, material);
    RenderHelper::Instance()->DrawLine(Vector3(box.max.x, box.max.y, 0), Vector3(box.min.x, box.max.y, 0), lineWidth, material);
    RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.max.y, 0), Vector3(box.min.x, box.min.y, 0), lineWidth, material);
}
	
void RenderHelper::DrawBox(const AABBox3 & box, float32 lineWidth, NMaterial *material)
{
    RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.min.y, box.min.z), Vector3(box.min.x, box.min.y, box.max.z), lineWidth, material);
    RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.min.y, box.min.z), Vector3(box.min.x, box.max.y, box.min.z), lineWidth, material);
    RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.max.y, box.max.z), Vector3(box.min.x, box.min.y, box.max.z), lineWidth, material);
    RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.max.y, box.max.z), Vector3(box.min.x, box.max.y, box.min.z), lineWidth, material);
	
    RenderHelper::Instance()->DrawLine(Vector3(box.max.x, box.min.y, box.min.z), Vector3(box.max.x, box.min.y, box.max.z), lineWidth, material);
	RenderHelper::Instance()->DrawLine(Vector3(box.max.x, box.min.y, box.min.z), Vector3(box.max.x, box.max.y, box.min.z), lineWidth, material);
	RenderHelper::Instance()->DrawLine(Vector3(box.max.x, box.max.y, box.max.z), Vector3(box.max.x, box.min.y, box.max.z), lineWidth, material);
	RenderHelper::Instance()->DrawLine(Vector3(box.max.x, box.max.y, box.max.z), Vector3(box.max.x, box.max.y, box.min.z), lineWidth, material);
	
	
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.min.y, box.min.z), Vector3(box.max.x, box.min.y, box.min.z), lineWidth, material);
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.max.y, box.min.z), Vector3(box.max.x, box.max.y, box.min.z), lineWidth, material);
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.min.y, box.max.z), Vector3(box.max.x, box.min.y, box.max.z), lineWidth, material);
	RenderHelper::Instance()->DrawLine(Vector3(box.min.x, box.max.y, box.max.z), Vector3(box.max.x, box.max.y, box.max.z), lineWidth, material);
}
	
void RenderHelper::DrawCornerBox(const AABBox3 & bbox, float32 lineWidth, NMaterial *material)
{
	float32 offs = ((bbox.max - bbox.min).Length()) * 0.1f + 0.1f;
    
    //1
    Vector3 point = bbox.min;
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, 0, offs), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, offs, 0), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(offs, 0, 0), lineWidth, material);
    
    //2
    point = bbox.max;
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, 0, offs), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, offs, 0), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(offs, 0, 0), lineWidth, material);
    
    //3
    point = Vector3(bbox.min.x, bbox.max.y, bbox.min.z);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, 0, offs), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, offs, 0), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(offs, 0, 0), lineWidth, material);
    
    //4
    point = Vector3(bbox.max.x, bbox.max.y, bbox.min.z);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, 0, offs), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, offs, 0), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(offs, 0, 0), lineWidth, material);
    
    //5
    point = Vector3(bbox.max.x, bbox.min.y, bbox.min.z);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, 0, offs), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, offs, 0), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(offs, 0, 0), lineWidth, material);
    
    //6
    point = Vector3(bbox.min.x, bbox.max.y, bbox.max.z);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, 0, offs), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, offs, 0), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(offs, 0, 0), lineWidth, material);
    
    //7
    point = Vector3(bbox.min.x, bbox.min.y, bbox.max.z);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, 0, offs), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, offs, 0), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(offs, 0, 0), lineWidth, material);
    
    //8
    point = Vector3(bbox.max.x, bbox.min.y, bbox.max.z);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(0, 0, offs), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point + Vector3(0, offs, 0), lineWidth, material);
    RenderHelper::Instance()->DrawLine(point, point - Vector3(offs, 0, 0), lineWidth, material);
}
	
void RenderHelper::DrawSphere(const Vector3 &center, float32 radius, float32 lineWidth, NMaterial *material)
{
	int32 n = 2;
    Vector<Vector3> points;
    Vector<int32> triangleIndices;

	int32 e;
	float32 segmentRad = PI / (2.0f * ((float32)(n + 1)));
	int32 numberOfSeparators = 4 * n + 4;
				
	for (e = -n; e <= n; e++)
	{
		float32 r_e = radius * cosf(segmentRad * e);
		float32 y_e = radius * sinf(segmentRad * e);
			
		for (int s = 0; s < numberOfSeparators; s++)
		{
			float32 z_s = r_e * sinf(segmentRad * s) * (-1.0f);
			float32 x_s = r_e * cosf(segmentRad * s);
			points.push_back(Vector3(x_s, y_e, z_s));
		}
	}
	points.push_back(Vector3(0, radius, 0));
	points.push_back(Vector3(0, -radius, 0));
		
	for (e = 0; e < 4 * n ; e++)
	{
		for (int i = 0; i < numberOfSeparators; i++)
		{
			triangleIndices.push_back(e * numberOfSeparators + i);
			triangleIndices.push_back(e * numberOfSeparators + i + 
								numberOfSeparators);
			triangleIndices.push_back(e * numberOfSeparators + (i + 1) % 
								numberOfSeparators + numberOfSeparators);
				
			triangleIndices.push_back(e * numberOfSeparators + (i + 1) % 
								numberOfSeparators + numberOfSeparators);
			triangleIndices.push_back(e * numberOfSeparators + 
								(i + 1) % numberOfSeparators);
			triangleIndices.push_back(e * numberOfSeparators + i);
		}
	}
		
//		for (int i = 0; i < numberOfSeparators; i++)
//		{
//			triangleIndices.push_back(e * numberOfSeparators + i);
//			triangleIndices.push_back(numberOfSeparators * (2 * n + 1));
//			triangleIndices.push_back(e * numberOfSeparators + (i + 1) %
//								numberOfSeparators);
//		}
		
//		for (int i = 0; i < numberOfSeparators; i++)
//		{
//			triangleIndices.push_back(i);
//			triangleIndices.push_back((i + 1) % numberOfSeparators);
//			triangleIndices.push_back(numberOfSeparators * (2 * n + 1) + 1);
//		}
		
	//draw
		
	int32 size = static_cast<int32>(triangleIndices.size()/3);
	for (int i = 0; i < size; i++)
	{
		Vector3 p1 = points[triangleIndices[i]] + center;
		Vector3 p2 = points[triangleIndices[i + 1]] + center;
		Vector3 p3 = points[triangleIndices[i + 2]] + center;
						
		RenderHelper::Instance()->DrawLine(p1, p2, lineWidth, material);
		RenderHelper::Instance()->DrawLine(p1, p3, lineWidth, material);
		RenderHelper::Instance()->DrawLine(p2, p3, lineWidth, material);
		
		p1.y = -p1.y;
		p2.y = -p2.y;
		p3.y = -p3.y;
		
		RenderHelper::Instance()->DrawLine(p1, p2, lineWidth, material);
		RenderHelper::Instance()->DrawLine(p1, p3, lineWidth, material);
		RenderHelper::Instance()->DrawLine(p2, p3, lineWidth, material);
	}			
}

void RenderHelper::FillSphere(const Vector3 &center, float32 radius, NMaterial *material)
{
	int32 n = 2;
	Vector<Vector3> points;
	Vector<int32> triangleIndices;

	int32 e;
	float32 segmentRad = PI / (2.0f * ((float32)(n + 1)));
	int32 numberOfSeparators = 4 * n + 4;

	for (e = -n; e <= n; e++)
	{
		float32 r_e = radius * cosf(segmentRad * e);
		float32 y_e = radius * sinf(segmentRad * e);

		for (int s = 0; s < numberOfSeparators; s++)
		{
			float32 z_s = r_e * sinf(segmentRad * s) * (-1.0f);
			float32 x_s = r_e * cosf(segmentRad * s);
			points.push_back(Vector3(x_s, y_e, z_s));
		}
	}
	points.push_back(Vector3(0, radius, 0));
	points.push_back(Vector3(0, -radius, 0));

	for (e = 0; e < 4 * n ; e++)
	{
		for (int i = 0; i < numberOfSeparators; i++)
		{
			triangleIndices.push_back(e * numberOfSeparators + i);
			triangleIndices.push_back(e * numberOfSeparators + i + 
				numberOfSeparators);
			triangleIndices.push_back(e * numberOfSeparators + (i + 1) % 
				numberOfSeparators + numberOfSeparators);

			triangleIndices.push_back(e * numberOfSeparators + (i + 1) % 
				numberOfSeparators + numberOfSeparators);
			triangleIndices.push_back(e * numberOfSeparators + 
				(i + 1) % numberOfSeparators);
			triangleIndices.push_back(e * numberOfSeparators + i);
		}
	}

	//fill

	int32 size = static_cast<int32>(triangleIndices.size()/3);
	for (int i = 0; i < size; i++)
	{
		Vector3 p1 = points[triangleIndices[i]] + center;
		Vector3 p2 = points[triangleIndices[i + 1]] + center;
		Vector3 p3 = points[triangleIndices[i + 2]] + center;

		Polygon3 poly;
		poly.AddPoint(p1);
		poly.AddPoint(p3);
		poly.AddPoint(p2);
		RenderHelper::Instance()->FillPolygon(poly, material);

		p1.y = 2 * center.y - p1.y;
		p2.y = 2 * center.y - p2.y;
		p3.y = 2 * center.y - p3.y;

		poly.Clear();
		poly.AddPoint(p1);
		poly.AddPoint(p3);
		poly.AddPoint(p2);
		RenderHelper::Instance()->FillPolygon(poly, material);
	}			
}

void RenderHelper::DrawArrow(const Vector3 &from, const Vector3 &to, float32 arrowLength, float32 lineWidth, NMaterial *material)
{
	if(0 != lineWidth && from != to)
	{
		Vector3 d = to - from;
		float32 ln = Min(arrowLength, d.Length());

		Vector3 c;
		if(ln < 1)
		{
			c = to - d * ln;
		}
		else
		{
			c = to - d / ln;
		}

		DAVA::float32 k = (to - c).Length() / 4;

		Vector3 n = c.CrossProduct(to);
		n.Normalize();
		n *= k;

		Vector3 p1 = c + n;
		Vector3 p2 = c - n;

		Vector3 nd = d.CrossProduct(n);
		nd.Normalize();
		nd *= k;

		Vector3 p3 = c + nd;
		Vector3 p4 = c - nd;

		RenderHelper::Instance()->DrawLine(from, c, lineWidth, material);

		RenderHelper::Instance()->DrawLine(p1, p3, lineWidth, material);
		RenderHelper::Instance()->DrawLine(p2, p3, lineWidth, material);
		RenderHelper::Instance()->DrawLine(p1, p4, lineWidth, material);
		RenderHelper::Instance()->DrawLine(p2, p4, lineWidth, material);
		RenderHelper::Instance()->DrawLine(p1, to, lineWidth, material);
		RenderHelper::Instance()->DrawLine(p2, to, lineWidth, material);
		RenderHelper::Instance()->DrawLine(p3, to, lineWidth, material);
		RenderHelper::Instance()->DrawLine(p4, to, lineWidth, material);
	}
}

void RenderHelper::FillArrow(const Vector3 &from, const Vector3 &to, float32 arrowLength, float32 lineWidth, NMaterial *material)
{
	Vector3 d = to - from;
	Vector3 c = to - (d * arrowLength / d.Length());

	DAVA::float32 k = arrowLength / 4;

	Vector3 n = c.CrossProduct(to);

	if(n.IsZero())
	{
		if(0 == to.x) n = Vector3(1, 0, 0);
		else if(0 == to.y) n = Vector3(0, 1, 0);
		else if(0 == to.z) n = Vector3(0, 0, 1);
	}

	n.Normalize();
	n *= k;

	Vector3 p1 = c + n;
	Vector3 p2 = c - n;

	Vector3 nd = d.CrossProduct(n);
	nd.Normalize();
	nd *= k;

	Vector3 p3 = c + nd;
	Vector3 p4 = c - nd;

	Polygon3 poly;
    poly.points.reserve(3);
        
	poly.AddPoint(p1);
	poly.AddPoint(p3);
	poly.AddPoint(p2);
	RenderHelper::Instance()->FillPolygon(poly, material);

	poly.Clear();
	poly.AddPoint(p1);
	poly.AddPoint(p4);
	poly.AddPoint(p2);
	RenderHelper::Instance()->FillPolygon(poly, material);

	poly.Clear();
	poly.AddPoint(p1);
	poly.AddPoint(p3);
	poly.AddPoint(to);
	RenderHelper::Instance()->FillPolygon(poly, material);

	poly.Clear();
	poly.AddPoint(p1);
	poly.AddPoint(p4);
	poly.AddPoint(to);
	RenderHelper::Instance()->FillPolygon(poly, material);

	poly.Clear();
	poly.AddPoint(p2);
	poly.AddPoint(p3);
	poly.AddPoint(to);
	RenderHelper::Instance()->FillPolygon(poly, material);

	poly.Clear();
	poly.AddPoint(p2);
	poly.AddPoint(p4);
	poly.AddPoint(to);
	RenderHelper::Instance()->FillPolygon(poly, material);

	if(0 != lineWidth)
	{
		RenderHelper::Instance()->DrawLine(from, c, lineWidth, material);
	}
}

void RenderHelper::FillBox(const AABBox3 & box, NMaterial *material)
{
	DAVA::Vector3 min = box.min;
	DAVA::Vector3 max = box.max;

	DAVA::Polygon3 poly;
    poly.points.reserve(4);
        
	poly.AddPoint(min);
	poly.AddPoint(DAVA::Vector3(min.x, min.y, max.z));
	poly.AddPoint(DAVA::Vector3(min.x, max.y, max.z));
	poly.AddPoint(DAVA::Vector3(min.x, max.y, min.z));
	RenderHelper::Instance()->FillPolygon(poly, material);

	poly.Clear();
	poly.AddPoint(min);
	poly.AddPoint(DAVA::Vector3(min.x, max.y, min.z));
	poly.AddPoint(DAVA::Vector3(max.x, max.y, min.z));
	poly.AddPoint(DAVA::Vector3(max.x, min.y, min.z));
	RenderHelper::Instance()->FillPolygon(poly, material);

	poly.Clear();
	poly.AddPoint(min);
	poly.AddPoint(DAVA::Vector3(min.x, min.y, max.z));
	poly.AddPoint(DAVA::Vector3(max.x, min.y, max.z));
	poly.AddPoint(DAVA::Vector3(max.x, min.y, min.z));
	RenderHelper::Instance()->FillPolygon(poly, material);

	poly.Clear();
	poly.AddPoint(max);
	poly.AddPoint(DAVA::Vector3(max.x, max.y, min.z));
	poly.AddPoint(DAVA::Vector3(max.x, min.y, min.z));
	poly.AddPoint(DAVA::Vector3(max.x, min.y, max.z));
	RenderHelper::Instance()->FillPolygon(poly, material);

	poly.Clear();
	poly.AddPoint(max);
	poly.AddPoint(DAVA::Vector3(max.x, max.y, min.z));
	poly.AddPoint(DAVA::Vector3(min.x, max.y, min.z));
	poly.AddPoint(DAVA::Vector3(min.x, max.y, max.z));
	RenderHelper::Instance()->FillPolygon(poly, material);

	poly.Clear();
	poly.AddPoint(max);
	poly.AddPoint(DAVA::Vector3(max.x, min.y, max.z));
	poly.AddPoint(DAVA::Vector3(min.x, min.y, max.z));
	poly.AddPoint(DAVA::Vector3(min.x, max.y, max.z));
	RenderHelper::Instance()->FillPolygon(poly, material);
}

void RenderHelper::DrawDodecahedron(const Vector3 &center, float32 radius, float32 lineWidth /* = 1.f */, NMaterial *material)
{
#ifdef RHI_COMPLETE
    RenderSystem2D::Instance()->Flush();

    RenderSystem2D::Instance()->UpdateClip();

    if (gDodecObject->GetIndexBufferID() != 0)
    {
        gDodecObject->BuildVertexBuffer(sizeof(gDodecVertexes) / sizeof(gDodecVertexes[0]));
        gDodecObject->BuildIndexBuffer();
    }
        
	Matrix4 drawMatrix;
	drawMatrix.CreateScale(DAVA::Vector3(radius, radius, radius));
	drawMatrix.SetTranslationVector(center);

	RenderManager::Instance()->SetDynamicParam(PARAM_WORLD, &drawMatrix, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    RenderManager::Instance()->SetRenderState(renderState);
    RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
	RenderManager::Instance()->SetRenderData(gDodecObject);
	RenderManager::Instance()->AttachRenderData();
    RenderManager::Instance()->FlushState();

	if(gDodecObject->GetIndexBufferID() != 0)
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_LINELIST, sizeof(gDodecIndexes) / sizeof(gDodecIndexes[0]), EIF_16, 0);
	}
	else
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_LINELIST, sizeof(gDodecIndexes) / sizeof(gDodecIndexes[0]), EIF_16, gDodecIndexes);
	}
#endif // RHI_COMPLETE
}

void RenderHelper::FillDodecahedron(const Vector3 &center, float32 radius, NMaterial *material)
{
#ifdef RHI_COMPLETE
    RenderSystem2D::Instance()->UpdateClip();

    if (gDodecObject->GetIndexBufferID() != 0)
    {
        gDodecObject->BuildVertexBuffer(sizeof(gDodecVertexes) / sizeof(gDodecVertexes[0]));
        gDodecObject->BuildIndexBuffer();
    }

	Matrix4 drawMatrix;
	drawMatrix.CreateScale(DAVA::Vector3(radius, radius, radius));
	drawMatrix.SetTranslationVector(center);

	RenderManager::Instance()->SetDynamicParam(PARAM_WORLD, &drawMatrix, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    RenderManager::Instance()->SetRenderState(renderState);
    RenderManager::Instance()->SetRenderEffect(RenderSystem2D::FLAT_COLOR);
	RenderManager::Instance()->SetRenderData(gDodecObject);
	RenderManager::Instance()->AttachRenderData();
    RenderManager::Instance()->FlushState();
        
	if(gDodecObject->GetIndexBufferID() != 0)
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, sizeof(gDodecIndexes) / sizeof(gDodecIndexes[0]), EIF_16, 0);
	}
	else
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, sizeof(gDodecIndexes) / sizeof(gDodecIndexes[0]), EIF_16, gDodecIndexes);
	}
#endif // RHI_COMPLETE
}

void RenderHelper::CreateClearPass(rhi::HTexture targetHandle, int32 passPriority, const Color & clearColor, const rhi::Viewport & viewport)
{
    rhi::RenderPassConfig clearPassConfig;
    clearPassConfig.priority = passPriority;
    clearPassConfig.colorBuffer[0].texture = targetHandle;
    clearPassConfig.colorBuffer[0].clearColor[0] = clearColor.r;
    clearPassConfig.colorBuffer[0].clearColor[1] = clearColor.g;
    clearPassConfig.colorBuffer[0].clearColor[2] = clearColor.b;
    clearPassConfig.colorBuffer[0].clearColor[3] = clearColor.a;
    clearPassConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    clearPassConfig.colorBuffer[0].storeAction = rhi::STOREACTION_NONE;
    clearPassConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    clearPassConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    clearPassConfig.viewport = viewport;

    rhi::HPacketList emptyPacketList;
    rhi::HRenderPass clearPass = rhi::AllocateRenderPass(clearPassConfig, 1, &emptyPacketList);

    rhi::BeginRenderPass(clearPass);
    rhi::BeginPacketList(emptyPacketList);
    rhi::EndPacketList(emptyPacketList);
    rhi::EndRenderPass(clearPass);
}

#if RHI_COMPLETE
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WINDOWS__)
void RenderHelper::GetLineWidthRange(int32& rangeMin, int32& rangeMax)
{
	int32 lineWidthMin = 1;
	int32 lineWidthMax = 1;

#if defined (__DAVAENGINE_OPENGL__)
	GLint range[2];
	glGetIntegerv(GL_LINE_WIDTH_RANGE, range);
	lineWidthMin = range[0];
	lineWidthMax = range[1];
#endif

	rangeMin = lineWidthMin;
	rangeMax = lineWidthMax;
}
#endif
#endif //RHI_COMPLETE

};
