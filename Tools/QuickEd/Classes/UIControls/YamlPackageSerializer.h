#ifndef __YAML_PACKAGE_SERIALIZER_H__
#define __YAML_PACKAGE_SERIALIZER_H__

#include "PackageSerializer.h"

namespace DAVA {
    class YamlNode;
}

class YamlPackageSerializer : public PackageSerializer
{
public:
    YamlPackageSerializer();
    virtual ~YamlPackageSerializer();
    
    virtual void PutValue(const DAVA::String &name, const DAVA::VariantType &value) override;
    virtual void PutValue(const DAVA::String &name, const DAVA::String &value) override;
    virtual void PutValue(const DAVA::String &name, const DAVA::Vector<DAVA::String> &value) override;
    virtual void PutValue(const DAVA::String &value) override;
    
    virtual void BeginMap() override;
    virtual void BeginMap(const DAVA::String &name) override;
    virtual void EndMap() override;
    
    virtual void BeginArray() override;
    virtual void BeginArray(const DAVA::String &name) override;
    virtual void EndArray() override;
    
    DAVA::YamlNode *GetYamlNode() const;
    void WriteToFile(const DAVA::FilePath &path);
    DAVA::String WriteToString();
    
private:
    DAVA::Vector<DAVA::YamlNode*> nodesStack;
};


#endif // __YAML_PACKAGE_SERIALIZER_H__
