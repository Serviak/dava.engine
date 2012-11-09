/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "FileSystem/FileSystem.h"
#include "FileSystem/FileList.h"
#include "Debug/DVAssert.h"
#include "Utils/Utils.h"
#include "Utils/StringFormat.h"
#include "FileSystem/ResourceArchive.h"


#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <copyfile.h>
#elif defined(__DAVAENGINE_WIN32__)
#include <direct.h>
#include <io.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <Shlobj.h>
#elif defined(__DAVAENGINE_ANDROID__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#endif //PLATFORMS

namespace DAVA
{

	
String FileSystem::virtualBundlePath = "";
	
    void FileSystem::ReplaceBundleName(const String &newBundlePath)
	{
		virtualBundlePath = newBundlePath;	
	}
	
	
#if defined(__DAVAENGINE_WIN32__)
    const char * FileSystem::FilepathRelativeToBundle(const char * relativePathname)
	{
		if(virtualBundlePath.empty())
		{
			return Format("./Data/%s", relativePathname);
		}
		else
		{
			return Format("%s/%s", virtualBundlePath.c_str(), relativePathname);
		}
	}
	
    const char * FileSystem::FilepathRelativeToBundle(const String & relativePathname)
	{
		return FilepathRelativeToBundle(relativePathname.c_str());
	}
#endif //#if defined(__DAVAENGINE_WIN32__)
	
	
#if defined(__DAVAENGINE_ANDROID__)

    const char * FileSystem::FilepathRelativeToBundle(const char * relativePathname)
	{
		return Format("assets/Data%s", relativePathname);
	}

