#include "SceneValidator.h"
#include "ErrorNotifier.h"
#include "EditorSettings.h"
#include "SceneInfoControl.h"

#include "PVRUtils.h"

SceneValidator::SceneValidator()
{
    sceneTextureCount = 0;
    sceneTextureMemory = 0;

    infoControl = NULL;
    
    pathForChecking = String("");
}

SceneValidator::~SceneValidator()
{
    SafeRelease(infoControl);
}

void SceneValidator::ValidateScene(Scene *scene)
{
    errorMessages.clear();

    ValidateScene(scene, errorMessages);

    ShowErrors();
}

void SceneValidator::ValidateScene(Scene *scene, Set<String> &errorsLog)
{
    if(scene) 
    {
        ValidateSceneNode(scene, errorsLog);
        ValidateLodNodes(scene, errorsLog);
        
        for (Set<SceneNode*>::iterator it = emptyNodesForDeletion.begin(); it != emptyNodesForDeletion.end(); ++it)
        {
            SceneNode * node = *it;
            if (node->GetParent())
            {
                node->GetParent()->RemoveNode(node);
            }
        }
        for (Set<SceneNode*>::iterator it = emptyNodesForDeletion.begin(); it != emptyNodesForDeletion.end(); ++it)
        {
            SceneNode * node = *it;
            SafeRelease(node);
        }
        emptyNodesForDeletion.clear();
    }
    else 
    {
        errorsLog.insert(String("Scene in NULL!"));
    }
}

void SceneValidator::ValidateScales(Scene *scene, Set<String> &errorsLog)
{
	if(scene) 
	{
		ValidateScalesInternal(scene, errorsLog);
	}
	else 
	{
		errorsLog.insert(String("Scene in NULL!"));
	}
}

void SceneValidator::ValidateScalesInternal(SceneNode *sceneNode, Set<String> &errorsLog)
{
//  Basic algorithm is here
// 	Matrix4 S, T, R; //Scale Transpose Rotation
// 	S.CreateScale(Vector3(1.5, 0.5, 2.0));
// 	T.CreateTranslation(Vector3(100, 50, 20));
// 	R.CreateRotation(Vector3(0, 1, 0), 2.0);
// 
// 	Matrix4 t = R*S*T; //Calculate complex matrix
// 
//	//Calculate Scale components from complex matrix
// 	float32 sx = sqrt(t._00 * t._00 + t._10 * t._10 + t._20 * t._20);
// 	float32 sy = sqrt(t._01 * t._01 + t._11 * t._11 + t._21 * t._21);
// 	float32 sz = sqrt(t._02 * t._02 + t._12 * t._12 + t._22 * t._22);
// 	Vector3 sCalculated(sx, sy, sz);

	if(!sceneNode) return;

	const Matrix4 & t = sceneNode->GetLocalTransform();
	float32 sx = sqrt(t._00 * t._00 + t._10 * t._10 + t._20 * t._20);
	float32 sy = sqrt(t._01 * t._01 + t._11 * t._11 + t._21 * t._21);
	float32 sz = sqrt(t._02 * t._02 + t._12 * t._12 + t._22 * t._22);

	if ((!FLOAT_EQUAL(sx, 1.0f)) 
		|| (!FLOAT_EQUAL(sy, 1.0f))
		|| (!FLOAT_EQUAL(sz, 1.0f)))
	{
 		errorsLog.insert(Format("Node %s: has scale (%.3f, %.3f, %.3f) ! Re-design level.", sceneNode->GetName().c_str(), sx, sy, sz));
	}

	int32 count = sceneNode->GetChildrenCount();
	for(int32 i = 0; i < count; ++i)
	{
		ValidateScalesInternal(sceneNode->GetChild(i), errorsLog);
	}
}



void SceneValidator::ValidateSceneNode(SceneNode *sceneNode)
{
    errorMessages.clear();

    ValidateSceneNode(sceneNode, errorMessages);
    
    ShowErrors();
    
}

