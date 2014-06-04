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

#ifndef __DAVAENGINE_VEGETATIONCUSTOMSLGEOMETRYGENERATOR_H__
#define __DAVAENGINE_VEGETATIONCUSTOMSLGEOMETRYGENERATOR_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"
#include "Base/AbstractQuadTree.h"

#include "Render/3D/PolygonGroup.h"
#include "Render/RenderDataObject.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Material.h"
#include "Render/Material/NMaterial.h"

#include "Render/Highlevel/Vegetation/TextureSheet.h"
#include "Render/Highlevel/Vegetation/VegetationGeometry.h"
#include "Render/Highlevel/Vegetation/VegetationMaterialTransformer.h"
#include "Render/Highlevel/Vegetation/VegetationSpatialData.h"
#include "Render/Highlevel/Vegetation/VegetationCustomGeometrySerializationData.h"

namespace DAVA
{

class VegetationCustomSLGeometry : public VegetationGeometry
{
    
public:
    
    VegetationCustomSLGeometry(const Vector<uint32>& _maxClusters,
                             uint32 _maxDensityLevels,
                             const Vector2& _unitSize,
                             const FilePath& _dataPath,
                             const uint32* _resolutionCellSquare,
                             uint32 _resolutionCellSquareCount,
                             const float32* _resolutionScale,
                             uint32 _resolutionScaleCount,
                             const uint32* _resolutionTilesPerRow,
                             uint32 _resolutionTilesPerRowCount,
                             const uint32* _resolutionClusterStride,
                             uint32 _resolutionClusterStrideCount,
                             const Vector3& _worldSize,
                             VegetationCustomGeometrySerializationData* geometryData);
    virtual ~VegetationCustomSLGeometry();
    
    virtual void Build(Vector<VegetationRenderData*>& renderDataArray, const FastNameSet& materialFlags);
    virtual void OnVegetationPropertiesChanged(Vector<VegetationRenderData*>& renderDataArray, KeyedArchive* props);
    
private:
    
    class CustomMaterialTransformer : public VegetationMaterialTransformer
    {
    public:
        
        virtual void TransformMaterialOnCreate(NMaterial* mat);
    };
    
    struct ClusterPositionData
    {
        Vector3 pos;
        uint32 densityId;
        float32 rotation;
        uint32 layerId;
    };
    
    struct ClusterResolutionData
    {
        ClusterPositionData position;

        uint32 resolutionId;
        uint32 effectiveResolutionId;
        uint32 cellIndex;
    };
    
    struct CustomGeometryLayerData
    {
        Vector<Vector3> sourcePositions;
        Vector<Vector2> sourceTextureCoords;
        Vector<Vector3> sourceNormals;
        Vector<VegetationIndex> sourceIndices;
        
        CustomGeometryLayerData();
        CustomGeometryLayerData(const CustomGeometryLayerData& src);
    };
    
    struct CustomGeometryEntityData
    {
        Vector<CustomGeometryLayerData> lods;
        NMaterial* material;
        
        CustomGeometryEntityData();
        CustomGeometryEntityData(const CustomGeometryEntityData& src);
        ~CustomGeometryEntityData();
        
        void SetMaterial(NMaterial* mat);
    };
    
    struct SortBufferData
    {
        uint32 indexOffset;
        uint32 size;
        Vector3 sortDirection;
    };
    
    struct VertexRangeData
    {
        uint32 index;
        uint32 size;
        uint32 rowSize;
    };
    
    struct BufferCellData
    {
        uint32 vertexStartIndex;
        uint32 vertexCount;
        uint32 clusterStartIndex;
        uint32 clusterCount;
    };
    
private:
    
    void GenerateClusterPositionData(const Vector<uint32>& layerClusterCount,
                                     Vector<ClusterPositionData>& clusters,
                                     Vector<VertexRangeData>& layerRanges);
    
    void GenerateClusterResolutionData(uint32 resolutionId,
                                       const Vector<uint32>& layerClusterCount,
                                       const Vector<ClusterPositionData>& clusterPosition,
                                       const Vector<VertexRangeData>& layerRanges,
                                       Vector<ClusterResolutionData>& clusterResolution);
    
    void GenerateVertexData(const Vector<CustomGeometryEntityData>& sourceGeomData,
                            const Vector<ClusterResolutionData>& clusterResolution,
                            
                            Vector<VegetationVertex>& vertexData,
                            Vector<BufferCellData>& cellOffsets);
    
    void GenerateIndexData(const Vector<CustomGeometryEntityData>& sourceGeomData,
                           const Vector<ClusterResolutionData>& clusterResolution,
                           const BufferCellData& rangeData,
                           
                           Vector<VegetationVertex>& vertexData,
                           Vector<VegetationIndex>& indexData,
                           Vector<SortBufferData>& directionOffsets);
    
    void GenerateSortedClusterIndexData(Vector3& cameraPosition,
                                        Vector<PolygonSortData>& sourceIndices,
                                        Vector<VegetationVertex>& vertexData);
    
    static bool PolygonByDistanceCompareFunction(const PolygonSortData& a, const PolygonSortData&  b);
    static bool ClusterByMatrixCompareFunction(const ClusterResolutionData& a, const ClusterResolutionData&  b);
    static int32 RandomShuffleFunc(int32 limit);
    
    void Rotate(float32 angle,
                const Vector<Vector3>& sourcePositions,
                const Vector<Vector3>& sourceNormals,
                Vector<Vector3>& rotatedPositions,
                Vector<Vector3>& rotatedNormals);
    
    uint32 PrepareResolutionId(uint32 currentResolutionId, uint32 cellX, uint32 cellY) const;
    void InitCustomGeometry(VegetationCustomGeometrySerializationData* geometryData);
    
private:
    
    Vector<uint32> maxClusters;
    uint32 maxDensityLevels;
    Vector2 unitSize;
    FilePath sourceDataPath;
    Vector<uint32> resolutionCellSquare;
    Vector<float32> resolutionScale;
    Vector<uint32> resolutionTilesPerRow;
    Vector<uint32> resolutionClusterStride;
    Vector3 worldSize;
    uint32 resolutionCount;
    
    Vector<CustomGeometryEntityData> customGeometryData;
};

};

#endif /* defined(__DAVAENGINE_VEGETATIONCUSTOMSLGEOMETRYGENERATOR_H__) */
