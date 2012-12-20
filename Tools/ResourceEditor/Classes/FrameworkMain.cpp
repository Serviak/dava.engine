//
//  main.m
//  TemplateProjectMacOS
//
//  Created by Vitaliy  Borodovsky on 3/16/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#include "DAVAEngine.h"
#include "GameCore.h"
#include "ResourcePackerScreen.h"
#include "TexturePacker/CommandLineParser.h"

#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/EditorConfig.h"
#include "SceneEditor/SceneValidator.h"

#include "SceneEditor/CommandLineTool.h"
#include "SceneEditor/SceneExporter.h"

#include "SceneEditor/PVRConverter.h"
#include "version.h"

using namespace DAVA;

 
void PrintUsage()
{
    printf("Usage:\n");

    printf("\t-usage or --help to display this help\n");
    printf("\t-exo - extended output\n"); 
    printf("\t-v or --verbose - detailed output\n");

    printf("\n");
    printf("resourcepacker [src_dir] - will pack resources from src_dir\n");
    
    printf("\n");
    printf("-sceneexporter [-clean [directory]] [-export [-indir [directory]] [-outdir [directory]] [-processdir [directory]] [-processfile [directory]] [-format]\n");
    printf("\t-clean - will delete all files from Data/3d/\n"); 
    printf("\t-export - will export level to Data/3d/\n"); 
    printf("\t-indir - path for Poject/DataSource/3d/ folder \n"); 
    printf("\t-outdir - path for Poject/Data/3d/ folder\n"); 
    printf("\t-processdir - foldername from DataSource/3d/ for exporting\n"); 
    printf("\t-processfile - filename from DataSource/3d/ for exporting\n"); 
    printf("\t-format - png, pvr, dxt\n"); 
    printf("\t-force - to don't display error dialogs");
    
    printf("\n");
    printf("Samples:");
    printf("-sceneexporter -clean /Users/User/Project/Data/3d/\n");
    printf("-sceneexporter -export -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processdir Maps/objects/\n");
    printf("-sceneexporter -export -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processfile Maps/level.sc2 -force\n");
}


bool CheckPosition(int32 commandPosition)
{
    if(CommandLineTool::Instance()->CheckPosition(commandPosition))
    {
        printf("Wrong arguments\n");
        PrintUsage();

        return false;
    }
    
    return true;
}

void ProcessRecourcePacker()
{
    Vector<String> & commandLine = Core::Instance()->GetCommandLine();
    
    ResourcePackerScreen * resourcePackerScreen = new ResourcePackerScreen();
    
    String path, lastDir;
    FileSystem::SplitPath(commandLine[1], path, lastDir);
    
    resourcePackerScreen->inputGfxDirectory = FileSystem::RealPath(commandLine[1]);// "/./../../ProjectDataSource/Gfx/");
    resourcePackerScreen->outputGfxDirectory = FileSystem::RealPath(resourcePackerScreen->inputGfxDirectory + "/../../Data/" + lastDir);
    resourcePackerScreen->excludeDirectory = FileSystem::RealPath(resourcePackerScreen->inputGfxDirectory + "/../");
    
    
    String excludeDirPath, excludeDirLastDir;
    FileSystem::SplitPath(resourcePackerScreen->excludeDirectory, excludeDirPath, excludeDirLastDir);
    if (excludeDirLastDir != "DataSource")
    {
        printf("[FATAL ERROR: Packer working only inside DataSource directory]");
        return;
    }
    
    new PVRConverter();

    
    if(commandLine.size() < 3)
    {
        printf("[FATAL ERROR: PVRTexTool path need to be second parameter]");
        return;
    }

#if defined (__DAVAENGINE_MACOS__)
	String toolName = String("/PVRTexToolCL");
#elif defined (__DAVAENGINE_WIN32__)
	String toolName = String("/PVRTexToolCL.exe");
#endif
    PVRConverter::Instance()->SetPVRTexTool(resourcePackerScreen->excludeDirectory + String("/") + commandLine[2] + toolName);

    uint64 elapsedTime = SystemTimer::Instance()->AbsoluteMS();
    printf("[Resource Packer Started]\n");
    printf("[INPUT DIR] - [%s]\n", resourcePackerScreen->inputGfxDirectory.c_str());
    printf("[OUTPUT DIR] - [%s]\n", resourcePackerScreen->outputGfxDirectory.c_str());
    printf("[EXCLUDE DIR] - [%s]\n", resourcePackerScreen->excludeDirectory.c_str());
    
    Texture::InitializePixelFormatDescriptors();
    resourcePackerScreen->PackResources();
    elapsedTime = SystemTimer::Instance()->AbsoluteMS() - elapsedTime;
    printf("[Resource Packer Compile Time: %0.3lf seconds]\n", (float64)elapsedTime / 1000.0);
    
    PVRConverter::Instance()->Release();
    
    SafeRelease(resourcePackerScreen);
}


void FrameworkDidLaunched()
{
//	EntityTest();

    new CommandLineTool();
    new SceneExporter();
    new EditorSettings();
	new EditorConfig();
    new SceneValidator();

    SceneValidator::Instance()->SetPathForChecking(EditorSettings::Instance()->GetProjectPath());
    
    String dataSourcePathname = EditorSettings::Instance()->GetDataSourcePath();
    String sourceFolder = String("DataSource/3d");
    
    if(!CommandLineTool::Instance()->CommandIsFound(String("-imagesplitter")))
    {
        if(sourceFolder.length() <= dataSourcePathname.length())
        {
            uint64 creationTime = SystemTimer::Instance()->AbsoluteMS();
            SceneValidator::Instance()->CreateDefaultDescriptors(dataSourcePathname);
            creationTime = SystemTimer::Instance()->AbsoluteMS() - creationTime;
            Logger::Info("[CreateDefaultDescriptors time is %ldms]", creationTime);
        }
    }
    
    
	if (Core::Instance()->IsConsoleMode())
	{
        if(     CommandLineTool::Instance()->CommandsCount() < 2 
           ||   (CommandLineTool::Instance()->CommandIsFound(String("-usage")))
           ||   (CommandLineTool::Instance()->CommandIsFound(String("-help")))
           )
        {
            PrintUsage();
			return;
        }
		
        if(CommandLineTool::Instance()->CommandIsFound(String("-v")) || CommandLineTool::Instance()->CommandIsFound(String("-verbose")))
        {
            CommandLineParser::Instance()->SetVerbose(true);
        }
        
        if(CommandLineTool::Instance()->CommandIsFound(String("-exo")))
        {
            CommandLineParser::Instance()->SetExtendedOutput(true);
        }
		
        if(!CommandLineTool::Instance()->CommandIsFound(String("-sceneexporter")) && !CommandLineTool::Instance()->CommandIsFound(String("-imagesplitter")))
        {
            ProcessRecourcePacker();
            return;  
        }
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
    SceneExporter::Instance()->Release();
	CommandLineTool::Instance()->Release();
}
