#include "FindResultsWidget.h"

#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "UI/Find/Finder/Finder.h"

#include <QtHelpers/HelperFunctions.h>

#include <QtConcurrent>
#include <QKeyEvent>

using namespace DAVA;

FindResultsWidget::FindResultsWidget(QWidget* parent)
    : QWidget(parent)
{
    qRegisterMetaType<FindItem>("FindItem");

    ui.setupUi(this);

    model = new QStandardItemModel();
    ui.treeView->setModel(model);
    connect(ui.treeView, &QTreeView::activated, this, &FindResultsWidget::OnActivated);
    ui.treeView->installEventFilter(this);
    ui.status->setVisible(false);
}

FindResultsWidget::~FindResultsWidget() = default;

void FindResultsWidget::Find(std::shared_ptr<FindFilter> filter, const ProjectData* projectData, const QStringList& files)
{
    Find(filter, projectData, [files](Finder* finder)
         {
             QtConcurrent::run(QtHelpers::InvokeInAutoreleasePool, [finder, files]() { finder->Process(files); });
         });
}

void FindResultsWidget::Find(std::shared_ptr<FindFilter> filter, const ProjectData* projectData, DocumentData* documentData)
{
    const PackageNode* package = documentData->GetPackageNode();
    Find(filter, projectData, [package](Finder* finder)
         {
             finder->Process(package);
         });
}

void FindResultsWidget::StopFind()
{
    if (finder)
    {
        finder->Stop();
    }
}

void FindResultsWidget::ClearResults()
{
    model->removeRows(0, model->rowCount());
}

void FindResultsWidget::OnItemFound(FindItem item)
{
    String fwPath = item.GetFile().GetFrameworkPath();
    QStandardItem* pathItem = new QStandardItem(fwPath.c_str());
    pathItem->setEditable(false);
    pathItem->setData(QString::fromStdString(fwPath), PACKAGE_DATA);
    model->appendRow(pathItem);

    for (const String& pathToControl : item.GetControlPaths())
    {
        QStandardItem* controlItem = new QStandardItem(QString::fromStdString(pathToControl));
        controlItem->setEditable(false);
        controlItem->setData(QString::fromStdString(fwPath), PACKAGE_DATA);
        controlItem->setData(QString::fromStdString(pathToControl), CONTROL_DATA);
        pathItem->appendRow(controlItem);
    }
}

void FindResultsWidget::OnProgressChanged(int filesProcessed, int totalFiles)
{
    ui.status->setVisible(true);
    ui.status->setText(QString("Find - %1%").arg(filesProcessed * 100 / totalFiles));
}

void FindResultsWidget::OnFindFinished()
{
    if (finder)
    {
        finder->deleteLater();
        finder = nullptr;
    }

    ui.status->setText(QString("Find - Finished"));
    ui.status->setVisible(false);
}

void FindResultsWidget::OnActivated(const QModelIndex& index)
{
    QString path = index.data(PACKAGE_DATA).toString();
    if (index.data(CONTROL_DATA).isValid())
    {
        QString control = index.data(CONTROL_DATA).toString();
        emit JumpToControl(FilePath(path.toStdString()), control.toStdString());
    }
    else
    {
        emit JumpToPackage(FilePath(path.toStdString()));
    }
}

void FindResultsWidget::Find(std::shared_ptr<FindFilter> filter, const ProjectData* projectData, const DAVA::Function<void(Finder*)>& finderAction)
{
    if (finder == nullptr)
    {
        ClearResults();

        setVisible(true);
        raise();

        finder = new Finder(std::move(filter), &(projectData->GetPrototypes()));

        connect(finder, &Finder::ProgressChanged, this, &FindResultsWidget::OnProgressChanged, Qt::QueuedConnection);
        connect(finder, &Finder::ItemFound, this, &FindResultsWidget::OnItemFound, Qt::QueuedConnection);
        connect(finder, &Finder::Finished, this, &FindResultsWidget::OnFindFinished, Qt::QueuedConnection);

        finderAction(finder);
    }
}

bool FindResultsWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            if (ui.treeView->selectionModel()->currentIndex().isValid())
            {
                OnActivated(ui.treeView->selectionModel()->currentIndex());
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}
