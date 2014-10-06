#include "UIPackage.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlEmitter.h"
#include "FileSystem/YamlParser.h"
#include "UI/UIControl.h"

namespace DAVA
{

UIPackage::UIPackage(const FilePath &path)
    : packagePath(path)
{
}

UIPackage::~UIPackage()
{
    for (auto iter = controls.begin(); iter != controls.end(); ++iter)
        SafeRelease(*iter);
    controls.clear();
    
    for (auto it = importedPackages.begin(); it != importedPackages.end(); ++it)
        SafeRelease(*it);
    importedPackages.clear();
}

String UIPackage::GetName() const
{
    return packagePath.GetBasename();
}

DAVA::int32 UIPackage::GetControlsCount() const
{
    return controls.size();
}
    
UIControl * UIPackage::GetControl(const String &name) const
{
    Vector<UIControl *>::const_iterator iter = controls.begin();
    Vector<UIControl *>::const_iterator end = controls.end();
    for (; iter != end; ++iter)
    {
        if ((*iter)->GetName() == name)
            return *iter;
    }

    return NULL;
}

UIControl * UIPackage::GetControl(int32 index) const
{
    return controls[index];
}

void UIPackage::AddControl(UIControl *control)
{
    controls.push_back(SafeRetain(control));
}

void UIPackage::RemoveControl(UIControl *control)
{
    Vector<UIControl *>::iterator iter = std::find(controls.begin(), controls.end(), control);
    if (iter != controls.end())
    {
        SafeRelease(*iter);
        controls.erase(iter);
    }
}

int32 UIPackage::GetPackagesCount() const
{
    return (int32) importedPackages.size();
}

UIPackage *UIPackage::GetPackage(int32 index) const
{
    return importedPackages[index];
}

void UIPackage::AddPackage(UIPackage *package)
{
    importedPackages.push_back(SafeRetain(package));
}

}
