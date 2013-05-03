#include "DAVAEngine.h"
#include "GameCore.h"
#include "TexturePacker/CommandLineParser.h"
#include "TexturePacker/ResourcePacker2D.h"

#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/EditorConfig.h"
#include "SceneEditor/SceneValidator.h"

#include "CommandLine/CommandLineManager.h"
#include "CommandLine/SceneExporter/SceneExporter.h"

#include "PVRConverter.h"
#include "version.h"

using namespace DAVA;


/*
    Is it possible, to store size before vector, seems doable and cache friendly, because to tranverse vector firstly you 
    need to read size.
 */
template<class T>
class CacheFriendlyVector
{
public:
    uint8 * data;
};



class EntityX
{
//    Vector<Component*> fastAccessArray;
//    Vector<Component*> allComponents;
    Component ** fastAccessArray;
    Component ** allComponents;
    EntityX ** children;
};


class EntitySTL
{
    CacheFriendlyVector<Component*> fastAccessArray;
    CacheFriendlyVector<Component*> allComponents;
    CacheFriendlyVector<EntitySTL*> children;
};

/*
    => 24 bytes for each entity. (8 * 3 = 24 = 64bit system) (12bytes on 32 bit system)
    =>
    
    Action update transform: (тут не может не быть cache misses)
 
    Entity * entity = GetEntity(); // cache miss
    TransformComponent * tc = entity->fastTransforms[COMPONENT_TRANSFORM]; // cache miss
    tc->UpdateLocalTransform(transform); // cache miss
 
 
    TransformSystem:
    for each object go to his children and add them, for them do the same, linearly
 
 */


void CreateDefaultDescriptorsAtDataSource3D()
{
    FilePath dataSourcePathname = EditorSettings::Instance()->GetDataSourcePath();
    String sourceFolder = String("DataSource/3d/");
    
    if(sourceFolder.length() <= dataSourcePathname.GetAbsolutePathname().length())
    {
//        uint64 creationTime = SystemTimer::Instance()->AbsoluteMS();
        SceneValidator::Instance()->CreateDefaultDescriptors(dataSourcePathname);
//        creationTime = SystemTimer::Instance()->AbsoluteMS() - creationTime;
//      Logger::Info("[CreateDefaultDescriptors time is %ldms]", creationTime);
    }
}

void FrameworkDidLaunched()
{
    uint32 size = sizeof(EntityX);
    uint32 size2 = sizeof(EntitySTL);
    
    new CommandLineManager();
    new SceneExporter();
    new EditorSettings();
	new EditorConfig();
    new SceneValidator();

    
    SceneValidator::Instance()->SetPathForChecking(EditorSettings::Instance()->GetProjectPath());
    if(!CommandLineManager::Instance()->IsCommandLineModeEnabled())
    {
        CreateDefaultDescriptorsAtDataSource3D();
    }
    
#if defined(__DAVAENGINE_IPHONE__)
	KeyedArchive * appOptions = new KeyedArchive();
	appOptions->SetInt32("orientation", Core::SCREEN_ORIENTATION_LANDSCAPE_LEFT);
    
	DAVA::Core::Instance()->SetVirtualScreenSize(480, 320);
	DAVA::Core::Instance()->RegisterAvailableResourceSize(480, 320, "Gfx");
#else
	KeyedArchive * appOptions = new KeyedArchive();
    
    int32 width = (int32)DAVA::Core::Instance()->GetVirtualScreenWidth();
    int32 height = (int32)DAVA::Core::Instance()->GetVirtualScreenHeight();
    if(width <= 0 || height <= 0)
    {
        width = EditorSettings::Instance()->GetScreenWidth();
        height = EditorSettings::Instance()->GetScreenHeight();

        DAVA::Core::Instance()->SetVirtualScreenSize(width, height);
    }
    
	appOptions->SetString("title", Format("dava framework - resource editor | %s", RESOURCE_EDITOR_VERSION));
	appOptions->SetInt32("width",	width);
	appOptions->SetInt32("height", height);

	appOptions->SetInt32("fullscreen", 0);
	appOptions->SetInt32("bpp", 32); 

	DAVA::Core::Instance()->RegisterAvailableResourceSize(width, height, "Gfx");
#endif
    
	GameCore * core = new GameCore();
	DAVA::Core::SetApplicationCore(core);
	DAVA::Core::Instance()->SetOptions(appOptions);
    DAVA::Core::Instance()->EnableReloadResourceOnResize(false);

	SafeRelease(appOptions);
}


void FrameworkWillTerminate()
{
	SceneValidator::Instance()->Release();
	EditorConfig::Instance()->Release();
	EditorSettings::Instance()->Release();

	CommandLineManager::Instance()->Release();
}
