#include "CommandLine/BeastCommandLineTool.h"

#if defined(__DAVAENGINE_BEAST__)

#include "Base/ScopedPtr.h"
#include "Logger/Logger.h"

#include "Scene/SceneEditor2.h"
#include "Beast/BeastRunner.h"

#include "CommandLine/Private/OptionName.h"
#include "CommandLine/Private/SceneConsoleHelper.h"
#include "Utils/SceneUtils/SceneUtils.h"

BeastCommandLineTool::BeastCommandLineTool(const DAVA::Vector<DAVA::String>& commandLine)
    : REConsoleModuleCommon(commandLine, "-beast")
{
    options.AddOption(OptionName::File, DAVA::VariantType(DAVA::String("")), "Full pathname of scene for beasting");
    options.AddOption(OptionName::Output, DAVA::VariantType(DAVA::String("")), "Full path for output folder for beasting");
    options.AddOption(OptionName::QualityConfig, DAVA::VariantType(DAVA::String("")), "Full path for quality.yaml file");
}

bool BeastCommandLineTool::PostInitInternal()
{
    scenePathname = options.GetOption(OptionName::File).AsString();
    if (scenePathname.IsEmpty() || !scenePathname.IsEqualToExtension(".sc2"))
    {
        DAVA::Logger::Error("Scene was not selected");
        return false;
    }

    outputPathname = options.GetOption(OptionName::Output).AsString();
    if (outputPathname.IsEmpty())
    {
        DAVA::Logger::Error("Out folder was not selected");
        return false;
    }
    else
    {
        outputPathname.MakeDirectoryPathname();
    }

    bool qualityInitialized = SceneConsoleHelper::InitializeQualitySystem(options, scenePathname);
    if (!qualityInitialized)
    {
        DAVA::Logger::Error("Cannot create path to quality.yaml from %s", scenePathname.GetAbsolutePathname().c_str());
        return false;
    }

    return true;
}

DAVA::TArc::ConsoleModule::eFrameResult BeastCommandLineTool::OnFrameInternal()
{
    DAVA::ScopedPtr<SceneEditor2> scene(new SceneEditor2());

    if (scene->LoadScene(scenePathname) == DAVA::SceneFileV2::eError::ERROR_NO_ERROR)
    {
        scene->Update(0.1f);

        BeastRunner beast(scene, outputPathname, BeastProxy::MODE_LIGHTMAPS, nullptr);
        beast.Run();

        scene->SaveScene();
    }

    return DAVA::TArc::ConsoleModule::eFrameResult::FINISHED;
}

void BeastCommandLineTool::BeforeDestroyedInternal()
{
    SceneConsoleHelper::FlushRHI();
}

void BeastCommandLineTool::ShowHelpInternal()
{
    REConsoleModuleCommon::ShowHelpInternal();

    DAVA::Logger::Info("Examples:");
    DAVA::Logger::Info("\t-beast -file /Users/SmokeTest/DataSource/3d/Maps/scene.sc2 -output /Users/SmokeTest/DataSource/3d/Maps/beast");
}


#endif //#if defined (__DAVAENGINE_BEAST__)
