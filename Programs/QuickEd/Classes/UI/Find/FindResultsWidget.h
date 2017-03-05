#pragma once

#include <QWidget>
#include <QPointer>
#include <QStandardItemModel>
#include "ui_FindResultsWidget.h"

#include "UI/Find/FindItem.h"
#include "UI/Find/FindFilter.h"
#include "Base/BaseTypes.h"

class DocumentData;
class ProjectData;
class FileSystemCache;
class Finder;

class FindResultsWidget : public QWidget
{
    Q_OBJECT
public:
    FindResultsWidget(QWidget* parent = nullptr);
    ~FindResultsWidget() override;

    void Find(std::shared_ptr<FindFilter> filter, ProjectData* projectData, const QStringList& files);
    void Find(std::shared_ptr<FindFilter> filter, ProjectData* projectData, DocumentData* documentData);

    void StopFind();
    void ClearResults();

signals:
    void JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName);
    void JumpToPackage(const DAVA::FilePath& packagePath);

private slots:
    void OnItemFound(FindItem item);
    void OnProgressChanged(int filesProcessed, int totalFiles);
    void OnFindFinished();
    void OnActivated(const QModelIndex& index);

private:
    bool eventFilter(QObject* obj, QEvent* event) override;

    enum
    {
        PACKAGE_DATA = Qt::UserRole + 1,
        CONTROL_DATA
    };

    Ui::FindResultsWidget ui;
    std::unique_ptr<FindFilter> filter;
    QStandardItemModel* model = nullptr;

    Finder* finder = nullptr;
};