    const char * FileSystem::FilepathRelativeToBundle(const String & relativePathname)
	{
		return FilepathRelativeToBundle(relativePathname.c_str());
	}

#endif //#if defined(__DAVAENGINE_ANDROID__)
	
FileSystem::FileSystem()
{
#if defined(__DAVAENGINE_ANDROID__)
	assetsPath[0] = 0;
	documentsPath[0] = 0;
	APKArchive = NULL;
#endif //#if defined(__DAVAENGINE_ANDROID__)

}

FileSystem::~FileSystem()
{	
	for (List<ResourceArchiveItem>::iterator ai = resourceArchiveList.begin();
		ai != resourceArchiveList.end(); ++ai)
	{
		ResourceArchiveItem & item = *ai;
		SafeRelease(item.archive);
	}
	resourceArchiveList.clear();

#if defined(__DAVAENGINE_ANDROID__)
	if(APKArchive)
	{
		zip_close(APKArchive);
		APKArchive = NULL;
	}
#endif //#if defined(__DAVAENGINE_ANDROID__)
}

FileSystem::eCreateDirectoryResult FileSystem::CreateDirectory(const String & filePath, bool isRecursive)
{
	if (!isRecursive)
	{
#ifdef __DAVAENGINE_WIN32__
		BOOL res = ::CreateDirectoryA(filePath.c_str(), 0);
		return (res == 0) ? DIRECTORY_CANT_CREATE : DIRECTORY_CREATED;
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
		int res = mkdir(filePath.c_str(), 0777);
		if (res == EEXIST)
			return DIRECTORY_EXISTS;
		return (res == 0) ? (DIRECTORY_CREATED) : (DIRECTORY_CANT_CREATE);
#endif //PLATFORMS
	}


	String path = filePath;
	std::replace(path.begin(), path.end(),'\\','/');
	Vector<String> tokens;
    Split(path, "/", tokens);
    
	String dir = "";

#if defined (__DAVAENGINE_WIN32__)
    if(0 < tokens.size() && 0 < tokens[0].length())
    {
        String::size_type pos = path.find(tokens[0]);
        if(String::npos != pos)
        {
            tokens[0] = path.substr(0, pos) + tokens[0];
        }
    }
#else //#if defined (__DAVAENGINE_WIN32__)
    size_t find = path.find(":");
    if(find == path.npos)
	{
        dir = "/";
    }
#endif //#if defined (__DAVAENGINE_WIN32__)
	
	for (size_t k = 0; k < tokens.size(); ++k)
	{
		dir += tokens[k] + "/";
#if defined(__DAVAENGINE_WIN32__)
		BOOL res = ::CreateDirectoryA(dir.c_str(), 0);
		if (k == tokens.size() - 1)
		{
			if (!res)
			{
				if (GetLastError() == ERROR_ALREADY_EXISTS)
					return DIRECTORY_EXISTS;
				else
					return DIRECTORY_CANT_CREATE;
			}
			return DIRECTORY_CREATED;
		}
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
		int res = mkdir(dir.c_str(), 0777);
		if (k == tokens.size() - 1)
		{
			if (res == EEXIST)
				return DIRECTORY_EXISTS;
			return (res == 0) ? (DIRECTORY_CREATED) : (DIRECTORY_CANT_CREATE);
		}
#endif //PLATFORMS
	}
	return DIRECTORY_CANT_CREATE;
}

bool FileSystem::CopyFile(const String & existingFile, const String & newFile)
{
#ifdef __DAVAENGINE_WIN32__
	BOOL ret = ::CopyFileA(existingFile.c_str(), newFile.c_str(), true);
	return ret != 0;
#elif defined(__DAVAENGINE_ANDROID__)

	int ret = 0;

	File *srcFile = File::Create(existingFile, File::OPEN | File::READ);
	File *dstFile = File::Create(existingFile, File::WRITE | File::CREATE);
	if(srcFile && dstFile)
	{
		int32 fileSize = srcFile->GetSize();

		const int32 TMP_DATA_SIZE = 1024;
		uint8 tmpData[TMP_DATA_SIZE];

		while(0 < fileSize)
		{
			int32 forRead = Min(TMP_DATA_SIZE, fileSize);
			int32 read = srcFile->Read(tmpData, forRead);
			if(read != forRead)
			{
				//error
			}

			int32 written = dstFile->Write(tmpData, read);
			if(written != read)
			{
				//error
			}

			fileSize -= written;
		}

		if(0 == fileSize)
		{
			ret = 1;
		}
	}

	SafeRelease(dstFile);
	SafeRelease(srcFile);

	return ret==0;

#else //iphone & macos
    int ret = copyfile(existingFile.c_str(), newFile.c_str(), NULL, COPYFILE_ALL | COPYFILE_EXCL);
    return ret==0;
	//DVASSERT(0 && "FileSystem::CopyFile not implemented for current platform");
#endif //PLATFORMS
}

bool FileSystem::MoveFile(const String & existingFile, const String & newFile)
{
#ifdef __DAVAENGINE_WIN32__
	BOOL ret = ::MoveFileA(existingFile.c_str(), newFile.c_str());
	return ret != 0;
#elif defined(__DAVAENGINE_ANDROID__)
	DVASSERT_MSG(0, "Not implemented");
#else //iphone & macos
	int ret = copyfile(existingFile.c_str(), newFile.c_str(), NULL, COPYFILE_ALL | COPYFILE_EXCL | COPYFILE_MOVE);
	return ret==0;
#endif //PLATFORMS
}


bool FileSystem::CopyDirectory(const String & sourceDirectory, const String & destinationDirectory)
{
	bool ret = true;

	FileList fileList(sourceDirectory);
	int32 count = fileList.GetCount();
	String fileOnly;
	String pathOnly;
	for(int32 i = 0; i < count; ++i)
	{
		if(!fileList.IsDirectory(i) && !fileList.IsNavigationDirectory(i))
		{
			const String & pathName = fileList.GetPathname(i);
			FileSystem::SplitPath(pathName, pathOnly, fileOnly);
			if(!CopyFile(pathName, destinationDirectory+"/"+fileOnly))
			{
				ret = false;
			}
		}
	}

	return ret;
}
	
bool FileSystem::DeleteFile(const String & filePath)
{
	// function unlink return 0 on success, -1 on error
	int res = remove(SystemPathForFrameworkPath(filePath).c_str());
	return (res == 0);
}
	
bool FileSystem::DeleteDirectory(const String & path, bool isRecursive)
{
	FileList * fileList = new FileList(path);
	for(int i = 0; i < fileList->GetCount(); ++i)
	{
		if(fileList->IsDirectory(i))
		{
			if(!fileList->IsNavigationDirectory(i))
			{
				if(isRecursive)
				{
//					Logger::Debug("- try to delete directory: %s / %s", fileList->GetPathname(i).c_str(), fileList->GetFilename(i).c_str());
					bool success = DeleteDirectory(fileList->GetPathname(i), isRecursive);
//					Logger::Debug("- delete directory: %s / %s- %d", fileList->GetPathname(i).c_str(), fileList->GetFilename(i).c_str(), success ? (1): (0));
					if (!success)return false;
				}
			}
		}
		else 
		{
			bool success = DeleteFile(fileList->GetPathname(i));
//			Logger::Debug("- delete file: %s / %s- %d", fileList->GetPathname(i).c_str(), fileList->GetFilename(i).c_str(), success ? (1): (0));
			if(!success)return false;
		}
	}
	SafeRelease(fileList);
#ifdef __DAVAENGINE_WIN32__
	String sysPath = SystemPathForFrameworkPath(path);
	int32 chmodres = _chmod(sysPath.c_str(), _S_IWRITE); // change read-only file mode
	int32 res = _rmdir(sysPath.c_str());
	return (res == 0);
	/*int32 res = ::RemoveDirectoryA(path.c_str());
	if (res == 0)
	{
		Logger::Warning("Failed to delete directory: %s error: 0x%x", path.c_str(), GetLastError());
	}
	return (res != 0);*/
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	int32 res = rmdir(SystemPathForFrameworkPath(path).c_str());
	return (res == 0);
#endif //PLATFORMS
}
	
uint32 FileSystem::DeleteDirectoryFiles(const String & path, bool isRecursive)
{
	uint32 fileCount = 0; 
	
	FileList * fileList = new FileList(path);
	for(int i = 0; i < fileList->GetCount(); ++i)
	{
		if(fileList->IsDirectory(i))
		{
			if(!fileList->IsNavigationDirectory(i))
			{
				if(isRecursive)
				{
					fileCount += DeleteDirectoryFiles(fileList->GetPathname(i), isRecursive);
				}
			}
		}
		else 
		{
			bool success = DeleteFile(fileList->GetPathname(i));
			if(success)fileCount++;
		}
	}
	SafeRelease(fileList);

	return fileCount;
}


	
File *FileSystem::CreateFileForFrameworkPath(const String & frameworkPath, uint32 attributes)
{
#if defined(__DAVAENGINE_ANDROID__)
    String::size_type find = frameworkPath.find("~res:");

	if(String::npos != find)
	{
		return File::CreateFromSystemPath(APKArchive, SystemPathForFrameworkPath(frameworkPath));
	}
	else
	{
		return File::CreateFromSystemPath(SystemPathForFrameworkPath(frameworkPath), attributes);
	}

#else //#if defined(__DAVAENGINE_ANDROID__)
	return File::CreateFromSystemPath(SystemPathForFrameworkPath(frameworkPath), attributes);
#endif //#if defined(__DAVAENGINE_ANDROID__)
}

const String & FileSystem::SystemPathForFrameworkPath(const String & frameworkPath)
{
	//DVASSERT(frameworkPath.size() > 0);
	if(frameworkPath[0] != '~')
	{
		return frameworkPath;
	}
	tempRetPath = frameworkPath;
	size_t find = tempRetPath.find("~res:");

	if(find != tempRetPath.npos)
	{
		tempRetPath = tempRetPath.erase(0, 5);
		tempRetPath = FilepathRelativeToBundle("") + tempRetPath;
	}
	else
	{
		find = tempRetPath.find("~doc:");
		if(find != tempRetPath.npos)
		{
			tempRetPath = tempRetPath.erase(0, 5);
			tempRetPath = FilepathInDocuments("") + tempRetPath;
		}
	}
	return tempRetPath;
}
	
const String & FileSystem::GetCurrentWorkingDirectory()
{
	char tempDir[2048];
#if defined(__DAVAENGINE_WIN32__)
	::GetCurrentDirectoryA(2048, tempDir);
	currentWorkingDirectory = tempDir;
	return currentWorkingDirectory;
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	getcwd(tempDir, 2048);
	currentWorkingDirectory = tempDir;
	return currentWorkingDirectory;
#endif //PLATFORMS
	return currentWorkingDirectory; 
}

bool FileSystem::SetCurrentWorkingDirectory(const String & newWorkingDirectory)
{
#if defined(__DAVAENGINE_WIN32__)
	BOOL res = ::SetCurrentDirectoryA(newWorkingDirectory.c_str());
	return (res != 0);
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	return !(chdir(newWorkingDirectory.c_str()) == 0);
#endif //PLATFORMS
	return false; 
}
    
bool FileSystem::IsDirectory(const String & pathToCheck)
{
    struct stat s;
    if( stat(pathToCheck.c_str(),&s) == 0 )
    {
        if( s.st_mode & S_IFDIR )
        {
            return true;
        }
        else if( s.st_mode & S_IFREG )
        {
            //it's a file
        }
        else
        {
            //something else
        }
    }
    else
    {
        //error
    }
    return false;
}

