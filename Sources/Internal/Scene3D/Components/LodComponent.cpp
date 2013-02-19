#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/SceneNode.h"

namespace DAVA
{

const float32 LodComponent::INVALID_DISTANCE = -1.f;
const float32 LodComponent::MIN_LOD_DISTANCE = 0.f;
const float32 LodComponent::MAX_LOD_DISTANCE = 500.f;

LodComponent::LodDistance::LodDistance()
{
	distance = nearDistance = nearDistanceSq = farDistance = farDistanceSq = (float32) INVALID_DISTANCE;
}

void LodComponent::LodDistance::SetDistance(const float32 &newDistance)
{
	distance = newDistance;
}

void LodComponent::LodDistance::SetNearDistance(const float32 &newDistance)
{
	nearDistance = newDistance;
	nearDistanceSq = nearDistance * nearDistance;
}

void LodComponent::LodDistance::SetFarDistance(const float32 &newDistance)
{
	farDistance = newDistance;
	farDistanceSq = farDistance * farDistance;
}

Component * LodComponent::Clone(SceneNode * toEntity)
{
	LodComponent * newLod = new LodComponent();
	newLod->SetEntity(toEntity);

	newLod->lodLayers = lodLayers;
	const List<LodData>::const_iterator endLod = newLod->lodLayers.end();
	for (List<LodData>::iterator it = newLod->lodLayers.begin(); it != endLod; ++it)
	{
		LodData & ld = *it;
		ld.nodes.clear();
	}
	//if(!newLod->lodLayers.empty())
	//{
	//	const List<LodData>::const_iterator endLod = newLod->lodLayers.end();
	//	newLod->currentLod = &(*newLod->lodLayers.begin());
	//	for (List<LodData>::iterator it = newLod->lodLayers.begin(); it != endLod; ++it)
	//	{
	//		LodData & ld = *it;
	//		uint32 size = ld.nodes.size();
	//		for (uint32 idx = 0; idx < size; ++idx)
	//		{
	//			int32 count = entity->GetChildrenCount();
	//			for (int32 i = 0; i < count; i++) 
	//			{
	//				SceneNode * child = entity->GetChild(i);
	//				if(child == ld.nodes[idx])
	//				{
	//					ld.nodes[idx] = toEntity->GetChild(i);
	//					if (newLod->currentLod != &ld) 
	//					{
	//						ld.nodes[idx]->SetUpdatable(false);
	//					}
	//					else 
	//					{
	//						ld.nodes[idx]->SetUpdatable(true);
	//					}

	//					break;
	//				}
	//			}
	//		}
	//	}
	//}

	//Lod values
	for(int32 iLayer = 0; iLayer < MAX_LOD_LAYERS; ++iLayer)
	{
		newLod->lodLayersArray[iLayer].distance = lodLayersArray[iLayer].distance;
		newLod->lodLayersArray[iLayer].nearDistance = lodLayersArray[iLayer].nearDistance;
		newLod->lodLayersArray[iLayer].nearDistanceSq = lodLayersArray[iLayer].nearDistanceSq;
		newLod->lodLayersArray[iLayer].farDistance = lodLayersArray[iLayer].farDistance;
		newLod->lodLayersArray[iLayer].farDistanceSq = lodLayersArray[iLayer].farDistanceSq;
	}

	newLod->forceDistance = forceDistance;
	newLod->forceDistanceSq = forceDistanceSq;
	newLod->forceLodLayer = forceLodLayer;

	return newLod;
}

void LodComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive)
	{
		uint32 i;

		archive->SetUInt32("lc.flags", flags);
		archive->SetFloat("lc.forceDistance", forceDistance);
		archive->SetFloat("lc.forceDistanceSq", forceDistanceSq);
		archive->SetInt32("lc.forceLodLayer", forceLodLayer);

		KeyedArchive *lodDistArch = new KeyedArchive();
		for (i = 0; i < MAX_LOD_LAYERS; ++i)
		{
			lodDistArch->SetFloat(KeyedArchive::GenKeyFromIndex(i), lodLayersArray[i].GetDistance());
		}
		archive->SetArchive("lc.loddist", lodDistArch);
		lodDistArch->Release();

		i = 0;
		KeyedArchive *lodDataArch = new KeyedArchive();
		List<LodData>::iterator it = lodLayers.begin();
		for(; it != lodLayers.end(); ++it)
		{
			KeyedArchive *lodDataIndexesArch = new KeyedArchive();
			for(uint32 j = 0; j < it->indexes.size(); ++j)
			{
				lodDataIndexesArch->SetInt32(KeyedArchive::GenKeyFromIndex(j), it->indexes[j]);
			}

			lodDataArch->SetInt32("layer", it->layer);
			lodDataArch->SetBool("isdummy", it->isDummy);
			lodDataArch->SetArchive("indexes", lodDataIndexesArch);
			lodDataArch->SetUInt32("indexescount", it->indexes.size());
			lodDataIndexesArch->Release();
			++i;
		}
		archive->SetUInt32("lc.loddatacount", lodLayers.size());
		archive->SetArchive("lc.loddata", lodDataArch);
		lodDataArch->Release();
	}
}

void LodComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive)
	{
		if(archive->IsKeyExists("lc.flags")) flags = archive->GetUInt32("lc.flags");
		if(archive->IsKeyExists("lc.forceDistance")) forceDistance = archive->GetFloat("lc.forceDistance");
		if(archive->IsKeyExists("lc.forceDistanceSq")) forceDistanceSq = archive->GetFloat("lc.forceDistanceSq");
		if(archive->IsKeyExists("lc.forceLodLayer")) forceLodLayer = archive->GetInt32("lc.forceLodLayer");

		KeyedArchive *lodDistArch = archive->GetArchive("lc.loddist");
		if(NULL != lodDistArch)
		{
			for(uint32 i = 0; i < MAX_LOD_LAYERS; ++i)
			{
				lodLayersArray[i].SetDistance(lodDistArch->GetFloat(KeyedArchive::GenKeyFromIndex(i)));
			}
		}

		KeyedArchive *lodDataArch = archive->GetArchive("lc.loddata");
		if(NULL != lodDataArch)
		{
			for(uint32 i = 0; i < archive->GetUInt32("lc.loddatacount"); ++i)
			{
				LodData data;

				if(lodDataArch->IsKeyExists("layer")) data.layer = lodDataArch->GetUInt32("layer");
				if(lodDataArch->IsKeyExists("isdummy")) data.isDummy = lodDataArch->GetBool("isdummy");

				KeyedArchive *lodDataIndexesArch = lodDataArch->GetArchive("indexes");
				if(NULL != lodDataIndexesArch)
				{
					for(uint32 j = 0; j < lodDataArch->GetUInt32("indexescount"); ++j)
					{
						data.indexes.push_back(lodDataIndexesArch->GetInt32(KeyedArchive::GenKeyFromIndex(j)));
					}
				}

				lodLayers.push_back(data);
			}
		}
	}
}

LodComponent::LodComponent()
:	forceLodLayer(INVALID_LOD_LAYER),
	forceDistance(INVALID_DISTANCE),
	forceDistanceSq(INVALID_DISTANCE)
{
	lodLayersArray.resize(MAX_LOD_LAYERS);

	flags = NEED_UPDATE_AFTER_LOAD;

	for(int32 iLayer = 0; iLayer < MAX_LOD_LAYERS; ++iLayer)
	{
		lodLayersArray[iLayer].SetDistance(GetDefaultDistance(iLayer));
		lodLayersArray[iLayer].SetFarDistance(MAX_LOD_DISTANCE * 2);
	}

	lodLayersArray[0].SetNearDistance(0.0f);
}

float32 LodComponent::GetDefaultDistance(int32 layer)
{
	float32 distance = MIN_LOD_DISTANCE + ((float32)(MAX_LOD_DISTANCE - MIN_LOD_DISTANCE) / (MAX_LOD_LAYERS-1)) * layer;
	return distance;
}

void LodComponent::SetCurrentLod(LodData *newLod)
{
	if (newLod != currentLod) 
	{
		if (currentLod) 
		{
			int32 size = currentLod->nodes.size();
			for (int i = 0; i < size; i++) 
			{
				currentLod->nodes[i]->SetLodVisible(false);
			}
		}
		currentLod = newLod;
		int32 size = currentLod->nodes.size();
		for (int i = 0; i < size; i++) 
		{
			currentLod->nodes[i]->SetLodVisible(true);
		}
	}
}

void LodComponent::SetForceDistance(const float32 &newDistance)
{
    forceDistance = newDistance;
    forceDistanceSq = forceDistance * forceDistance;
}
    
float32 LodComponent::GetForceDistance() const
{
    return forceDistance;
}

void LodComponent::GetLodData(List<LodData*> &retLodLayers)
{
	retLodLayers.clear();

	List<LodData>::const_iterator endIt = lodLayers.end();
	for(List<LodData>::iterator it = lodLayers.begin(); it != endIt; ++it)
	{
		LodData *ld = &(*it);
		retLodLayers.push_back(ld);
	}
}
    
void LodComponent::SetLodLayerDistance(int32 layerNum, float32 distance)
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    
    if(INVALID_DISTANCE != distance)
    {
        float32 nearDistance = distance * 0.95f;
        float32 farDistance = distance * 1.05f;
        
        if(GetLodLayersCount() - 1 == layerNum)
        {
            lodLayersArray[layerNum].SetFarDistance(MAX_LOD_DISTANCE * 2);
        }
        if(layerNum)
        {
            lodLayersArray[layerNum-1].SetFarDistance(farDistance);
        }
        
        lodLayersArray[layerNum].SetDistance(distance);
        lodLayersArray[layerNum].SetNearDistance(nearDistance);
    }
    else 
    {
        lodLayersArray[layerNum].SetDistance(distance);
    }
}

void LodComponent::SetForceLodLayer(int32 layer)
{
    forceLodLayer = layer;
}
    
int32 LodComponent::GetForceLodLayer()
{
    return forceLodLayer;
}

int32 LodComponent::GetMaxLodLayer()
{
	int32 ret = -1;
	const List<LodData>::const_iterator &end = lodLayers.end();
	for (List<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
	{
		LodData & ld = *it;
		if(ld.layer > ret)
		{
			ret = ld.layer;
		}
	}

	return ret;
}

    
};