void SceneValidator::ValidateSceneNode(SceneNode *sceneNode, Set<String> &errorsLog)
{
    if(!sceneNode) return;
    
    int32 count = sceneNode->GetChildrenCount();
    for(int32 i = 0; i < count; ++i)
    {
        SceneNode *node = sceneNode->GetChild(i);
        MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode*>(node);
        if(mesh)
        {
            ValidateMeshInstance(mesh, errorsLog);
        }
        else 
        {
            LandscapeNode *landscape = dynamic_cast<LandscapeNode*>(node);
            if (landscape) 
            {
                ValidateLandscape(landscape, errorsLog);
            }
            else
            {
                ValidateSceneNode(node, errorsLog);
            }
        }
        
        KeyedArchive *customProperties = node->GetCustomProperties();
        if(customProperties->IsKeyExists("editor.referenceToOwner"))
        {
            String dataSourcePath = EditorSettings::Instance()->GetDataSourcePath();
            if(1 < dataSourcePath.length())
            {
                if('/' == dataSourcePath[0])
                {
                    dataSourcePath = dataSourcePath.substr(1);
                }
                
                String referencePath = customProperties->GetString("editor.referenceToOwner");
                String::size_type pos = referencePath.rfind(dataSourcePath);
                if((String::npos != pos) && (1 != pos))
                {
                    referencePath.replace(pos, dataSourcePath.length(), "");
                    customProperties->SetString("editor.referenceToOwner", referencePath);
                    
                    errorsLog.insert(Format("Node %s: referenceToOwner isn't correct. Re-save level.", node->GetName().c_str()));
                }
            }
        }
    }
    
    if(typeid(SceneNode) == typeid(*sceneNode))
    {
        Set<DataNode*> dataNodeSet;
        sceneNode->GetDataNodes(dataNodeSet);
        if (dataNodeSet.size() == 0)
        {
            SceneNode * parent = sceneNode->GetParent();
            if (parent)
            {
                emptyNodesForDeletion.insert(SafeRetain(sceneNode));
            }
        }
    }
}

void SceneValidator::ValidateTexture(Texture *texture)
{
    errorMessages.clear();

    ValidateTexture(texture, errorMessages);

    ShowErrors();
}

void SceneValidator::ValidateTexture(Texture *texture, Set<String> &errorsLog)
{
    if(!texture) return;

    bool pathIsCorrect = ValidatePathname(texture->GetPathname());
    if(!pathIsCorrect)
    {
        String path = FileSystem::AbsoluteToRelativePath(EditorSettings::Instance()->GetDataSourcePath(), texture->GetPathname());
        errorsLog.insert("Wrong path of: " + path);
    }
    if(IsntPower2(texture->GetWidth()) || IsntPower2(texture->GetHeight()))
    {
        String path = FileSystem::AbsoluteToRelativePath(EditorSettings::Instance()->GetDataSourcePath(), texture->GetPathname());
        errorsLog.insert("Wrong size of " + path);
    }
}

void SceneValidator::ValidateLandscape(LandscapeNode *landscape)
{
    errorMessages.clear();
    
    ValidateLandscape(landscape, errorMessages);
    
    ShowErrors();
}

void SceneValidator::ValidateLandscape(LandscapeNode *landscape, Set<String> &errorsLog)
{
    if(!landscape) return;
    
    for(int32 i = 0; i < LandscapeNode::TEXTURE_COUNT; ++i)
    {
        ValidateTexture(landscape->GetTexture((LandscapeNode::eTextureLevel)i), errorsLog);
    }
    
    bool pathIsCorrect = ValidatePathname(landscape->GetHeightmapPathname());
    if(!pathIsCorrect)
    {
        String path = FileSystem::AbsoluteToRelativePath(EditorSettings::Instance()->GetDataSourcePath(), landscape->GetHeightmapPathname());
        errorsLog.insert("Wrong path of Heightmap: " + path);
    }
}

bool SceneValidator::IsntPower2(int32 num)
{
    return ((num & (num - 1)) > 0);
}

void SceneValidator::ShowErrors()
{
    if(0 < errorMessages.size())
    {
        ErrorNotifier::Instance()->ShowError(errorMessages);
    }
}

