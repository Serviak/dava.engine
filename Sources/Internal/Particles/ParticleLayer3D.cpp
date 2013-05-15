#include "Particles/ParticleLayer3D.h"
#include "Render/RenderDataObject.h"
#include "Render/RenderManager.h"
#include "Render/Material.h"
#include "Math/MathHelpers.h"
#include "Render/Highlevel/Camera.h"
#include "ParticleEmitter3D.h"

namespace DAVA
{

ParticleLayer3D::ParticleLayer3D(ParticleEmitter* parent)
{
	isLong = false;
	renderData = new RenderDataObject();
	this->parent = parent;

	//TODO: set material from outside
	
	Material * material = new Material();
	material->SetType(Material::MATERIAL_VERTEX_COLOR_ALPHABLENDED);
	material->SetAlphablend(true);
	material->SetBlendSrc(BLEND_SRC_ALPHA);
	material->SetBlendDest(BLEND_ONE);
	material->SetName("ParticleLayer3D_material");

	renderBatch->SetMaterial(material);
	SafeRelease(material);
}

ParticleLayer3D::~ParticleLayer3D()
{
	DeleteAllParticles();
	SafeRelease(renderData);
}

void ParticleLayer3D::Draw(Camera * camera)
{
	DrawLayer(camera);
}

void ParticleLayer3D::DrawLayer(Camera* camera)
{
	if (!sprite)
	{
		return;
	}

    Matrix4 rotationMatrix = Matrix4::IDENTITY;
    switch(RenderManager::Instance()->GetRenderOrientation())
    {
        case Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT:
            //glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
            rotationMatrix.CreateRotation(Vector3(0.0f, 0.0f, 1.0f), DegToRad(90.0f));
            break;
        case Core::SCREEN_ORIENTATION_LANDSCAPE_RIGHT:
            //glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
            rotationMatrix.CreateRotation(Vector3(0.0f, 0.0f, 1.0f), DegToRad(-90.0f));
            break;
    }

    Matrix4 mv = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW)*rotationMatrix;
    
	_up = Vector3(mv._01, mv._11, mv._21);
	_left = Vector3(mv._00, mv._10, mv._20);
	direction = camera->GetDirection();

	verts.clear();
	textures.clear();
	colors.clear();
	int32 totalCount = 0;

	// Reserve the memory for vectors to avoid the resize operations. Actually there can be less than count
	// particles (for Single Particle or Superemitter one), but never more than count.
	static const int32 POINTS_PER_PARTICLE = 6;
	verts.reserve(count * POINTS_PER_PARTICLE * 3); // 6 vertices per each particle, 3 coords per vertex.
	textures.reserve(count * POINTS_PER_PARTICLE * 2); // 6 texture coords per particle, 2 values per texture coord.
	colors.reserve(count * POINTS_PER_PARTICLE);
	
	Particle * current = head;
	if(current)
	{
		renderBatch->GetMaterial()->GetRenderState()->SetTexture(sprite->GetTexture(current->frame));
	}

	int32 verticesCount = 0;
	int32 texturesCount = 0;
	int32 colorsCount = 0;

