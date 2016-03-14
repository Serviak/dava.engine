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
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include <iostream>

#include <FileSystem/FileSystem.h>
#include <FileSystem/ResourceArchive.h>
#include <FileSystem/FileList.h>
#include <CommandLine/ProgramOptions.h>

using namespace DAVA;

void FrameworkDidLaunched()
{
}

void FrameworkWillTerminate()
{
}

void CollectAllFilesInDirectory(const String& pathDirName,
                                bool includeHidden,
                                Vector<String>& output)
{
    FilePath pathToDir = pathDirName;
    RefPtr<FileList> fileList(new FileList(pathToDir, includeHidden));
    for (auto file = 0; file < fileList->GetCount(); ++file)
    {
        if (fileList->IsDirectory(file))
        {
            auto directoryName = fileList->GetFilename(file);
            if ((directoryName != "..") && (directoryName != "."))
            {
                String subDir = pathDirName == "."
                ?
                directoryName + '/'
                :
                pathDirName + directoryName + '/';
                CollectAllFilesInDirectory(subDir, includeHidden, output);
            }
        }
        else
        {
            String filename = fileList->GetFilename(file);
            if (filename.at(0) == '.' && !includeHidden)
            {
                continue;
            }
            String pathname =
            (pathDirName == "." ? filename : pathDirName + filename);
            output.push_back(pathname);
        }
    }
}

ResourceArchive::CompressionType ToPackType(
const String& value,
ResourceArchive::CompressionType defaultVal)
{
    const Vector<std::pair<String, ResourceArchive::CompressionType>>
    packTypes = { { "lz4", ResourceArchive::CompressionType::Lz4 },
                  { "lz4hc", ResourceArchive::CompressionType::Lz4HC },
                  { "none", ResourceArchive::CompressionType::None } };
    for (auto& pair : packTypes)
    {
        if (pair.first == value)
        {
            return pair.second;
        }
    }
    std::cerr << "can't convert: \"" << value
              << "\" into any valid compression type, use default\n";
    return defaultVal;
}

String ToString(ResourceArchive::CompressionType packType)
{
    static const Vector<const char*> packTypes = { "none", "lz4", "lz4hc" };

    size_t index = static_cast<size_t>(packType);

    return packTypes.at(index);
}

void OnOneFilePacked(const ResourceArchive::FileInfo& info)
{
    std::cout << "packing file: " << info.name
              << " compressed: " << info.compressedSize
              << " original: " << info.originalSize
              << " packingType: " << ToString(info.compressionType) << '\n';
}

int PackDirectory(const String& dir,
                  const String& pakfileName,
                  const ResourceArchive::Rules& compressionRules,
                  bool includeHidden)
{
    std::cout << "===================================================\n"
              << "=== Packer started\n"
              << "=== Pack directory: " << dir << '\n'
              << "=== Pack archiveName: " << pakfileName << '\n'
              << "===================================================\n";

    auto dirWithSlash = (dir.back() == '/' ? dir : dir + '/');

    RefPtr<FileSystem> fileSystem(new FileSystem());

    FilePath absPathPack =
    fileSystem->GetCurrentWorkingDirectory() + pakfileName;

    if (!fileSystem->SetCurrentWorkingDirectory(dirWithSlash))
    {
        std::cerr << "can't set CWD to: " << dirWithSlash << '\n';
        return EXIT_FAILURE;
    }

    Vector<String> files;

    CollectAllFilesInDirectory(".", includeHidden, files);

    if (files.empty())
    {
        std::cerr << "no files found in: " << dir << '\n';
        return EXIT_FAILURE;
    }

    if (fileSystem->IsFile(absPathPack))
    {
        std::cerr << "pakfile already exist! Delete it.\n";
        if (0 != std::remove(absPathPack.GetAbsolutePathname().c_str()))
        {
            std::cerr << "can't remove existing pakfile.\n";
            return EXIT_FAILURE;
        }
    }

    std::stable_sort(begin(files), end(files));

    std::cout << "start packing...\n";
    if (ResourceArchive::CreatePack(absPathPack.GetAbsolutePathname(), files,
                                    compressionRules, OnOneFilePacked))
    {
        std::cout << "Success!\n";
        return EXIT_SUCCESS;
    }
    else
    {
        std::cout << "Failed!\n";
        return EXIT_FAILURE;
    }
}

bool ParsePackRules(const ProgramOptions& packOptions,
                    const ResourceArchive::CompressionType defaultPackTypa,
                    ResourceArchive::Rules& outputRules)
{
    for (uint32 i = 0; i < packOptions.GetOptionValuesCount("--rule"); ++i)
    {
        VariantType option = packOptions.GetOption("--rule", i);
        String str = option.AsString();
        size_t dotPos = str.find('.');
        if (dotPos == String::npos)
        {
            std::cerr << "incorrect option: " << str << '\n';
            return false;
        }
        String ext = str.substr(0, dotPos);
        String compressionType = str.substr(dotPos + 1);
        ResourceArchive::CompressionType packType =
        ToPackType(compressionType, defaultPackTypa);
        outputRules.push_back({ ext, packType });
    }
    return true;
}