void SceneValidator::ValidateMeshInstance(MeshInstanceNode *meshNode, Set<String> &errorsLog)
{
    meshNode->RemoveFlag(SceneNode::NODE_INVALID);
    
    const Vector<PolygonGroupWithMaterial*> & polygroups = meshNode->GetPolygonGroups();
    //Vector<Material *>materials = meshNode->GetMaterials();
    for(int32 iMat = 0; iMat < (int32)polygroups.size(); ++iMat)
    {
        Material * material = polygroups[iMat]->GetMaterial();

        ValidateMaterial(material, errorsLog);

        if (material->Validate(polygroups[iMat]->GetPolygonGroup()) == Material::VALIDATE_INCOMPATIBLE)
        {
            meshNode->AddFlag(SceneNode::NODE_INVALID);
            errorsLog.insert(Format("Material: %s incompatible with node:%s.", material->GetName().c_str(), meshNode->GetFullName().c_str()));
            errorsLog.insert("For lightmapped objects check second coordinate set. For normalmapped check tangents, binormals.");
        }
    }
    
    int32 lightmapCont = meshNode->GetLightmapCount();
    for(int32 iLight = 0; iLight < lightmapCont; ++iLight)
    {
        ValidateTexture(meshNode->GetLightmapDataForIndex(iLight)->lightmap, errorsLog);
    }
}


void SceneValidator::ValidateMaterial(Material *material)
{
    errorMessages.clear();

    ValidateMaterial(material, errorMessages);

    ShowErrors();
}

void SceneValidator::ValidateMaterial(Material *material, Set<String> &errorsLog)
{
    for(int32 iTex = 0; iTex < Material::TEXTURE_COUNT; ++iTex)
    {
        ValidateTexture(material->GetTexture((Material::eTextureLevel)iTex), errorsLog);
        
        if(material->GetTextureName((Material::eTextureLevel)iTex).find(".pvr.png") != String::npos)
        {
            errorsLog.insert(material->GetName() + ": wrong texture name " + material->GetTextureName((Material::eTextureLevel)iTex));
        }
    }
}

void SceneValidator::EnumerateSceneTextures()
{
    sceneTextureCount = 0;
    sceneTextureMemory = 0;
    
    const Map<String, Texture*> textureMap = Texture::GetTextureMap();
    KeyedArchive *settings = EditorSettings::Instance()->GetSettings(); 
    String projectPath = settings->GetString("ProjectPath");
	for(Map<String, Texture *>::const_iterator it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		Texture *t = it->second;
        if(String::npos != t->relativePathname.find(projectPath))
        {
            String::size_type pvrpngPos = t->relativePathname.find(".pvr.png");
            if(String::npos != pvrpngPos)
            {
                String pvrPath = FileSystem::ReplaceExtension(t->relativePathname, "");
                sceneTextureMemory += PVRUtils::Instance()->GetPVRDataLength(pvrPath);
            }
            else 
            {
                sceneTextureMemory += t->GetDataSize();
            }
            
            ++sceneTextureCount;
        }
	}
    
    if(infoControl)
    {
        infoControl->InvalidateTexturesInfo(sceneTextureCount, sceneTextureMemory);
    }
}

void SceneValidator::SetInfoControl(SceneInfoControl *newInfoControl)
{
    SafeRelease(infoControl);
    infoControl = SafeRetain(newInfoControl);
    
    sceneStats.Clear();
}

void SceneValidator::CollectSceneStats(const RenderManager::Stats &newStats)
{
    sceneStats = newStats;
    infoControl->SetRenderStats(sceneStats);
}