	while(current != 0)
	{
		Vector3 topRight;
		Vector3 topLeft;
		Vector3 botRight;
		Vector3 botLeft;

		if (IsLong())
		{
			CalcLong(current, topLeft, topRight, botLeft, botRight);
		}
		else
		{
			CalcNonLong(current, topLeft, topRight, botLeft, botRight);
		}

		verts[verticesCount] = topLeft.x;//0
		verticesCount ++;
		verts[verticesCount] = topLeft.y;
		verticesCount ++;
		verts[verticesCount] = topLeft.z;
		verticesCount ++;

		verts[verticesCount] = topRight.x;//1
		verticesCount ++;
		verts[verticesCount] = topRight.y;
		verticesCount ++;
		verts[verticesCount] = topRight.z;
		verticesCount ++;

		verts[verticesCount] = botLeft.x;//2
		verticesCount ++;
		verts[verticesCount] = botLeft.y;
		verticesCount ++;
		verts[verticesCount] = botLeft.z;
		verticesCount ++;

		verts[verticesCount] = botLeft.x;//2
		verticesCount ++;
		verts[verticesCount] = botLeft.y;
		verticesCount ++;
		verts[verticesCount] = botLeft.z;
		verticesCount ++;

		verts[verticesCount] = topRight.x;//1
		verticesCount ++;
		verts[verticesCount] = topRight.y;
		verticesCount ++;
		verts[verticesCount] = topRight.z;
		verticesCount ++;

		verts[verticesCount] = botRight.x;//3
		verticesCount ++;
		verts[verticesCount] = botRight.y;
		verticesCount ++;
		verts[verticesCount] = botRight.z;
		verticesCount ++;

		float32 *pT = sprite->GetTextureVerts(current->frame);

		textures[texturesCount] = pT[0];
		texturesCount ++;
		textures[texturesCount] = pT[1];
		texturesCount ++;

		textures[texturesCount] = pT[2];
		texturesCount ++;
		textures[texturesCount] = pT[3];
		texturesCount ++;

		textures[texturesCount] = pT[4];
		texturesCount ++;
		textures[texturesCount] = pT[5];
		texturesCount ++;

		textures[texturesCount] = pT[4];
		texturesCount ++;
		textures[texturesCount] = pT[5];
		texturesCount ++;

		textures[texturesCount] = pT[2];
		texturesCount ++;
		textures[texturesCount] = pT[3];
		texturesCount ++;

		textures[texturesCount] = pT[6];
		texturesCount ++;
		textures[texturesCount] = pT[7];
		texturesCount ++;

		// Yuri Coder, 2013/04/03. Need to use drawColor here instead of just colot
		// to take colorOverlife property into account.
		uint32 color = (((uint32)(current->drawColor.a*255.f))<<24) |  (((uint32)(current->drawColor.b*255.f))<<16) |
			(((uint32)(current->drawColor.g*255.f))<<8) | ((uint32)(current->drawColor.r*255.f));
		for(int32 i = 0; i < POINTS_PER_PARTICLE; ++i)
		{
			colors[i + colorsCount] = color;
		}
		colorsCount += POINTS_PER_PARTICLE;

		totalCount++;
		current = TYPE_PARTICLES == type ? current->next : 0;
	}

	renderBatch->SetTotalCount(totalCount);
	if(totalCount > 0)
	{			
		renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, &verts.front());
		renderData->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, 0, &textures.front());
		renderData->SetStream(EVF_COLOR, TYPE_UNSIGNED_BYTE, 4, 0, &colors.front());

		if (IsLong())
		{
			RenderManager::Instance()->SetRenderData(renderData);
			renderBatch->GetMaterial()->PrepareRenderState();
		}
		renderBatch->SetRenderDataObject(renderData);
	}
}

void ParticleLayer3D::CalcNonLong(Particle* current,
								  Vector3& topLeft,
								  Vector3& topRight,
								  Vector3& botLeft,
								  Vector3& botRight)
{
	Vector3 dx(_left);
	Vector3 dy(_up);

	float32 sine;
	float32 cosine;
	SinCosFast(current->angle, sine, cosine);

	// Draw pivot point is Sprite center + layer pivot point.
	Vector2 drawPivotPoint = GetDrawPivotPoint();

	float32 pivotRight = ((sprite->GetWidth() /2 - drawPivotPoint.x)*current->size.x*current->sizeOverLife.x)/2.f;
	float32 pivotLeft = (drawPivotPoint.x*current->size.x*current->sizeOverLife.x)/2.f;
	float32 pivotUp = (drawPivotPoint.y*current->size.y*current->sizeOverLife.y)/2.f;
	float32 pivotDown = ((sprite->GetHeight()-drawPivotPoint.y)*current->size.y*current->sizeOverLife.y)/2.f;

	Vector3 dxc = dx*cosine;
	Vector3 dxs = dx*sine;
	Vector3 dyc = dy*cosine;
	Vector3 dys = dy*sine;

	// Apply offset to the current position according to the emitter position.
	UpdateCurrentParticlePosition(current);

	topLeft = currentParticlePosition+(dxs+dyc)*pivotLeft + (dxc-dys)*pivotDown;
	topRight = currentParticlePosition+(-dxc+dys)*pivotUp + (dxs+dyc)*pivotLeft;
	botLeft = currentParticlePosition+(dxc-dys)*pivotDown + (-dxs-dyc)*pivotRight;
	botRight = currentParticlePosition+(-dxs-dyc)*pivotRight + (-dxc+dys)*pivotUp;
}