    const String & FileSystem::GetCurrentDocumentsDirectory()
    {
        return currentDocDirectory; 
    }
    
    void FileSystem::SetCurrentDocumentsDirectory(const String & newDocDirectory)
    {
        currentDocDirectory = newDocDirectory;
    }
    
    const String FileSystem::FilepathInDocuments(const char * relativePathname)
    {
        //return Format("./Documents/%s", relativePathname);
        return currentDocDirectory + relativePathname;
    }
    
    const String FileSystem::FilepathInDocuments(const String & relativePathname)
    {
        return FilepathInDocuments(relativePathname.c_str());
    }
    
    void FileSystem::SetDefaultDocumentsDirectory()
    {
#if defined(__DAVAENGINE_WIN32__)
        SetCurrentDocumentsDirectory(GetUserDocumentsPath() + "DAVAProject\\");
#elif defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__) || defined (__DAVASOUND_ANDROID__)
        SetCurrentDocumentsDirectory(GetUserDocumentsPath() + "DAVAProject/");
#endif //PLATFORMS
    }
    
	
#if defined(__DAVAENGINE_WIN32__)
    const String FileSystem::GetUserDocumentsPath()
    {
        char * szPath = new char[MAX_PATH];
        SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, szPath);
        int32 n = strlen(szPath);
        szPath[n] = '\\';
        szPath[n+1] = 0;
        String str(szPath);
        delete[] szPath;
        return str;
    }
    
    const String FileSystem::GetPublicDocumentsPath()
    {
        char * szPath = new char[MAX_PATH];
        SHGetFolderPathA(NULL, CSIDL_COMMON_DOCUMENTS, NULL, SHGFP_TYPE_CURRENT, szPath);
        int32 n = strlen(szPath);
        szPath[n] = '\\';
        szPath[n+1] = 0;
        String str(szPath);
        delete[] szPath;
        return str;
    }
