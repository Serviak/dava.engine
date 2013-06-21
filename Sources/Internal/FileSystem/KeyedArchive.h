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
#ifndef __DAVAENGINE_KEYED_ARCHIVE_H__
#define __DAVAENGINE_KEYED_ARCHIVE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/VariantType.h"
#include "FileSystem/File.h"

#include "Math/MathConstants.h"
#include "Math/Matrix2.h"
#include "Math/Matrix3.h"
#include "Math/Matrix4.h"
#include "Math/Math2D.h"


namespace DAVA 
{
/**
	\ingroup filesystem
	\brief this is a class that should be used for serialization & deserialization of the items
 */
class YamlNode;
    
class KeyedArchive : public BaseObject
{
public:
	KeyedArchive();
	KeyedArchive(const KeyedArchive &arc);
	virtual ~KeyedArchive();
	
	/**
        \brief Dumps archive to console
	 */
	void Dump();

	/**
		\brief Function to check if key is available in this archive.
		\param[in] key string key
		\returns true if key available
	 */
	bool IsKeyExists(const String & key);
	
	/**
		\brief Function to get variable from archive.
		\param[in] key string key
		\param[in] defaultValue this is value that is used if variable with this key do not exists in archive
		\returns value of variable or defaultValue if key isn't available
	 */
	bool GetBool(const String & key, bool defaultValue = false);
	/**
		\brief Function to get variable from archive.
		\param[in] key string key
		\param[in] defaultValue this is value that is used if variable with this key do not exists in archive
		\returns value of variable or defaultValue if key isn't available
	 */
	int32 GetInt32(const String & key, int32 defaultValue = 0);
	/**
     \brief Function to get variable from archive.
     \param[in] key string key
     \param[in] defaultValue this is value that is used if variable with this key do not exists in archive
     \returns value of variable or defaultValue if key isn't available
	 */
	uint32 GetUInt32(const String & key, uint32 defaultValue = 0);
	/**
		\brief Function to get variable from archive.
		\param[in] key string key
		\param[in] defaultValue this is value that is used if variable with this key do not exists in archive
		\returns value of variable or defaultValue if key isn't available
	 */
	float32 GetFloat(const String & key, float32 defaultValue = 0.0f);
	/**
		\brief Functions to get variable from archive.
		\param[in] key string key
		\param[in] defaultValue this is value that is used if variable with this key do not exists in archive
		\returns value of variable or defaultValue if key isn't available
	 */
	String GetString(const String & key, const String & defaultValue = "");
	/**
		\brief Function to get variable from archive.
		\param[in] key string key
		\param[in] defaultValue this is value that is used if variable with this key do not exists in archive
		\returns value of variable or defaultValue if key isn't available
		*/

	WideString GetWideString(const String & key, const WideString & defaultValue = L"");
	/**
        \brief Function to get variable from archive.
        \param[in] key string key
        \param[in] defaultValue this is value that is used if variable with this key do not exists in archive
        \returns value of variable or defaultValue if key isn't available
	 */
	const uint8 *GetByteArray(const String & key, const uint8 *defaultValue = NULL);
	/**
        \brief Function to get variable from archive.
        \param[in] key string key
        \param[in] defaultValue this is value that is used if variable with this key do not exists in archive
        \returns value of variable or defaultValue if key isn't available
	 */
	int32 GetByteArraySize(const String & key, int32 defaultValue = 0);
    
    /**
        \brief Function to load data from byte array as keyed archive.
        Call to this function is equivalent to creation of KeyedArchive class. Object returned from this function should be released. 
        If key is unavailable function returns 0
        \param[in] key string key
        \param[in] value we want to set for this key
	 */
	KeyedArchive * GetArchiveFromByteArray(const String & key);

    /**
     \brief Function to get archive from archive. Returns pointer to the archive inside.
     \param[in] key string key
     \param[in] defaultValue we want to set for this key
	 */
	KeyedArchive * GetArchive(const String & key, KeyedArchive * defaultValue = 0);
    
    /**
     \brief Function to get variable from archive.
     \param[in] key string key
     \param[in] defaultValue this is value that is used if variable with this key do not exists in archive
     \returns value of variable or defaultValue if key isn't available
	 */
	int64 GetInt64(const String & key, int64 defaultValue = 0);
    
    /**
     \brief Function to get variable from archive.
     \param[in] key string key
     \param[in] defaultValue this is value that is used if variable with this key do not exists in archive
     \returns value of variable or defaultValue if key isn't available
	 */
	uint64 GetUInt64(const String & key, uint64 defaultValue = 0);
    
