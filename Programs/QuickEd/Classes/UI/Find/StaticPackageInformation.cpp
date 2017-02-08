#include "StaticPackageInformation.h"

#include "Debug/DVAssert.h"

using namespace DAVA;

StaticPackageInformation::StaticPackageInformation(const String& path_)
    : path(path_)
{
}

String StaticPackageInformation::GetPath() const
{
    return path;
}

void StaticPackageInformation::VisitImportedPackages(const Function<void(const PackageInformation*)>& visitor) const
{
    for (const std::shared_ptr<PackageInformation>& importedPackage : importedPackages)
    {
        visitor(importedPackage.get());
    }
}

void StaticPackageInformation::VisitControls(const Function<void(const ControlInformation*)>& visitor) const
{
    for (const std::shared_ptr<ControlInformation>& control : controls)
    {
        visitor(control.get());
    }
}

void StaticPackageInformation::VisitPrototypes(const Function<void(const ControlInformation*)>& visitor) const
{
    for (const std::shared_ptr<ControlInformation>& prototype : prototypes)
    {
        visitor(prototype.get());
    }
}

void StaticPackageInformation::AddImportedPackage(const std::shared_ptr<StaticPackageInformation>& package)
{
    importedPackages.push_back(package);
}

void StaticPackageInformation::AddControl(const std::shared_ptr<StaticControlInformation>& control)
{
    controls.push_back(control);
}

void StaticPackageInformation::AddPrototype(const std::shared_ptr<StaticControlInformation>& prototype)
{
    prototypes.push_back(prototype);
}

const Vector<std::shared_ptr<StaticPackageInformation>>& StaticPackageInformation::GetImportedPackages() const
{
    return importedPackages;
}

const Vector<std::shared_ptr<StaticControlInformation>>& StaticPackageInformation::GetPrototypes() const
{
    return prototypes;
}

const Vector<std::shared_ptr<StaticControlInformation>>& StaticPackageInformation::GetControls() const
{
    return controls;
}

std::shared_ptr<StaticControlInformation> StaticPackageInformation::FindPrototypeByName(const FastName& name) const
{
    for (const std::shared_ptr<StaticControlInformation>& prototype : prototypes)
    {
        if (prototype->GetName() == name)
        {
            return prototype;
        }
    }
    return std::shared_ptr<StaticControlInformation>();
}

void PackageInformationCache::Put(const std::shared_ptr<StaticPackageInformation>& package)
{
    DVASSERT(packages.find(package->GetPath()) == packages.end());
    packages[package->GetPath()] = package;
}

std::shared_ptr<StaticPackageInformation> PackageInformationCache::Find(const String& path)
{
    auto it = packages.find(path);
    if (it != packages.end())
    {
        return it->second;
    }

    return std::shared_ptr<StaticPackageInformation>();
}