#endif //#if defined(__DAVAENGINE_WIN32__)
    
#if defined(__DAVAENGINE_ANDROID__)
    const String FileSystem::GetUserDocumentsPath()
    {
        return documentsPath;
    }
    
    const String FileSystem::GetPublicDocumentsPath()
    {
        //TODO: need to return real path;
        return documentsPath;
    }
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)	
    
String FileSystem::RealPath(const String & _path)
{
	
	String path = (_path);
	std::replace(path.begin(), path.end(),'\\','/');
	
	const String & delims="/";
	
	// Skip delims at beginning, find start of first token
	String::size_type lastPos = path.find_first_not_of(delims, 0);
	// Find next delimiter @ end of token
	String::size_type pos     = path.find_first_of(delims, lastPos);
	
	// output vector
	Vector<String> tokens;
	
	
	while (String::npos != pos || String::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(path.substr(lastPos, pos - lastPos));
		// Skip delims.  Note the "not_of". this is beginning of token
		lastPos = path.find_first_not_of(delims, pos);
		// Find next delimiter at end of token.
		pos     = path.find_first_of(delims, lastPos);
	}
	
	if (tokens.size() > 0)
	{
		if (tokens[0] == ".")
		{
			tokens[0] = FileSystem::Instance()->GetCurrentWorkingDirectory();
			std::replace(tokens[0].begin(), tokens[0].end(),'\\','/');
		}
	}

	String result;
	
	for (int i = 0; i < (int)tokens.size(); ++i)
	{
		if (tokens[i] == String("."))
		{		
			for (int k = i + 1; k < (int)tokens.size(); ++k)
			{
				tokens[k - 1] = tokens[k];
			}
			i--;
			tokens.pop_back();
			continue;
		}
		if (tokens[i] == String(".."))
		{		
			for (int k = i + 1; k < (int)tokens.size(); ++k)
			{
				tokens[k - 2] = tokens[k];
			}
			i-=2;
			tokens.pop_back();
			tokens.pop_back();
			continue;
		}	
	}
#if !defined(__DAVAENGINE_WIN32__)
	result = "/";
#endif
	for (int k = 0; k < (int)tokens.size(); ++k)
	{
		result += tokens[k];
		if (k + 1 != (int)tokens.size())
			result += String("/");
	}
	return result;
}

