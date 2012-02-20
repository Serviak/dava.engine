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
#ifndef __DAVAENGINE_FILELIST_H__
#define __DAVAENGINE_FILELIST_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FileSystem.h"
#include "Utils/Utils.h"

namespace DAVA
{

/** 
	\brief Class used to enumerate files in directories
*/
class	FileList : public BaseObject
{
public:
	FileList(const String & filepath);
	virtual ~FileList();

	/**
		\brief Get total item count in current list
		This function return total number of items in directory including "." and ".." items
		\returns number of items in this directory,
	 */
	int32 GetCount();
	
	/**
		\brief Get total item count in current list
		This function return number of files in directory
		\returns number of files in this directory
	 */
	int32 GetFileCount();
	/**
		\brief Get total directory count in current list
		This function return number of files in directory
		\returns number of subdirectories in this directory
	 */
	int32 GetDirectoryCount();
	
	/**
		\brief Get current path 
		
	 */
	const String & GetCurrentPath();
	
	//! Get file name
	const String & GetFilename(int32 index);
    
	//! Get path name
	const String & GetPathname(int32 index);

	/**
		\brief is file with given index in this list is a directory
		\return true if this is directory
	 */
	bool IsDirectory(int32 index);
	
	/*
		\brief is file with given index, is navigation directory. 
		This funciton checks is directory == "." or directory == ".."
		\return true if this is ".", or ".." directory
	 */
	bool IsNavigationDirectory(int32 index);
	
    void Sort();
    
private:
	struct FileEntry
	{
		String name;
		String pathName;
		uint32		size;
		bool		isDirectory;
        
        bool operator< (const FileEntry &other) const
        {
            if (!isDirectory && other.isDirectory) 
            {
                return false;
            }
            else if(isDirectory && !other.isDirectory)
            {
                return true;
            }
            
            return (CompareStrings(name, other.name) < 0);
        }
	};
	String					path;
	Vector< FileEntry >	fileList;
	int32				fileCount;
	int32				directoryCount;
};


}; // end of namespace DAVA


#endif // __LOGENGINE_ANSIFILESYSTEM_H__