#ifndef __EDITOR_UI_PACKAGE_BUILDER_H__
#define __EDITOR_UI_PACKAGE_BUILDER_H__

#include "DAVAEngine.h"
#include "UI/AbstractUIPackageBuilder.h"

class PackageNode;
class ControlNode;
class PropertiesSection;
class PackageCommandExecutor;

class EditorUIPackageBuilder : public DAVA::AbstractUIPackageBuilder
{
public:
    EditorUIPackageBuilder(PackageNode *basePackage = nullptr, ControlNode *insertingTarget = nullptr, PackageCommandExecutor *executor = nullptr);
    virtual ~EditorUIPackageBuilder();
     
    virtual DAVA::RefPtr<DAVA::UIPackage> BeginPackage(const DAVA::FilePath &packagePath) override;
    virtual void EndPackage() override;
    
    virtual DAVA::RefPtr<DAVA::UIPackage> ProcessImportedPackage(const DAVA::String &packagePath, DAVA::AbstractUIPackageLoader *loader) override;
    
    virtual DAVA::UIControl *BeginControlWithClass(const DAVA::String &className) override;
    virtual DAVA::UIControl *BeginControlWithCustomClass(const DAVA::String &customClassName, const DAVA::String &className) override;
    virtual DAVA::UIControl *BeginControlWithPrototype(const DAVA::String &packageName, const DAVA::String &prototypeName, const DAVA::String &customClassName, DAVA::AbstractUIPackageLoader *loader) override;
    virtual DAVA::UIControl *BeginControlWithPath(const DAVA::String &pathName) override;
    virtual DAVA::UIControl *BeginUnknownControl(const DAVA::YamlNode *node) override;
    virtual void EndControl(bool isRoot) override;
    
    virtual void BeginControlPropertiesSection(const DAVA::String &name) override;
    virtual void EndControlPropertiesSection() override;
    
    virtual DAVA::UIControlBackground *BeginBgPropertiesSection(int index, bool sectionHasProperties) override;
    virtual void EndBgPropertiesSection() override;
    
    virtual DAVA::UIControl *BeginInternalControlSection(int index, bool sectionHasProperties) override;
    virtual void EndInternalControlSection() override;
    
    virtual void ProcessProperty(const DAVA::InspMember *member, const DAVA::VariantType &value) override;

    DAVA::RefPtr<PackageNode> GetPackageNode() const;

private:
    struct ControlDescr {
        ControlNode *node;
        bool addToParent;

        ControlDescr();
        ControlDescr(ControlNode *node, bool addToParent);
        ControlDescr(const ControlDescr &descr);
        ~ControlDescr();
        ControlDescr &operator=(const ControlDescr &descr);
    };
    
private:
    PackageNode *packageNode;
    PackageNode *basePackage;
    ControlNode *insertingTarget;
    
    DAVA::List<ControlDescr> controlsStack;
    DAVA::BaseObject *currentObject;
    PropertiesSection *currentSection;
    
    PackageCommandExecutor *commandExecutor;
};

#endif // __EDITOR_UI_PACKAGE_BUILDER_H__