String FileSystem::NormalizePath(const String & _path)
{
	if(_path.empty())
		return String("");
	
	String path = (_path);
    std::replace(path.begin(), path.end(),'\\','/');

    Vector<String> tokens;
    Split(path, "/", tokens);

    //TODO: correctly process situation ../../folders/filename
    for (int32 i = 0; i < (int32)tokens.size(); ++i)
    {
        if (String(".") == tokens[i])
        {		
            for (int32 k = i + 1; k < (int32)tokens.size(); ++k)
            {
                tokens[k - 1] = tokens[k];
            }
            --i;
            tokens.pop_back();
        }
        else if ((1 <= i) && (String("..") == tokens[i] && String("..") != tokens[i-1]))
        {		
            for (int32 k = i + 1; k < (int32)tokens.size(); ++k)
            {
                tokens[k - 2] = tokens[k];
            }
            i-=2;
            tokens.pop_back();
            tokens.pop_back();
        }	
    }

    String result = "";
    if('/' == path[0])
		result = "/";
    
    for (int32 k = 0; k < (int32)tokens.size(); ++k)
    {
        result += tokens[k];
        if (k + 1 != (int32)tokens.size())
            result += String("/");
    }

	//process last /
	if(('/' == path[path.length() - 1]) && (path.length() != 1)) 
		result += String("/");

    return result;
}

    
String FileSystem::GetCanonicalPath(const String &path)
{
	String canonicalPath = FileSystem::Instance()->NormalizePath(path);
    
    String::size_type resPos = canonicalPath.find("~res:");
    String::size_type docPos = canonicalPath.find("~doc:");
    
    if(String::npos == resPos && String::npos == docPos)
    {
        String::size_type colonPos = canonicalPath.find(":");
        if((String::npos != colonPos) && (colonPos < canonicalPath.length() - 1))
        {
            canonicalPath = canonicalPath.substr(colonPos + 1);
        }
    }
    
	return canonicalPath;
}

    
String FileSystem::ReplaceExtension(const String & filename, const String & newExt)
{
	String::size_type dotpos = filename.rfind(".");
	if (dotpos == String::npos)
		return String();
	
	String result = filename.substr(0, dotpos) + newExt;
	return result;
}	

