#include "EditorUIPackageBuilder.h"

#include "PackageHierarchy/PackageNode.h"
#include "PackageHierarchy/ImportedPackagesNode.h"
#include "PackageHierarchy/PackageControlsNode.h"
#include "UIControls/ControlProperties/ControlPropertiesSection.h"
#include "UIControls/ControlProperties/BackgroundPropertiesSection.h"
#include "UIControls/ControlProperties/InternalControlPropertiesSection.h"
#include "UIControls/ControlProperties/ValueProperty.h"
#include "UIControls/ControlProperties/LocalizedTextValueProperty.h"
#include "UIControls/PackageHierarchy/ControlNode.h"
#include "UIControls/PackageHierarchy/ControlPrototype.h"
#include "UI/UIPackage.h"

using namespace DAVA;

const String EXCEPTION_CLASS_UI_TEXT_FIELD = "UITextField";
const String EXCEPTION_CLASS_UI_LIST = "UIList";

EditorUIPackageBuilder::EditorUIPackageBuilder()
    : packageNode(NULL)
    , currentObject(NULL)
{
    
}

EditorUIPackageBuilder::~EditorUIPackageBuilder()
{
    SafeRelease(packageNode);
}

UIPackage * EditorUIPackageBuilder::FindInCache(const String &packagePath) const
{
    return nullptr;
}

RefPtr<UIPackage> EditorUIPackageBuilder::BeginPackage(const FilePath &packagePath)
{
    DVASSERT(packageNode == NULL);
    SafeRelease(packageNode);
    RefPtr<UIPackage> package(new UIPackage());
    packageNode = new PackageNode(package.Get(), packagePath);
    return package;
}

void EditorUIPackageBuilder::EndPackage()
{
    DVASSERT(packageNode != NULL);
}

RefPtr<UIPackage> EditorUIPackageBuilder::ProcessImportedPackage(const String &packagePath, AbstractUIPackageLoader *loader)
{
    // store state
    PackageNode *prevPackageNode = packageNode;
    DAVA::List<ControlDescr> prevControlsStack = controlsStack;
    
    DAVA::BaseObject *prevObj = currentObject;
    PropertiesSection *prevSect = currentSection;

    // clear state
    packageNode = NULL;
    controlsStack.clear();
    currentObject = NULL;
    currentSection = NULL;
    
    // load package
    RefPtr<UIPackage> result(loader->LoadPackage(packagePath));
    PackageControlsNode *controlsNode = SafeRetain(packageNode->GetPackageControlsNode());
    controlsNode->SetName(packageNode->GetName());
    SafeRelease(packageNode);
    
    prevPackageNode->GetImportedPackagesNode()->Add(controlsNode);
    SafeRelease(controlsNode);

    // restore state
    packageNode = prevPackageNode;
    controlsStack = prevControlsStack;
    currentObject = prevObj;
    currentSection = prevSect;

    return result;
}

UIControl *EditorUIPackageBuilder::BeginControlWithClass(const String &className)
{
    UIControl *control = ObjectFactory::Instance()->New<UIControl>(className);
    if (control && className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST)//TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }

    controlsStack.push_back(ControlDescr(ControlNode::CreateFromControl(control), true));
    return control;
}

UIControl *EditorUIPackageBuilder::BeginControlWithCustomClass(const String &customClassName, const String &className)
{
    UIControl *control = ObjectFactory::Instance()->New<UIControl>(className);
    control->SetCustomControlClassName(customClassName);
    if (control && className != EXCEPTION_CLASS_UI_TEXT_FIELD && className != EXCEPTION_CLASS_UI_LIST)//TODO: fix internal staticText for Win\Mac
    {
        control->RemoveAllControls();
    }

    controlsStack.push_back(ControlDescr(ControlNode::CreateFromControl(control), true));
    return control;
}

UIControl *EditorUIPackageBuilder::BeginControlWithPrototype(const String &packageName, const String &prototypeName, const String &customClassName, AbstractUIPackageLoader *loader)
{
    RefPtr<ControlPrototype> prototype;
    
    if (packageName.empty())
    {
        ControlNode *prototypeNode = packageNode->GetPackageControlsNode()->FindControlNodeByName(prototypeName);
        if (!prototypeNode)
        {
            if (loader->LoadControlByName(prototypeName))
                prototypeNode = packageNode->GetPackageControlsNode()->FindControlNodeByName(prototypeName);
        }

        if (prototypeNode)
            prototype = new ControlPrototype(prototypeNode);
    }
    else
    {
        PackageControlsNode *importedPackage = packageNode->GetImportedPackagesNode()->FindPackageControlsNodeByName(packageName);
        if (importedPackage)
        {
            ControlNode *prototypeNode = importedPackage->FindControlNodeByName(prototypeName);
            if (prototypeNode)
                prototype = new ControlPrototype(prototypeNode, importedPackage->GetPackagePath());
        }
    }
    DVASSERT(prototype);
    ControlNode *node = ControlNode::CreateFromPrototype(prototype.Get());
    node->GetControl()->SetCustomControlClassName(customClassName);
    controlsStack.push_back(ControlDescr(node, true));

    return node->GetControl();
}

