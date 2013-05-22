#include "Scene/System/HoodSystem/NormalHood.h"
#include "Scene/System/ModifSystem.h"

// framework
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

NormalHood::NormalHood() : HoodObject(2.0f)
{
	axisX = CreateLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(baseSize, 0, 0));
	axisX->axis = ST_AXIS_X;

	axisY = CreateLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, baseSize, 0));
	axisY->axis = ST_AXIS_Y;

	axisZ = CreateLine(DAVA::Vector3(0, 0, 0), DAVA::Vector3(0, 0, baseSize));
	axisZ->axis = ST_AXIS_Z;
}

NormalHood::~NormalHood()
{

}

void NormalHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis)
{
	int oldState = DAVA::RenderManager::Instance()->GetState();
	DAVA::eBlendMode oldBlendSrc = DAVA::RenderManager::Instance()->GetSrcBlend();
	DAVA::eBlendMode oldBlendDst = DAVA::RenderManager::Instance()->GetDestBlend();

	DAVA::RenderManager::Instance()->SetState(DAVA::RenderState::STATE_BLEND | DAVA::RenderState::STATE_COLORMASK_ALL | DAVA::RenderState::STATE_DEPTH_WRITE);
	DAVA::RenderManager::Instance()->SetBlendMode(DAVA::BLEND_SRC_ALPHA, DAVA::BLEND_ONE_MINUS_SRC_ALPHA);

	// x
	if(selectedAxis & ST_AXIS_X) 
		DAVA::RenderManager::Instance()->SetColor(colorS);
	else 
		DAVA::RenderManager::Instance()->SetColor(colorX);

	DAVA::RenderHelper::Instance()->DrawLine(axisX->curFrom, axisX->curTo);

	// y
	if(selectedAxis & ST_AXIS_Y) 
		DAVA::RenderManager::Instance()->SetColor(colorS);
	else 
		DAVA::RenderManager::Instance()->SetColor(colorY);

	DAVA::RenderHelper::Instance()->DrawLine(axisY->curFrom, axisY->curTo);

	// z
	if(selectedAxis & ST_AXIS_Z) 
		DAVA::RenderManager::Instance()->SetColor(colorS);
	else 
		DAVA::RenderManager::Instance()->SetColor(colorZ);

	DAVA::RenderHelper::Instance()->DrawLine(axisZ->curFrom, axisZ->curTo);

	DAVA::RenderManager::Instance()->SetBlendMode(oldBlendSrc, oldBlendDst);
	DAVA::RenderManager::Instance()->SetState(oldState);
}