    /**
     \brief Function to get variable from archive.
     \param[in] key string key
     \param[in] defaultValue this is value that is used if variable with this key do not exists in archive
     \returns value of variable or defaultValue if key isn't available
	 */
	Vector2 GetVector2(const String & key, const Vector2 & defaultValue = Vector2());
    
    /**
     \brief Function to get variable from archive.
     \param[in] key string key
     \param[in] defaultValue this is value that is used if variable with this key do not exists in archive
     \returns value of variable or defaultValue if key isn't available
	 */
	Vector3 GetVector3(const String & key, const Vector3 & defaultValue = Vector3());
  
    /**
     \brief Function to get variable from archive.
     \param[in] key string key
     \param[in] defaultValue this is value that is used if variable with this key do not exists in archive
     \returns value of variable or defaultValue if key isn't available
	 */
	Vector4 GetVector4(const String & key, const Vector4 & defaultValue = Vector4());

    /**
     \brief Function to get variable from archive.
     \param[in] key string key
     \param[in] defaultValue this is value that is used if variable with this key do not exists in archive
     \returns value of variable or defaultValue if key isn't available
	 */
	Matrix2 GetMatrix2(const String & key, const Matrix2 & defaultValue = Matrix2());
    
    /**
     \brief Function to get variable from archive.
     \param[in] key string key
     \param[in] defaultValue this is value that is used if variable with this key do not exists in archive
     \returns value of variable or defaultValue if key isn't available
	 */
	Matrix3 GetMatrix3(const String & key, const Matrix3 & defaultValue = Matrix3());
    
    /**
     \brief Function to get variable from archive.
     \param[in] key string key
     \param[in] defaultValue this is value that is used if variable with this key do not exists in archive
     \returns value of variable or defaultValue if key isn't available
	 */
	Matrix4 GetMatrix4(const String & key, const Matrix4 & defaultValue = Matrix4());
    /*
        \brief Function to get object from byte array.
        \param[in] key string key
        \returns object
     */
    template <class T>
    T GetByteArrayAsType(const String & key, const T & defaultValue = T());

	
	/**
		\brief Function to get variable from archive.
		\param[in] key string key
		\returns value of variable or default VariantType class if value isn't available
	 */
	VariantType *GetVariant(const String & key);

	/**
		\brief Function to set variable in archive.
		\param[in] key string key
		\param[in] value we want to set for this key
	 */
	void SetBool(const String & key, bool value);
	/**
		\brief Function to set variable in archive.
		\param[in] key string key
		\param[in] value we want to set for this key
	 */
	void SetInt32(const String & key, int32 value);
	/**
     \brief Function to set variable in archive.
     \param[in] key string key
     \param[in] value we want to set for this key
	 */
	void SetUInt32(const String & key, uint32 value);
    
	/**
		\brief Function to set variable in archive.
		\param[in] key string key
		\param[in] value we want to set for this key
	 */
	void SetFloat(const String & key, float32 value);
	/**
		\brief function to set variable in archive
		\param[in] key string key
		\param[in] value we want to set for this key
	 */
	void SetString(const String & key, const String & value);
	/**
		\brief Function to set variable in archive.
		\param[in] key string key
		\param[in] value we want to set for this key
	 */

	void SetWideString(const String & key, const WideString & value);
	/**
        \brief Function to set variable in archive.
        \param[in] key string key
        \param[in] value we want to set for this key
        \param[in] arraySize size fo the array we want tot save
	 */
	void SetByteArray(const String & key, const uint8 * value, int32 arraySize);
	/**
		\brief Function to set variable in archive. Variant value is copying inside this method
		\param[in] key string key
		\param[in] value we want to set for this key
	 */
	void SetVariant(const String & key, const VariantType &value);
	/**
        \brief Function to set another keyed archive as kye for this archive.
        \param[in] key string key
        \param[in] value we want to set for this key
	 */
	void SetByteArrayFromArchive(const String & key, KeyedArchive * archive);

	/**
     \brief Function to set another keyed archive as key for this archive.
     Function is copying archive inside. If you need to work with this archive later use GetArchive().
     \param[in] key string key
     \param[in] value we want to set for this key
	 */
	void SetArchive(const String & key, KeyedArchive * archive);
    
    /**
     \brief Function to set variable in archive.
     \param[in] key string key
     \param[in] value we want to set for this key
	 */
	void SetInt64(const String & key, int64 &value);
	/**
     \brief Function to set variable in archive.
     \param[in] key string key
     \param[in] value we want to set for this key
	 */
	void SetUInt64(const String & key, uint64 &value);
    
