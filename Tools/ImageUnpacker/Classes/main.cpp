#include "Engine/Public/Engine.h"
#include "CommandLine/CommandLineParser.h"
#include "Render/Image/Image.h"
#include "Render/RenderBase.h"
#include "Render/Image/ImageSystem.h"
#include "Render/Image/ImageConvert.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/Texture.h"

#include "Debug/DVAssert.h"
#include "FileSystem/FileList.h"
#include "Base/BaseTypes.h"

using namespace DAVA;

void PrintUsage()
{
    printf("Usage:\n");

    printf("\t-usage or --help to display this help\n");
    printf("\t-file - pvr or dds file to unpack as png\n");
    printf("\t-folder - folder with pvr or dds files to unpack as png\n");
    printf("\t-saveas -ext -folder - will open png files from folder and save as ext parameter mean\n");

    printf("Example:\n");
    printf("\t-saveas -ext .tga -folder /Users/nickname/test/");
}

void SaveSingleImage(const FilePath& newImagePath, Image* image)
{
    if ((FORMAT_RGBA8888 == image->format) || (FORMAT_A8 == image->format) || (FORMAT_A16 == image->format))
    {
        ImageSystem::Save(newImagePath, image, image->format);
    }
    else
    {
        Image* savedImage = Image::Create(image->width, image->height, FORMAT_RGBA8888);

        ImageConvert::ConvertImageDirect(image->format, savedImage->format, image->data, image->width, image->height, image->width * PixelFormatDescriptor::GetPixelFormatSizeInBytes(image->format),
                                         savedImage->data, savedImage->width, savedImage->height, savedImage->width * PixelFormatDescriptor::GetPixelFormatSizeInBytes(savedImage->format));

        ImageSystem::Save(newImagePath, savedImage);
        savedImage->Release();
    }
}

void GenerateFacePathnames(const FilePath& filePath, Vector<FilePath>& faceNames, const String& extension)
{
    faceNames.resize(Texture::CUBE_FACE_COUNT, FilePath());

    String baseName = filePath.GetBasename();
    for (auto face = 0; face < Texture::CUBE_FACE_COUNT; ++face)
    {
        faceNames[face] = filePath;
        faceNames[face].ReplaceFilename(baseName + Texture::FACE_NAME_SUFFIX[face] + extension);
    }
}

void SaveCubemap(const FilePath& newImagePath, const Vector<Image*>& images)
{
    Vector<FilePath> faceNames;
    GenerateFacePathnames(newImagePath, faceNames, ".png");

    for (auto image : images)
    {
        if (0 == image->mipmapLevel)
        {
            SaveSingleImage(faceNames[image->cubeFaceID], image);
        }
    }
}

void UnpackFile(const FilePath& sourceImagePath)
{
    Vector<Image*> images;
    ImageSystem::Load(sourceImagePath, images);

    if (images.size() != 0)
    {
        FilePath imagePathname = FilePath::CreateWithNewExtension(sourceImagePath, ".png");

        Image* image = images[0];

        if (image->cubeFaceID == Texture::INVALID_CUBEMAP_FACE)
        {
            SaveSingleImage(imagePathname, image);
        }
        else
        {
            SaveCubemap(imagePathname, images);
        }

        for_each(images.begin(), images.end(), SafeRelease<Image>);
    }
    else
    {
        Logger::Error("Cannot load file: ", sourceImagePath.GetStringValue().c_str());
    }
}

void UnpackFolder(const FilePath& folderPath)
{
    ScopedPtr<FileList> fileList(new FileList(folderPath));
    for (int fi = 0; fi < fileList->GetCount(); ++fi)
    {
        const FilePath& pathname = fileList->GetPathname(fi);
        if (fileList->IsDirectory(fi) && !fileList->IsNavigationDirectory(fi))
        {
            UnpackFolder(pathname);
        }
        else
        {
            if (pathname.IsEqualToExtension(".pvr") || pathname.IsEqualToExtension(".dds"))
            {
                UnpackFile(pathname);
            }
        }
    }
}

void ResavePNG(const FilePath& folderPath, const String& extension)
{
    ScopedPtr<FileList> fileList(new FileList(folderPath));

    for (int fi = 0; fi < fileList->GetCount(); ++fi)
    {
        const FilePath& pathname = fileList->GetPathname(fi);
        if (fileList->IsDirectory(fi) && !fileList->IsNavigationDirectory(fi))
        {
            ResavePNG(pathname, extension);
        }
        else
        {
            if (pathname.IsEqualToExtension(".png"))
            {
                Vector<Image*> images;
                ImageSystem::Load(pathname, images);

                FilePath tgaPathname = FilePath::CreateWithNewExtension(pathname, extension);
                ImageSystem::Save(tgaPathname, images);

                for_each(images.begin(), images.end(), SafeRelease<Image>);
            }
        }
    }
}

void ProcessImageUnpacker()
{
#if RHI_COMPLETE
    RenderManager::Create(Core::RENDERER_OPENGL);
#endif //#if RHI_COMPLETE

    FilePath sourceFolderPath = CommandLineParser::GetCommandParam(String("-folder"));
    FilePath sourceFilePath = CommandLineParser::GetCommandParam(String("-file"));

    bool needShowUsage = true;
    if (CommandLineParser::CommandIsFound("-saveas") && sourceFolderPath.IsEmpty() == false)
    {
        String ext = CommandLineParser::GetCommandParam(String("-ext"));
        if (!ext.empty())
        {
            if (ext[0] != '.')
            {
                ext = "." + ext;
            }

            sourceFolderPath.MakeDirectoryPathname();
            ResavePNG(sourceFolderPath, ext);

            needShowUsage = false;
        }
    }
    else if (sourceFolderPath.IsEmpty() == false)
    {
        sourceFolderPath.MakeDirectoryPathname();
        UnpackFolder(sourceFolderPath);
        needShowUsage = false;
    }
    else if (sourceFilePath.IsEmpty() == false)
    {
        UnpackFile(sourceFilePath);
        needShowUsage = false;
    }

    if (needShowUsage)
    {
        PrintUsage();
    }

#if RHI_COMPLETE
    RenderManager::Instance()->Release();
#endif //#if RHI_COMPLETE
}

void Process(Engine& e)
{
    e.GetContext()->logger->SetLogLevel(Logger::LEVEL_INFO);
    DVASSERT(e.IsConsoleMode() == true);

    if (CommandLineParser::GetCommandsCount() < 2
        || (CommandLineParser::CommandIsFound(String("-usage")))
        || (CommandLineParser::CommandIsFound(String("-help")))
        )
    {
        PrintUsage();
        return;
    }

    ProcessImageUnpacker();
}

int GameMain(Vector<String> cmdline)
{
    Engine e;
    e.Init(eEngineRunMode::CONSOLE_MODE, {});

    e.update.Connect([&e](float32)
                     {
                         Process(e);
                         e.Quit(0);
                     });

    return e.Run();
}