void SceneValidator::ReloadTextures()
{
    bool isAlphaPremultiplicationEnabled = Image::IsAlphaPremultiplicationEnabled();
    bool isMipmapsEnabled = Texture::IsMipmapGenerationEnabled();

    const Map<String, Texture*> textureMap = Texture::GetTextureMap();
	for(Map<String, Texture *>::const_iterator it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		Texture *texture = it->second;
        
        Image::EnableAlphaPremultiplication(texture->isAlphaPremultiplied);
        
        if(texture->isMimMapTexture) Texture::EnableMipmapGeneration();
        else Texture::DisableMipmapGeneration();

        Image *image = Image::CreateFromFile(texture->relativePathname);
        if(image)
        {
            texture->TexImage(0, image->GetWidth(), image->GetHeight(), image->GetData());
            if(texture->isMimMapTexture)
            {
                texture->GenerateMipmaps();
            }
            texture->SetWrapMode(texture->wrapModeS, texture->wrapModeT);
                
            SafeRelease(image);
        }
	}
    
    if(isMipmapsEnabled) Texture::EnableMipmapGeneration();
    else Texture::DisableMipmapGeneration();
    
    Image::EnableAlphaPremultiplication(isAlphaPremultiplicationEnabled);
}

void SceneValidator::ValidateLodNodes(Scene *scene, Set<String> &errorsLog)
{
    Vector<LodNode *> lodnodes;
    scene->GetChildNodes(lodnodes); 
    
    for(int32 index = 0; index < (int32)lodnodes.size(); ++index)
    {
        LodNode *ln = lodnodes[index];
        
        int32 layersCount = ln->GetLodLayersCount();
        for(int32 layer = 0; layer < layersCount; ++layer)
        {
            float32 distance = ln->GetLodLayerDistance(layer);
            if(LodNode::INVALID_DISTANCE == distance)
            {
                ln->SetLodLayerDistance(layer, ln->GetDefaultDistance(layer));
                errorsLog.insert(Format("Node %s: lod distances weren't correct. Re-save.", ln->GetName().c_str()));
            }
        }
        
        List<LodNode::LodData *>lodLayers;
        ln->GetLodData(lodLayers);
        
        List<LodNode::LodData *>::const_iterator endIt = lodLayers.end();
        int32 layer = 0;
        for(List<LodNode::LodData *>::iterator it = lodLayers.begin(); it != endIt; ++it, ++layer)
        {
            LodNode::LodData * ld = *it;
            
            if(ld->layer != layer)
            {
                ld->layer = layer;

                errorsLog.insert(Format("Node %s: lod layers weren't correct. Rename childs. Re-save.", ln->GetName().c_str()));
            }
        }
    }
}

String SceneValidator::SetPathForChecking(const String &pathname)
{
    String oldPath = pathForChecking;
    pathForChecking = pathname;
    return pathForChecking;
}


#include "FuckingErrorDialog.h"
bool SceneValidator::ValidatePathname(const String &pathForValidation)
{
    DVASSERT(0 < pathForChecking.length()); 
    //Need to set path to DataSource/3d for path correction  
    //Use SetPathForChecking();
    
    String normalizedPath = FileSystem::NormalizePath(pathForValidation);
    
    String::size_type fboFound = normalizedPath.find(String("FBO"));
    String::size_type resFound = normalizedPath.find(String("~res:"));
    if((String::npos != fboFound) || (String::npos != resFound))
    {
        return true;   
    }
    
    
    String::size_type foundPos = normalizedPath.find(pathForChecking);
    bool pathIsCorrect = (String::npos != foundPos);
    if(!pathIsCorrect)
    {
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        
        FuckingErrorDialog *dlg = new FuckingErrorDialog(screen->GetRect(), String("Wrong path: ") + pathForValidation);
        screen->AddControl(dlg);
        SafeRelease(dlg);
    }
    
    return pathIsCorrect;
    
}

void SceneValidator::EnumerateNodes(DAVA::Scene *scene)
{
    int32 nodesCount = 0;
    if(scene)
    {
        for(int32 i = 0; i < scene->GetChildrenCount(); ++i)
        {
            nodesCount += EnumerateSceneNodes(scene->GetChild(i));
        }
    }
    
    if(infoControl)
        infoControl->SetNodesCount(nodesCount);
}

int32 SceneValidator::EnumerateSceneNodes(DAVA::SceneNode *node)
{
    //TODO: lode node can have several nodes at layer
    
    int32 nodesCount = 1;
    for(int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        nodesCount += EnumerateSceneNodes(node->GetChild(i));
    }
    
    return nodesCount;
}