	/**
     \brief Function to set variable in archive.
     \param[in] key string key
     \param[in] value we want to set for this key
	 */
	void SetVector2(const String & key, Vector2 &value);
    
    /**
     \brief Function to set variable in archive.
     \param[in] key string key
     \param[in] value we want to set for this key
	 */
	void SetVector3(const String & key, Vector3 &value);
 
    /**
     \brief Function to set variable in archive.
     \param[in] key string key
     \param[in] value we want to set for this key
	 */
	void SetVector4(const String & key, Vector4 &value);

    /**
     \brief Function to set variable in archive.
     \param[in] key string key
     \param[in] value we want to set for this key
	 */
	void SetMatrix2(const String & key, Matrix2 &value);
    
    /**
     \brief Function to set variable in archive.
     \param[in] key string key
     \param[in] value we want to set for this key
	 */
	void SetMatrix3(const String & key, Matrix3 &value);
    
    /**
     \brief Function to set variable in archive.
     \param[in] key string key
     \param[in] value we want to set for this key
	 */
	void SetMatrix4(const String & key, Matrix4 &value);
    
    /**
        \brief Function to set value from template type to byte array.  
        This functionality is added to perform simple storage of complex types, like Vector3, Vector4, Matrix4 and others to byte arrays
        \param[in] key string key
        \param[in] value value we want to set for given key
     */
    template<class T>
    void SetByteArrayAsType(const String & key, const T & value);
	
	/**
		\brief Function loads data from given file.
		\param[in] pathName relative pathname in application documents folder
	 */
	bool Load(const String & pathName);
	/**
		\brief Function saves data to given file.
		\param[in] pathName relative pathname in application documents folder
	 */
	bool Save(const String & pathName);

	/**
        \brief Function loads data from given file.
        \param[in] file to load from
	 */
	bool Load(File *file);
	/**
        \brief Function saves data to given file.
        \param[in] file to save
	 */
	bool Save(File *file);
    
    /**
     \brief Function loads data from given yaml file.
     \param[in] pathName relative pathname in application documents folder
	 */
	bool LoadFromYamlFile(const String & pathName);

    /**
     \brief Function saves data to given yaml file.
     \param[in] file to save
	 */
	bool SaveToYamlFile(const String & pathName);

	/**
		\brief Deletes named key.
		\param[in] key name of the key to delete
	 */
	void DeleteKey(const String & key);

	/**
		\brief Deletes all keys, making archive empty.
	 */
	void DeleteAllKeys();

	uint32 Count(const String & key = "");
    
	/**
     \brief Function to get all data of archive.
     \returns map of VariantType class with names
	 */
    const Map<String, VariantType*> & GetArchieveData();

	/**
     \brief Function to get all data of archive.
     \returns map of VariantType class with names
	 */
    const Map<String, VariantType*> & GetArchieveData() const;
    
    /**
     \brief Function loads data from given yaml Node.
     \param[in] pathName relative pathname in application documents folder
	 */
	bool LoadFromYamlNode(YamlNode* rootNode);

//	yaml
// 	/**
// 		\brief this function loads data from given yaml file
// 		\param[in] pathName relative pathname in application documents folder
// 	*/
// 	bool LoadFromYaml(const String & pathName);
// 
// 	/**
// 		\brief this function saes data to given yaml file
// 		\param[in] pathName relative pathname in application documents folder
// 	*/
// 	bool SaveToYaml(const String & pathName);
    
	static const char* GenKeyFromIndex(uint32 index);


private:
	Map<String, VariantType*> objectMap;

public:
	INTROSPECTION_EXTEND(KeyedArchive, BaseObject, NULL);
};
    
// Implementation 

template <class T>
T KeyedArchive::GetByteArrayAsType(const String & key, const T & defaultValue)
{
    int size = GetByteArraySize(key);
    if (size != 0)
    {
        DVASSERT(size == sizeof(T));
 
        T value;
        const uint8 * arrayData = GetByteArray(key);
        memcpy(&value, arrayData, sizeof(T));
        return value;
    }else
    {
        return defaultValue;
    }
}
    
template<class T>
void KeyedArchive::SetByteArrayAsType(const String & key, const T & value)
{
    SetByteArray(key, (uint8 * )&value, sizeof(T));
}

};

#endif // __DAVAENGINE_KEYED_ARCHIVE_H__