String	FileSystem::GetExtension(const String & filename)
{
	String::size_type dotpos = filename.rfind(".");
	if (dotpos == String::npos)
		return String();
	return filename.substr(dotpos);
}
	
void  FileSystem::SplitPath(const String & filePath, String & path, String & filename)
{
	String fullPath(filePath);
	std::replace(fullPath.begin(),fullPath.end(),'\\','/');
	// now only Unix style slashes
	String::size_type lastSlashPos = fullPath.find_last_of('/');
	
	if (lastSlashPos==String::npos)
	{
		path = "";
		filename = fullPath;
	}
	else
	{
		path = fullPath.substr(0, lastSlashPos) + '/';
		filename = fullPath.substr(lastSlashPos + 1, fullPath.size() - lastSlashPos - 1);
	}
}

uint8 * FileSystem::ReadFileContents(const String & pathname, uint32 & fileSize)
{
    File * fp = File::Create(pathname, File::OPEN|File::READ);
	if (!fp)
	{
		Logger::Error("Failed to open file: %s", pathname.c_str());
		return 0;
	}
	fileSize = fp->GetSize();
	uint8 * bytes = new uint8[fileSize];
	uint32 dataRead = fp->Read(bytes, fileSize);
    
	if (dataRead != fileSize)
	{
		Logger::Error("Failed to read data from file: %s", pathname.c_str());
		return 0;
	}

	SafeRelease(fp);
    return bytes;
};

void FileSystem::AttachArchive(const String & archiveName, const String & attachPath)
{
	ResourceArchive * resourceArchive = new ResourceArchive();

	if (!resourceArchive->Open(archiveName)) 
	{
		delete resourceArchive;
		resourceArchive = 0;
		return;
	}
	ResourceArchiveItem item;
	item.attachPath = attachPath;
	item.archive = resourceArchive;
	resourceArchiveList.push_back(item);
}

int32 FileSystem::Spawn(const String& command)
{
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_WIN32__) 
	return std::system(command.c_str());
#else
	return 0;
#endif
}

String FileSystem::AbsoluteToRelativePath(const String &folderPathname, const String &absolutePathname)
{
    String workingFolderPath = folderPathname;
    String workingFilePath = absolutePathname;
    
    std::replace(workingFolderPath.begin(),workingFolderPath.end(),'\\','/');
    std::replace(workingFilePath.begin(),workingFilePath.end(),'\\','/');
    
    if((0 == absolutePathname.length()) || ('/' != absolutePathname[0]))
        return absolutePathname;

    String filePath;
    String fileName;
    FileSystem::SplitPath(workingFilePath, filePath, fileName);
    
    Vector<String> folders;
    Split(workingFolderPath, "/", folders);
    Vector<String> fileFolders;
    Split(filePath, "/", fileFolders);

    Vector<String>::size_type equalCount = 0;
    for(; equalCount < folders.size() && equalCount < fileFolders.size(); ++equalCount)
    {
        if(folders[equalCount] != fileFolders[equalCount])
        {
            break;
        }
    }
    
    String retPath = "";
    for(Vector<String>::size_type i = equalCount; i < folders.size(); ++i)
    {
        retPath += "../";
    }

    for(Vector<String>::size_type i = equalCount; i < fileFolders.size(); ++i)
    {
        retPath += fileFolders[i] + "/";
    }
    
    return (retPath + fileName);
}

#if defined(__DAVAENGINE_ANDROID__)

void FileSystem::SetPath(const char8 *docPath, const char8 *assets)
{
	strcpy(documentsPath, docPath);
	strcpy(assetsPath, assets);

	if(APKArchive)
	{
		zip_close(APKArchive);
		APKArchive = NULL;
	}

	APKArchive = zip_open(assetsPath, 0, NULL);
	if (!APKArchive)
	{
		Logger::Error("[FileSystem::SetPath] can't open APK from path: %s", assetsPath);
	}
}




#endif //#if defined(__DAVAENGINE_ANDROID__)
    
}




