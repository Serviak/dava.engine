#include "KeyedArchiveYamlTest.h"
#include "Math/Math2D.h"

#define FILE_PATH "~res:/KeyedArchives/keyed_archive_original.yaml"
#define GENERATED_FILE_PATH "KeyedArchives/keyed_archive_created.yaml"
//#define GENERATED_FILE_PATH "~res:/KeyedArchives/keyed_archive_created.yaml"
//#define GENERATED_FILE_PATH "/Users/user/Documents/work/gitHub/dava.framework/Projects/UnitTests/Data/KeyedArchives/keyed_archive_created.yaml"

#define BOOLMAPID       "mapNamebool"
#define INT32MAPID      "mapNameint32"
#define UINT32MAPID     "mapNameUInt32"
#define FLOATMAPID      "mapNamefloat"
#define STRINGMAPID     "mapNameString"
#define WSTRINGMAPID    "mapNameWideString"
#define BYTEARRMAPID    "mapNameByteArrey"
#define INT64MAPID      "mapNameint64"
#define UINT64MAPID     "mapNameUInt64"
#define VECTOR2MAPID    "mapNamevector2"
#define VECTOR3MAPID    "mapNameVector3"
#define VECTOR4MAPID    "mapNameVector4"
#define MATRIX2MAPID    "mapNameMatrix2"
#define MATRIX3MAPID    "mapNameMatrix3"
#define MATRIX4MAPID    "mapNameMatrix4"
#define KEYEDARCHMAPID  "mapNameKArch"

KeyedArchiveYamlTest::KeyedArchiveYamlTest()
: TestTemplate<KeyedArchiveYamlTest>("KeyedArchiveYamlTest")
{
	RegisterFunction(this, &KeyedArchiveYamlTest::PerformTest, "PerformTest", NULL);
}

void KeyedArchiveYamlTest::LoadResources()
{
}


void KeyedArchiveYamlTest::UnloadResources()
{
}


void KeyedArchiveYamlTest::PerformTest(PerfFuncData * data)
{
    bool loaded = false;
    
    loadedArchive.DeleteAllKeys();
    
    loaded = loadedArchive.LoadFromYamlFile(FILE_PATH);
    TEST_VERIFY(false != loaded);
    
    String documentsPath = FileSystem::Instance()->GetCurrentDocumentsDirectory();
    String generatedPath = documentsPath + GENERATED_FILE_PATH;

    String folder, filename;
    FileSystem::Instance()->SplitPath(generatedPath, folder, filename);
    FileSystem::Instance()->CreateDirectory(folder, true);
    
    loadedArchive.SaveToYamlFile(generatedPath);
    
    KeyedArchive loadedArchiveFromGeneratedFile;
    loaded = loadedArchiveFromGeneratedFile.LoadFromYamlFile(generatedPath);
    
    TEST_VERIFY(false != loaded);
    
    TEST_VERIFY(*loadedArchive.GetVariant(BOOLMAPID) == *loadedArchiveFromGeneratedFile.GetVariant(BOOLMAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(INT32MAPID) == *loadedArchiveFromGeneratedFile.GetVariant(INT32MAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(UINT32MAPID) == *loadedArchiveFromGeneratedFile.GetVariant(UINT32MAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(FLOATMAPID) == *loadedArchiveFromGeneratedFile.GetVariant(FLOATMAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(STRINGMAPID) == *loadedArchiveFromGeneratedFile.GetVariant(STRINGMAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(WSTRINGMAPID) == *loadedArchiveFromGeneratedFile.GetVariant(WSTRINGMAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(BYTEARRMAPID) == *loadedArchiveFromGeneratedFile.GetVariant(BYTEARRMAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(INT64MAPID) == *loadedArchiveFromGeneratedFile.GetVariant(INT64MAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(UINT64MAPID) == *loadedArchiveFromGeneratedFile.GetVariant(UINT64MAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(VECTOR2MAPID) == *loadedArchiveFromGeneratedFile.GetVariant(VECTOR2MAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(VECTOR3MAPID) == *loadedArchiveFromGeneratedFile.GetVariant(VECTOR3MAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(VECTOR4MAPID) == *loadedArchiveFromGeneratedFile.GetVariant(VECTOR4MAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(MATRIX2MAPID) == *loadedArchiveFromGeneratedFile.GetVariant(MATRIX2MAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(MATRIX3MAPID) == *loadedArchiveFromGeneratedFile.GetVariant(MATRIX3MAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(MATRIX4MAPID) == *loadedArchiveFromGeneratedFile.GetVariant(MATRIX4MAPID));
    TEST_VERIFY(*loadedArchive.GetVariant(KEYEDARCHMAPID) == *loadedArchiveFromGeneratedFile.GetVariant(KEYEDARCHMAPID));   
}