void ParticleLayer3D::CalcLong(Particle* current,
							   Vector3& topLeft,
							   Vector3& topRight,
							   Vector3& botLeft,
							   Vector3& botRight)
{
	Vector3 vecShort = current->direction.CrossProduct(direction);
	vecShort /= 2.f;
		
	Vector3 vecLong = -current->direction;

	float32 widthDiv2 = sprite->GetWidth()*current->size.x*current->sizeOverLife.x;
	float32 heightDiv2 = sprite->GetHeight()*current->size.y*current->sizeOverLife.y;

	// Apply offset to the current position according to the emitter position.
	UpdateCurrentParticlePosition(current);

	topRight = currentParticlePosition + widthDiv2*vecShort;
	topLeft = currentParticlePosition - widthDiv2*vecShort;
	botRight = topRight + heightDiv2*vecLong;
	botLeft = topLeft + heightDiv2*vecLong;
}


void ParticleLayer3D::LoadFromYaml(const FilePath & configPath, YamlNode * node)
{
	ParticleLayer::LoadFromYaml(configPath, node);
	SetAdditive(additive);
}

ParticleLayer * ParticleLayer3D::Clone(ParticleLayer * dstLayer /*= 0*/)
{
	if(!dstLayer)
	{
		// YuriCoder, 2013/04/30. TODO - this part isn't supposed to work, since
		// dstLayer is always NULL here. Return to it later.
		ParticleEmitter* parentFor3DLayer = NULL;
		if (dynamic_cast<ParticleLayer3D*>(dstLayer))
		{
			parentFor3DLayer = (dynamic_cast<ParticleLayer3D*>(dstLayer))->GetParent();
		}

		dstLayer = new ParticleLayer3D(parentFor3DLayer);
		dstLayer->SetLong(this->isLong);
	}

	ParticleLayer::Clone(dstLayer);

	return dstLayer;
}

Material * ParticleLayer3D::GetMaterial()
{
	return renderBatch->GetMaterial();
}
	
void ParticleLayer3D::SetAdditive(bool additive)
{
	ParticleLayer::SetAdditive(additive);
	if(additive)
	{
		renderBatch->GetMaterial()->SetBlendSrc(BLEND_SRC_ALPHA);
		renderBatch->GetMaterial()->SetBlendDest(BLEND_ONE);
	}
	else
	{
		renderBatch->GetMaterial()->SetBlendSrc(BLEND_SRC_ALPHA);
		renderBatch->GetMaterial()->SetBlendDest(BLEND_ONE_MINUS_SRC_ALPHA);
	}
}

bool ParticleLayer3D::IsLong()
{
	return isLong;
}

void ParticleLayer3D::SetLong(bool value)
{
	isLong = value;
	renderBatch->GetMaterial()->SetTwoSided(isLong);
}

void ParticleLayer3D::UpdateCurrentParticlePosition(Particle* particle)
{
	if (this->parent)
	{
		// For Superemitter adjust the particle position according to the
		// current emitter position.
		this->currentParticlePosition = particle->position + (parent->GetPosition() - parent->GetInitialTranslationVector());
	}
	else
	{
		// For all other types just leave the particle position untouched.
		this->currentParticlePosition = particle->position;
	}
}

void ParticleLayer3D::CreateInnerEmitter()
{
	SafeRelease(this->innerEmitter);
	this->innerEmitter = new ParticleEmitter3D();
}

};
