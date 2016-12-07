#include "Classes/Project/ProjectManagerData.h"

#include "Classes/Deprecated/EditorConfig.h"
#include "SpritesPacker/SpritesPackerModule.h"

namespace ProjectManagerDataDetails
{
const char* DATA_PATH = "Data/";
const char* DATA_SOURCE_PATH = "DataSource/3d/";
const char* PARTICLE_CONFIG_PATH = "Data/Configs/Particles/";
const char* PARTICLE_DATA_PATH = "Data/Gfx/Particles/";
const char* WORKSPACE_PATH = "~doc:/ResourceEditor/";
}

const DAVA::String ProjectManagerData::ProjectPathProperty = DAVA::String("ProjectPath");

ProjectManagerData::ProjectManagerData()
{
}

ProjectManagerData::~ProjectManagerData() = default;

bool ProjectManagerData::IsOpened() const
{
    return (!projectPath.IsEmpty());
}

const DAVA::FilePath& ProjectManagerData::GetProjectPath() const
{
    return projectPath;
}

DAVA::FilePath ProjectManagerData::GetDataPath() const
{
    return projectPath + ProjectManagerDataDetails::DATA_PATH;
}

DAVA::FilePath ProjectManagerData::GetDataSourcePath() const
{
    return projectPath + ProjectManagerDataDetails::DATA_SOURCE_PATH;
}

DAVA::FilePath ProjectManagerData::GetParticlesConfigPath() const
{
    return projectPath + ProjectManagerDataDetails::PARTICLE_CONFIG_PATH;
}

DAVA::FilePath ProjectManagerData::GetParticlesDataPath() const
{
    return projectPath + ProjectManagerDataDetails::PARTICLE_DATA_PATH;
}

DAVA::FilePath ProjectManagerData::GetWorkspacePath() const
{
    return DAVA::FilePath(ProjectManagerDataDetails::WORKSPACE_PATH + projectPath.GetLastDirectoryName() + "/");
}

const QVector<ProjectManagerData::AvailableMaterialTemplate>& ProjectManagerData::GetAvailableMaterialTemplates() const
{
    return templates;
}

const QVector<ProjectManagerData::AvailableMaterialQuality>& ProjectManagerData::GetAvailableMaterialQualities() const
{
    return qualities;
}

DAVA::FilePath ProjectManagerData::CreateProjectPathFromPath(const DAVA::FilePath& pathname)
{
    DAVA::String fullPath = pathname.GetAbsolutePathname();
    DAVA::String::size_type pos = fullPath.find("/Data");
    if (pos != DAVA::String::npos)
    {
        return fullPath.substr(0, pos + 1);
    }

    return DAVA::FilePath();
}

const EditorConfig* ProjectManagerData::GetEditorConfig() const
{
    return editorConfig.get();
}

const SpritesPackerModule* ProjectManagerData::GetSpritesModules() const
{
    return spritesPacker.get();
}