bool UnpackFile(const ResourceArchive& ra,
                const ResourceArchive::FileInfo& fileInfo)
{
    ResourceArchive::ContentAndSize file;
    if (!ra.LoadFile(fileInfo.name, file))
    {
        std::cerr << "can't load file: " << fileInfo.name << " from archive\n";
        return false;
    }
    String filePath(fileInfo.name);
    FilePath fullPath = filePath;
    FilePath dirPath = fullPath.GetDirectory();
    FileSystem::eCreateDirectoryResult result =
    FileSystem::Instance()->CreateDirectory(dirPath, true);
    if (FileSystem::DIRECTORY_CANT_CREATE == result)
    {
        std::cerr << "can't create unpack path dir: "
                  << dirPath.GetAbsolutePathname() << '\n';
        return false;
    }
    RefPtr<File> f(File::Create(fullPath, File::CREATE | File::WRITE));
    if (!f)
    {
        std::cerr << "can't create file: " << fileInfo.name << '\n';
        return false;
    }
    uint32 writeOk = f->Write(file.content.get(), file.size);
    if (writeOk < file.size)
    {
        std::cerr << "can't write into file: " << fileInfo.name << '\n';
        return false;
    }
    return true;
}

int UnpackToDirectory(const String& pak, const String& dir)
{
    RefPtr<FileSystem> fs(new FileSystem());
    if (!fs)
    {
        std::cerr << "can't create FileSystem\n";
        return EXIT_FAILURE;
    }
    String cwd = fs->GetCurrentWorkingDirectory().GetAbsolutePathname();

    std::cout << "===================================================\n"
              << "=== Unpacker started\n"
              << "=== Unpack directory: " << dir << '\n'
              << "=== Unpack archiveName: " << pak << '\n'
              << "===================================================\n";

    fs->CreateDirectory(dir);
    if (!fs->SetCurrentWorkingDirectory(dir + '/'))
    {
        std::cerr << "can't set CWD to " << dir << '\n';
        return EXIT_FAILURE;
    }

    RefPtr<ResourceArchive> ra(new ResourceArchive());

    String absPathPackFile = cwd + pak;
    if (!ra->Open(absPathPackFile))
    {
        std::cerr << "can't open archive: " << absPathPackFile << '\n';
        return EXIT_FAILURE;
    }

    for (auto& fileInfo : ra->GetFilesInfo())
    {
        std::cout << "start unpacking: " << fileInfo.name
                  << " compressed: " << fileInfo.compressedSize
                  << " original: " << fileInfo.originalSize
                  << " packingType: " << ToString(fileInfo.compressionType)
                  << '\n';
        if (!UnpackFile(*ra, fileInfo))
        {
            return EXIT_FAILURE;
        }
    }
    std::cout << "Success!\n";
    return EXIT_SUCCESS;
}

int ListContent(const String& pakfile)
{
    RefPtr<FileSystem> fs(new FileSystem());
    if (!fs)
    {
        std::cerr << "can't create FileSystem\n";
        return EXIT_FAILURE;
    }
    RefPtr<ResourceArchive> ra(new ResourceArchive());
    if (!ra->Open(pakfile))
    {
        return EXIT_FAILURE;
    }

    for (auto& info : ra->GetFilesInfo())
    {
        std::cout << info.name << " compressed: " << info.compressedSize
                  << " original: " << info.originalSize
                  << " type: " << ToString(info.compressionType) << '\n';
    }
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    ProgramOptions packOptions("pack");
    packOptions.AddOption("--compression", VariantType(String("lz4hc")),
                          "default compression method, lz4hc - default");
    packOptions.AddOption("--add_hidden", VariantType(String("false")),
                          "add hidden files to pack list (false or true)");
    // default rule pack all files into lz4hc
    packOptions.AddOption("--rule", VariantType(String(".lz4hc")),
                          "rule to select compression type like: --rule "
                          "xml.lz4 supported lz4, lz4hc, none",
                          true);
    packOptions.AddArgument("directory");
    packOptions.AddArgument("pakfile");

    ProgramOptions unpackOptions("unpack");
    unpackOptions.AddArgument("pakfile");
    unpackOptions.AddArgument("directory");

    ProgramOptions listOptions("list");
    listOptions.AddArgument("pakfile");

    if (packOptions.Parse(argc, argv))
    {
        String compression = packOptions.GetOption("--compression").AsString();
        ResourceArchive::CompressionType defaultPackTypa =
        ToPackType(compression, ResourceArchive::CompressionType::Lz4HC);

        ResourceArchive::Rules compressionRules;

        if (!ParsePackRules(packOptions, defaultPackTypa, compressionRules))
        {
            std::cerr << "can't parse packing rules\n";
            return EXIT_FAILURE;
        }

        auto dirName = packOptions.GetArgument("directory");
        auto pakFile = packOptions.GetArgument("pakfile");
        auto addHidden = packOptions.GetOption("--add_hidden").AsString();

        return PackDirectory(dirName, pakFile, compressionRules,
                             addHidden == "true");
    }

    if (unpackOptions.Parse(argc, argv))
    {
        auto pakFile = unpackOptions.GetArgument("pakfile");
        auto dirName = unpackOptions.GetArgument("directory");

        return UnpackToDirectory(pakFile, dirName);
    }

    if (listOptions.Parse(argc, argv))
    {
        auto pakFile = listOptions.GetArgument("pakfile");
        return ListContent(pakFile);
    }

    {
        packOptions.PrintUsage();
        unpackOptions.PrintUsage();
        listOptions.PrintUsage();
        return EXIT_FAILURE;
    }
}