UIControl *EditorUIPackageBuilder::BeginControlWithPath(const String &pathName)
{
    ControlNode *control = NULL;
    if (!controlsStack.empty())
    {
        control = controlsStack.back().node;
        Vector<String> controlNames;
        Split(pathName, "/", controlNames, false, true);
        for (Vector<String>::const_iterator iter = controlNames.begin(); iter!=controlNames.end(); ++iter)
        {
            control = control->FindByName(*iter);
            if (!control)
                break;
        }
    }

    controlsStack.push_back(ControlDescr(SafeRetain(control), false));

    if (!control)
        return NULL;

    return control->GetControl();
}

UIControl *EditorUIPackageBuilder::BeginUnknownControl(const YamlNode *node)
{
    DVASSERT(false);
    return NULL;
}

void EditorUIPackageBuilder::EndControl(bool isRoot)
{
    ControlNode *lastControl = SafeRetain(controlsStack.back().node);
    bool addToParent = controlsStack.back().addToParent;
    controlsStack.pop_back();
    
    if (addToParent)
    {
        if (controlsStack.empty() || isRoot)
            packageNode->GetPackageControlsNode()->Add(lastControl);
        else
            controlsStack.back().node->Add(lastControl);
    }
    SafeRelease(lastControl);
}

void EditorUIPackageBuilder::BeginControlPropertiesSection(const String &name)
{
    currentSection = controlsStack.back().node->GetPropertiesRoot()->GetControlPropertiesSection(name);
    currentObject = controlsStack.back().node->GetControl();
}

void EditorUIPackageBuilder::EndControlPropertiesSection()
{
    currentSection = NULL;
    currentObject = NULL;
}

UIControlBackground *EditorUIPackageBuilder::BeginBgPropertiesSection(int index, bool sectionHasProperties)
{
    ControlNode *node = controlsStack.back().node;
    BackgroundPropertiesSection *section = node->GetPropertiesRoot()->GetBackgroundPropertiesSection(index);
    if (section && sectionHasProperties)
    {
        if (section->GetBg() == NULL)
            section->CreateControlBackground();

        if (section->GetBg())
        {
            currentObject = section->GetBg();
            currentSection = section;
            return section->GetBg();
        }
    }
    
    return NULL;
}

void EditorUIPackageBuilder::EndBgPropertiesSection()
{
    currentSection = NULL;
    currentObject = NULL;
}

UIControl *EditorUIPackageBuilder::BeginInternalControlSection(int index, bool sectionHasProperties)
{
    ControlNode *node = controlsStack.back().node;
    InternalControlPropertiesSection *section = node->GetPropertiesRoot()->GetInternalControlPropertiesSection(index);
    if (section && sectionHasProperties)
    {
        if (section->GetInternalControl() == NULL)
            section->CreateInternalControl();
        
        if (section->GetInternalControl())
        {
            currentObject = section->GetInternalControl();
            currentSection = section;
            return section->GetInternalControl();
        }
    }
    
    return NULL;
}

void EditorUIPackageBuilder::EndInternalControlSection()
{
    currentSection = NULL;
    currentObject = NULL;
}

void EditorUIPackageBuilder::ProcessProperty(const InspMember *member, const VariantType &value)
{
    if (currentObject && currentSection && (member->Flags() & I_EDIT))
    {
        ValueProperty *property = currentSection->FindProperty(member);
        if (property && value.GetType() != VariantType::TYPE_NONE)
            property->SetValue(value);
    }
}

RefPtr<PackageNode> EditorUIPackageBuilder::GetPackageNode() const
{
    return DAVA::RefPtr<PackageNode>(SafeRetain(packageNode));
}

////////////////////////////////////////////////////////////////////////////////
// ControlDescr
////////////////////////////////////////////////////////////////////////////////
EditorUIPackageBuilder::ControlDescr::ControlDescr() : node(NULL), addToParent(false)
{
}

EditorUIPackageBuilder::ControlDescr::ControlDescr(ControlNode *node, bool addToParent) : node(node), addToParent(addToParent)
{
}

EditorUIPackageBuilder::ControlDescr::ControlDescr(const ControlDescr &descr)
{
    node = DAVA::SafeRetain(descr.node);
    addToParent = descr.addToParent;
}

EditorUIPackageBuilder::ControlDescr::~ControlDescr()
{
    DAVA::SafeRelease(node);
}

EditorUIPackageBuilder::ControlDescr &EditorUIPackageBuilder::ControlDescr::operator=(const ControlDescr &descr)
{
    DAVA::SafeRetain(descr.node);
    DAVA::SafeRelease(node);
    
    node = descr.node;
    addToParent = descr.addToParent;
    return *this;
}
