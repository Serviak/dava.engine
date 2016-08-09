#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

using namespace DAVA;

auto GetIndex = [](const FileList* files, DAVA::String filename)
{
    auto i = 0;
    for (; i < files->GetCount(); ++i)
    {
        if (files->GetFilename(i) == filename)
            break;
    }
    return i;
};

DAVA_TESTCLASS (FileListTest)
{
    DEDUCE_COVERED_FILES_FROM_TESTCLASS()

    FileListTest()
    {
        FileSystem::Instance()->DeleteDirectory("~doc:/TestData/FileListTest/", true);
        FileSystem::Instance()->RecursiveCopy("~res:/TestData/FileListTest/", "~doc:/TestData/FileListTest/");
        
    #if defined(__DAVAENGINE_WINDOWS__)
        FileSystem::Instance()->DeleteDirectory("~doc:/TestData/FileListTestWindowsExtension/", true);
        FileSystem::Instance()->RecursiveCopy("~res:/TestData/FileListTestWindowsExtension/", "~doc:/TestData/FileListTestWindowsExtension/");
    #endif
    }

    DAVA_TEST (ResTestFunction)
    {
        ScopedPtr<FileList> fileList(new FileList("~res:/TestData/FileListTest/"));

        TEST_VERIFY(fileList->GetDirectoryCount() == 3);
        TEST_VERIFY(fileList->GetFileCount() == 0);

        for (int32 ifo = 0; ifo < fileList->GetCount(); ++ifo)
        {
            if (fileList->IsNavigationDirectory(ifo))
                continue;

            String filename = fileList->GetFilename(ifo);
            FilePath pathname = fileList->GetPathname(ifo);
            ScopedPtr<FileList> files(new FileList(pathname));
            TEST_VERIFY(files->GetDirectoryCount() == 0);

            if (filename == "Folder1")
            {
                TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/");
                TEST_VERIFY(files->GetFileCount() == 3);

                for (int32 ifi = 0; ifi < files->GetCount(); ++ifi)
                {
                    if (files->IsNavigationDirectory(ifi))
                        continue;

                    String filename = files->GetFilename(ifi);
                    FilePath pathname = files->GetPathname(ifi);

                    if (filename == "file1")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/file1");
                    }
                    else if (filename == "file2.txt")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/file2.txt");
                    }
                    else if (filename == "file3.doc")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder1/file3.doc");
                    }
                    else
                    {
                        TEST_VERIFY(false);
                    }
                }
            }
            else if (filename == "Folder2")
            {
                TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/");
                TEST_VERIFY(files->GetFileCount() == 6);
                for (int32 ifi = 0; ifi < files->GetCount(); ++ifi)
                {
                    if (files->IsNavigationDirectory(ifi))
                        continue;

                    String filename = files->GetFilename(ifi);
                    FilePath pathname = files->GetPathname(ifi);

                    if (filename == "file1")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file1");
                    }
                    else if (filename == "file1.txt")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file1.txt");
                    }
                    else if (filename == "file2")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file2");
                    }
                    else if (filename == "file2.txt")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file2.txt");
                    }
                    else if (filename == "file3")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file3");
                    }
                    else if (filename == "file3.doc")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder2/file3.doc");
                    }
                    else
                    {
                        TEST_VERIFY(false);
                    }
                }
            }
            else if (filename == "Folder3")
            {
                TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder3/");
                TEST_VERIFY(files->GetFileCount() == 2);
                for (int32 ifi = 0; ifi < files->GetCount(); ++ifi)
                {
                    if (files->IsNavigationDirectory(ifi))
                        continue;

                    String filename = files->GetFilename(ifi);
                    FilePath pathname = files->GetPathname(ifi);

                    if (filename == "file1")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder3/file1");
                    }
                    else if (filename == "file3.doc")
                    {
                        TEST_VERIFY(pathname == "~res:/TestData/FileListTest/Folder3/file3.doc");
                    }
                    else
                    {
                        TEST_VERIFY(false);
                    }
                }
            }
            else
            {
                TEST_VERIFY(false);
            }
        }
    }

