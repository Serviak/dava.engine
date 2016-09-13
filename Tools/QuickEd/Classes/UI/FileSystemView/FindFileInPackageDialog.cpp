#include "UI/FileSystemView/FindFileInPackageDialog.h"
#include "ui_FindFileInPackageDialog.h"

#include <QHBoxLayout>
#include <QCompleter>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QApplication>
#include <QAbstractItemView>
#include <QKeyEvent>

FindFileInPackageDialog::FindFileInPackageDialog(const DAVA::String& rootPath, const DAVA::Vector<DAVA::String>& files, QWidget* parent)
    : QDialog(parent)
    , projectDir(QString::fromStdString(rootPath))
    , ui(new Ui::FindFileInPackageDialog())
{
    ui->setupUi(this);
    installEventFilter(this);

    Init(files);
}

QString FindFileInPackageDialog::GetFilePath(const DAVA::String& rootPath, const DAVA::Vector<DAVA::String>& files, QWidget* parent)
{
    FindFileInPackageDialog dialog(rootPath, files, parent);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString filePath = dialog.ui->lineEdit->text();
        if (dialog.availableFiles.contains(filePath))
        {
            return dialog.availableFiles[filePath];
        }
    }
    return QString();
}

void FindFileInPackageDialog::Init(const DAVA::Vector<DAVA::String>& files)
{
    QPair<int, QString> longestPath(-1, QString());
    for (const DAVA::String& filePath : files)
    {
        QString absPath = QString::fromStdString(filePath);
        QString relPath = absPath.right(absPath.size() - projectDir.size() - 1); //chop "/" symbol at begin of relPath
        int relPathSize = relPath.size();
        const int maxPathLen = 200;
        if (relPathSize > maxPathLen)
        {
            relPath.remove(0, relPathSize - maxPathLen);
            relPath.prepend("...");
        }
        relPathSize = relPath.size();
        if (relPathSize > longestPath.first)
        {
            longestPath = { relPathSize, relPath };
        }
        availableFiles.insert(relPath, absPath);
    }

    qApp->restoreOverrideCursor();
    //the only way to not create model and use stringlist is a pass stringlist to the QCompleter c-tor :(
    completer = new QCompleter(availableFiles.keys(), this);
    completer->setFilterMode(Qt::MatchContains);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->installEventFilter(this);
    completer->popup()->installEventFilter(this);

    ui->lineEdit->installEventFilter(this);
    QFontMetrics fm(ui->lineEdit->font());
    int width = fm.width(longestPath.second) + 50; //50px for scrollbar
    ui->lineEdit->setMinimumWidth(width);
    ui->lineEdit->setCompleter(completer);

    ui->lineEdit->setFocus();
}

bool FindFileInPackageDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Tab)
        {
            completer->complete();
            QAbstractItemView* popup = completer->popup();
            int currentRow = popup->currentIndex().row();
            if (currentRow < popup->model()->rowCount())
            {
                popup->setCurrentIndex(popup->model()->index(currentRow + 1, 0));
            }
            else
            {
                popup->setCurrentIndex(popup->model()->index(0, 0));
            }
            return true;
        }
        else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            accept();
        }
    }
    return QDialog::eventFilter(obj, event);
}
