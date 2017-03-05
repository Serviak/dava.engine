#include "UI/Find/Finder/Finder.h"
#include "UI/Find/PackageInformation/StaticPackageInformationBuilder.h"
#include "UI/Find/PackageInformation/ControlInformationHelpers.h"
#include "UI/Find/PackageInformation/ControlNodeInformation.h"
#include "UI/Find/PackageInformation/PackageNodeInformation.h"

#include <UI/UIPackageLoader.h>

using namespace DAVA;

Finder::Finder(std::shared_ptr<FindFilter> filter_, const DAVA::Map<DAVA::String, DAVA::Set<DAVA::FastName>>* prototypes_)
    : filter(filter_)
    , prototypes(prototypes_)
{
}

Finder::~Finder()
{
}

void Finder::Process(const QStringList& files)
{
    QStringList sortedFiles = files;
    sortedFiles.sort();

    PackageInformationCache packagesCache;

    FindItem currentItem;
    int filesProcessed = 0;
    for (const QString& pathStr : sortedFiles)
    {
        QMutexLocker locker(&mutex);
        if (cancelling)
        {
            break;
        }

        FilePath path(pathStr.toStdString());
        StaticPackageInformationBuilder builder(&packagesCache);

        if (UIPackageLoader(*prototypes).LoadPackage(path, &builder))
        {
            const std::shared_ptr<PackageInformation>& package = builder.GetPackage();

            ProcessPackage(currentItem, package.get());
        }
        filesProcessed++;
        emit ProgressChanged(filesProcessed, sortedFiles.size());
    }

    emit Finished();
}

void Finder::Process(const PackageNode* package)
{
    FindItem currentItem;

    PackageNodeInformation packageInfo(package);
    ProcessPackage(currentItem, &packageInfo);

    emit Finished();
}

void Finder::Process(const PackageNode* package, const SortedControlNodeSet& controlNodesToProcess)
{
    FindItem currentItem;

    PackageNodeInformation packageInfo(package);

    if (filter->CanAcceptPackage(&packageInfo))
    {
        currentItem = FindItem(packageInfo.GetPath());

        for (const ControlNode* controlNode : controlNodesToProcess)
        {
            ControlNodeInformation controlInfo(controlNode);
            CollectControls(currentItem, *filter, &controlInfo);
        }

        if (!currentItem.GetControlPaths().empty())
        {
            emit ItemFound(currentItem);
        }
    }

    emit Finished();
}

void Finder::Stop()
{
    QMutexLocker locker(&mutex);
    cancelling = true;
}

void Finder::CollectControls(FindItem& currentItem, const FindFilter& filter, const ControlInformation* control)
{
    if (filter.CanAcceptControl(control))
    {
        currentItem.AddPathToControl(ControlInformationHelpers::GetPathToControl(control));
    }

    control->VisitChildren(
    [&currentItem, &filter](const ControlInformation* child)
    {
        CollectControls(currentItem, filter, child);
    });
}

void Finder::ProcessPackage(FindItem& currentItem, const PackageInformation* package)
{
    if (filter->CanAcceptPackage(package))
    {
        currentItem = FindItem(package->GetPath());

        package->VisitControls(
        [this, &currentItem](const ControlInformation* control)
        {
            CollectControls(currentItem, *filter, control);
        });

        package->VisitPrototypes(
        [this, &currentItem](const ControlInformation* prototype)
        {
            CollectControls(currentItem, *filter, prototype);
        });

        if (!currentItem.GetControlPaths().empty())
        {
            emit ItemFound(currentItem);
        }
    }
}