#if defined(__DAVAENGINE_WINDOWS__)
    DAVA_TEST (FileListTestWindowsExtensions)
    {
        FileSystem* fs = FileSystem::Instance();
        const String fileContent = "Hello :)";
        const FilePath extPath = "~doc:/TestData/FileListTestWindowsExtension/";

        //extract cyrillic path from file
        String cyrillicPathString;
        RefPtr<File> cyrillicPathFile(File::Create(extPath + "path.txt", File::OPEN | File::READ));
        TEST_VERIFY(cyrillicPathFile != nullptr);

        cyrillicPathFile->ReadString(cyrillicPathString);
        TEST_VERIFY(!cyrillicPathString.empty());

        //create path and file
        const FilePath cyrillicPath = extPath + cyrillicPathString;
        FileSystem::eCreateDirectoryResult result = fs->CreateDirectory(cyrillicPath.GetDirectory());
        TEST_VERIFY(result == FileSystem::DIRECTORY_CREATED);
        TEST_VERIFY(fs->IsDirectory(cyrillicPath.GetDirectory()));

        RefPtr<File> cyrillicFile(File::Create(cyrillicPath, File::CREATE | File::WRITE));
        TEST_VERIFY(cyrillicFile != nullptr);
        cyrillicFile->WriteString(fileContent);
        cyrillicFile = nullptr;
        TEST_VERIFY(fs->IsFile(cyrillicPath));

        //explore created path
        String upperLevel = cyrillicPath.GetDirectory().GetAbsolutePathname();
        upperLevel.pop_back();
        RefPtr<FileList> fileList(new FileList(FilePath(upperLevel).GetDirectory()));
        TEST_VERIFY(fileList->GetDirectoryCount() == 1); //cyrillic directory
        TEST_VERIFY(fileList->GetFileCount() == 1); //file with cyrillic path definition

        for (int32 ifo = 0; ifo < fileList->GetCount(); ++ifo)
        {
            if (fileList->IsNavigationDirectory(ifo) || !fileList->IsDirectory(ifo))
            {
                continue;
            }

            String filename = fileList->GetFilename(ifo);
            FilePath pathname = fileList->GetPathname(ifo);
            RefPtr<FileList> files(new FileList(pathname));
            TEST_VERIFY(files->GetDirectoryCount() == 0);

            TEST_VERIFY(files->GetFileCount() == 1);
            FilePath txtFileName = files->GetPathname(2); //first file name
            TEST_VERIFY(txtFileName.GetExtension() == ".txt");

            RefPtr<File> file(File::Create(txtFileName, File::OPEN | File::READ));
            TEST_VERIFY(file != nullptr);
            if (file == nullptr)
            {
                continue;
            }

            String content;
            file->ReadString(content);

            TEST_VERIFY(content == fileContent);
        }
    }
#endif // __DAVAENGINE_WINDOWS__

    DAVA_TEST (DocTestFunction)
    {
        ScopedPtr<FileList> fileList(new FileList("~doc:/TestData/FileListTest/"));

        TEST_VERIFY(fileList->GetDirectoryCount() == 3);
        TEST_VERIFY(fileList->GetFileCount() == 0);

        for (int32 ifo = 0; ifo < fileList->GetCount(); ++ifo)
        {
            if (fileList->IsNavigationDirectory(ifo))
                continue;

            String filename = fileList->GetFilename(ifo);
            FilePath pathname = fileList->GetPathname(ifo);
            ScopedPtr<FileList> files(new FileList(pathname));
            TEST_VERIFY(files->GetDirectoryCount() == 0);

            if (filename == "Folder1")
            {
                TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/");
                TEST_VERIFY(files->GetFileCount() == 3);

                for (int32 ifi = 0; ifi < files->GetCount(); ++ifi)
                {
                    if (files->IsNavigationDirectory(ifi))
                        continue;

                    String filename = files->GetFilename(ifi);
                    FilePath pathname = files->GetPathname(ifi);

                    if (filename == "file1")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/file1");
                    }
                    else if (filename == "file2.txt")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/file2.txt");
                    }
                    else if (filename == "file3.doc")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder1/file3.doc");
                    }
                    else
                    {
                        TEST_VERIFY(false);
                    }
                }
            }
            else if (filename == "Folder2")
            {
                TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/");
                TEST_VERIFY(files->GetFileCount() == 6);
                for (int32 ifi = 0; ifi < files->GetCount(); ++ifi)
                {
                    if (files->IsNavigationDirectory(ifi))
                        continue;

                    String filename = files->GetFilename(ifi);
                    FilePath pathname = files->GetPathname(ifi);

                    if (filename == "file1")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file1");
                    }
                    else if (filename == "file1.txt")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file1.txt");
                    }
                    else if (filename == "file2")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file2");
                    }
                    else if (filename == "file2.txt")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file2.txt");
                    }
                    else if (filename == "file3")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file3");
                    }
                    else if (filename == "file3.doc")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder2/file3.doc");
                    }
                    else
                    {
                        TEST_VERIFY(false);
                    }
                }
            }
            else if (filename == "Folder3")
            {
                TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder3/");
                TEST_VERIFY(files->GetFileCount() == 2);
                for (int32 ifi = 0; ifi < files->GetCount(); ++ifi)
                {
                    if (files->IsNavigationDirectory(ifi))
                        continue;

                    String filename = files->GetFilename(ifi);
                    FilePath pathname = files->GetPathname(ifi);

                    if (filename == "file1")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder3/file1");
                    }
                    else if (filename == "file3.doc")
                    {
                        TEST_VERIFY(pathname == "~doc:/TestData/FileListTest/Folder3/file3.doc");
                    }
                    else
                    {
                        TEST_VERIFY(false);
                    }
                }
            }
            else
            {
                TEST_VERIFY(false);
            }
        }
    }

    DAVA_TEST (HiddenFileTest)
    {
#if defined(__DAVAENGINE_WIN32__)
        FilePath file1 = FilePath("~doc:/TestData/FileListTest/Folder1/file1");
        auto file1str = file1.GetAbsolutePathname();
        auto attrs = GetFileAttributesA(file1str.c_str());

        if (attrs & FILE_ATTRIBUTE_HIDDEN)
        {
            SetFileAttributesA(file1str.c_str(), attrs ^ FILE_ATTRIBUTE_HIDDEN);
        }

        ScopedPtr<FileList> files(new FileList("~doc:/TestData/FileListTest/Folder1/"));
        TEST_VERIFY(files->GetFileCount() == 3);
        auto i = GetIndex(files, "file1");
        TEST_VERIFY(i < files->GetCount());
        TEST_VERIFY(files->IsHidden(i) == false);

        SetFileAttributesA(file1str.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);

        files = new FileList("~doc:/TestData/FileListTest/Folder1/");
        TEST_VERIFY(files->GetFileCount() == 3);
        i = GetIndex(files, "file1");
        TEST_VERIFY(i < files->GetCount());
        TEST_VERIFY(files->IsHidden(i) == true);

        SetFileAttributesA(file1str.c_str(), attrs);
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
        FilePath file1 = "~doc:/TestData/FileListTest/Folder1/file1";
        FilePath file1hidden = "~doc:/TestData/FileListTest/Folder1/.file1";
        TEST_VERIFY(FileSystem::Instance()->CopyFile(file1, file1hidden, true));
        ScopedPtr<FileList> files(new FileList("~doc:/TestData/FileListTest/Folder1/"));
        TEST_VERIFY(files->GetFileCount() == 4);
        for (auto i = 0; i < files->GetCount(); ++i)
        {
            if (!files->IsDirectory(i))
            {
                bool startsWithDot = (files->GetFilename(i)[0] == '.');
                TEST_VERIFY(files->IsHidden(i) == startsWithDot);
            }
        }
        FileSystem::Instance()->DeleteFile(file1hidden);
#endif //PLATFORMS
    }

    DAVA_TEST (HiddenDirTest)
    {
#if defined(__DAVAENGINE_WIN32__)
        FilePath dir1 = FilePath("~doc:/TestData/FileListTest/Folder1/");
        auto dir1str = dir1.GetAbsolutePathname();
        auto attrs = GetFileAttributesA(dir1str.c_str());

        if (attrs & FILE_ATTRIBUTE_HIDDEN)
        {
            SetFileAttributesA(dir1str.c_str(), attrs ^ FILE_ATTRIBUTE_HIDDEN);
        }

        ScopedPtr<FileList> files(new FileList("~doc:/TestData/FileListTest/"));
        TEST_VERIFY(files->GetDirectoryCount() == 3);
        auto i = GetIndex(files, "Folder1");
        TEST_VERIFY(i < files->GetCount());
        TEST_VERIFY(files->IsHidden(i) == false);

        SetFileAttributesA(dir1str.c_str(), attrs | FILE_ATTRIBUTE_HIDDEN);

        files = new FileList("~doc:/TestData/FileListTest/");
        TEST_VERIFY(files->GetDirectoryCount() == 3);
        i = GetIndex(files, "Folder1");
        TEST_VERIFY(i < files->GetCount());
        TEST_VERIFY(files->IsHidden(i) == true);

        SetFileAttributesA(dir1str.c_str(), attrs);

#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

        FilePath folder1hidden = "~doc:/TestData/FileListTest/.Folder1/";
        TEST_VERIFY(FileSystem::Instance()->CreateDirectory(folder1hidden, true));
        ScopedPtr<FileList> files(new FileList("~doc:/TestData/FileListTest/"));
        TEST_VERIFY(files->GetDirectoryCount() == 4);
        for (auto i = 0; i < files->GetCount(); ++i)
        {
            if (files->IsDirectory(i))
            {
                bool startsWithDot = (files->GetFilename(i)[0] == '.');
                TEST_VERIFY(files->IsHidden(i) == startsWithDot);
            }
        }
        FileSystem::Instance()->DeleteDirectory(folder1hidden);

#endif //PLATFORMS
    }
};
