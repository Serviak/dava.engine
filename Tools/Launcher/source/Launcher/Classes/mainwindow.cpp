#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logger.h"
#include <QMessageBox>
#include <QComboBox>
#include <QSet>
#include "processhelper.h"
#include "settings.h"

#define LAUNCER_VER "0.7"

#define COLUMN_NAME 0
#define COLUMN_CUR_VER 1
#define COLUMN_NEW_VER 2

#define ST_TAB 0
#define DEV_TAB 1
#define DEP_TAB 2

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("Launcher %1").arg(QString(LAUNCER_VER)));

    m_pInstaller = new Installer(this);
    connect(m_pInstaller, SIGNAL(SoftWareAvailable(AvailableSoftWare)), this, SLOT(AvailableSoftWareUpdated(AvailableSoftWare)));
    connect(m_pInstaller, SIGNAL(StartDownload()), this, SLOT(OnDownloadStarted()));
    connect(m_pInstaller, SIGNAL(DownloadFinished()), this, SLOT(OnDownloadFinished()));
    connect(m_pInstaller, SIGNAL(DownloadProgress(int)), this, SLOT(OnDownloadProgress(int)));

    m_pUpdateTimer = new QTimer(this);
    connect(m_pUpdateTimer, SIGNAL(timeout()), this, SLOT(on_btnRefresh_clicked()));
    m_pUpdateTimer->start(Settings::GetInstance()->GetUpdateTimerInterval());

    ui->stableTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->developmentTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->dependenciesTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    connect(ui->stableTable, SIGNAL(itemSelectionChanged()), this, SLOT(UpdateBtn()));
    connect(ui->dependenciesTable, SIGNAL(itemSelectionChanged()), this, SLOT(UpdateBtn()));
    connect(ui->developmentTable, SIGNAL(itemSelectionChanged()), this, SLOT(UpdateBtn()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(UpdateBtn()));

    connect(ui->stableTable, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_btnRun_clicked()));
    connect(ui->developmentTable, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_btnRun_clicked()));

    connect(Logger::GetInstance(), SIGNAL(LogAdded(QString)), this, SLOT(OnLogAdded(QString)));

    ui->downloadProgress->setVisible(false);
    ui->downloadProgress->setRange(0, 100);

    m_bBusy = false;
    UpdateBtn();
    m_pInstaller->Init();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::AvailableSoftWareUpdated(const AvailableSoftWare& software) {
    m_SoftWare = software;
    FillTableSoft(ui->stableTable, software.m_Stable);
    FillTableSoft(ui->developmentTable, software.m_Development);
    FillTableDependencies(ui->dependenciesTable, software.m_Dependencies);
}

void MainWindow::OnComboBoxValueChanged(const QString& value) {
    m_SelectedAppVersion = value;
    UpdateBtn();
}

void MainWindow::FillTableSoft(QTableWidget* table, const AvailableSoftWare::SoftWareMap& soft) {
    AvailableSoftWare::SoftWareMap::const_iterator iter;
    table->setRowCount(0);
    for (iter = soft.begin(); iter != soft.end(); ++iter) {
        const QString& name = iter.key();
        const SoftWare& config = iter.value();

        int i = table->rowCount();
        table->insertRow(i);
        table->setItem(i, COLUMN_NAME, new QTableWidgetItem(name));
        table->setItem(i, COLUMN_CUR_VER, new QTableWidgetItem(config.m_CurVersion));
        if (config.m_AvailableVersion.size() == 1) {
            table->setItem(i, COLUMN_NEW_VER, new QTableWidgetItem(*(config.m_AvailableVersion.begin())));
        } else {
            QComboBox* pComboBox = new QComboBox(table);
            connect(pComboBox,
                    SIGNAL(currentIndexChanged(QString)),
                    this,
                    SLOT(OnComboBoxValueChanged(QString)));

            for (QSet<QString>::const_iterator iter = config.m_AvailableVersion.begin();
                 iter != config.m_AvailableVersion.end();
                 ++iter) {
                pComboBox->addItem(*iter);
            }
            pComboBox->setCurrentIndex(pComboBox->count() - 1);
            table->setCellWidget(i, COLUMN_NEW_VER, pComboBox);
        }
    }
    table->resizeRowsToContents();
}

void MainWindow::FillTableDependencies(QTableWidget* table, const AvailableSoftWare::SoftWareMap& soft) {
    AvailableSoftWare::SoftWareMap::const_iterator iter;
    table->setRowCount(0);
    for (iter = soft.begin(); iter != soft.end(); ++iter) {
        const QString& name = iter.key();
        const SoftWare& config = iter.value();

        int i = table->rowCount();
        table->insertRow(i);
        table->setItem(i, COLUMN_NAME, new QTableWidgetItem(name));
        if (config.m_CurVersion.isEmpty())
            table->setItem(i, COLUMN_CUR_VER, new QTableWidgetItem("Not installed"));
        else if (config.m_CurVersion == config.m_NewVersion)
            table->setItem(i, COLUMN_CUR_VER, new QTableWidgetItem("Installed latest"));
        else
            table->setItem(i, COLUMN_CUR_VER, new QTableWidgetItem("Installed not latest"));
    }
    table->resizeRowsToContents();
}

void MainWindow::UpdateSelectedApp(QTableWidget* table) {
    QList<QTableWidgetItem *> items = table->selectedItems();
    if (!items.size())
        return;

    for (int i = 0; i < items.size(); i++) {
        if (items[i]->column() == COLUMN_NAME)
            m_SelectedApp = items[i]->text();
        else if (items[i]->column() == COLUMN_NEW_VER)
            m_SelectedAppVersion = items[i]->text();
    }
    if (m_SelectedAppVersion.isEmpty()) {
        QWidget* pWidget = table->cellWidget(items[0]->row(), COLUMN_NEW_VER);
        QComboBox* pComboBox = dynamic_cast<QComboBox*>(pWidget);
        if (pComboBox)
            m_SelectedAppVersion = pComboBox->currentText();
    }
}

void MainWindow::UpdateBtn() {
    //update buttons
    ui->btnRemove->setEnabled(false);
    //ui->btnInstall->setEnabled(false);
    ui->btnInstall->setVisible(false);
    ui->btnRun->setVisible(false);
    ui->btnReinstall->setVisible(false);
    ui->btnCancel->setVisible(false);

    if (m_bBusy) {
        ui->btnCancel->setVisible(true);
        return;
    }

    m_SelectedApp.clear();
    m_SelectedAppVersion.clear();

    const AvailableSoftWare::SoftWareMap* pSelectedMap = NULL;
    switch (ui->tabWidget->currentIndex()) {
    case ST_TAB: {
        pSelectedMap = &m_SoftWare.m_Stable;
        m_SelectedAppType = eAppTypeStable;
        UpdateSelectedApp(ui->stableTable);
    }break;
    case DEV_TAB: {
        pSelectedMap = &m_SoftWare.m_Development;
        m_SelectedAppType = eAppTypeDevelopment;
        UpdateSelectedApp(ui->developmentTable);
    }break;
    case DEP_TAB: {
        pSelectedMap = &m_SoftWare.m_Dependencies;
        m_SelectedAppType = eAppTypeDependencies;
        UpdateSelectedApp(ui->dependenciesTable);
    }break;
    default:
        return;
    }

    if (pSelectedMap == NULL || m_SelectedApp.isEmpty())
        return;

    if (pSelectedMap->value(m_SelectedApp).m_CurVersion.isEmpty()) {
        //install
        ui->btnInstall->setVisible(true);
    } else {
        ui->btnInstall->setVisible(false);
        ui->btnRemove->setEnabled(true);
        ui->btnRun->setEnabled(!pSelectedMap->value(m_SelectedApp).m_RunPath.isEmpty());

        switch (ui->tabWidget->currentIndex())
        {
        case DEV_TAB: {
            if (pSelectedMap->value(m_SelectedApp).m_CurVersion != m_SelectedAppVersion)
            {
                ui->btnInstall->setEnabled(true);
                ui->btnInstall->setVisible(true);
            }
        };
        case ST_TAB: {
            ui->btnRun->setVisible(true);
        } break;
        case DEP_TAB: {
            ui->btnReinstall->setVisible(true);
        }break;
        }

        /*if (ui->tabWidget->currentIndex() != DEP_TAB)
            ui->btnRun->setVisible(true);
        else
            ui->btnReinstall->setVisible(true);
        ui->btnRemove->setEnabled(true);
        ui->btnRun->setEnabled(!pSelectedMap->value(m_SelectedApp).m_RunPath.isEmpty());*/
    }
}

void MainWindow::OnLogAdded(const QString &log) {
    ui->label->setText(log);
    ui->listWidget->addItem(log);
}

void MainWindow::on_btnInstall_clicked() {
    m_pInstaller->Install(m_SelectedApp, m_SelectedAppVersion, m_SelectedAppType);
}

void MainWindow::on_btnRefresh_clicked() {
    m_pInstaller->CheckForUpdate();
}

void MainWindow::on_btnCancel_clicked() {
    m_pInstaller->AbortCurInstallation();
}

void MainWindow::on_btnRemove_clicked() {
    if (0 == QMessageBox::information(this,
                                     tr("Confirmation"),
                                     tr("Are you sure you want to remove %1.").arg(m_SelectedApp),
                                     tr("ok"),
                                     tr("Cancel"))) {
        m_pInstaller->Delete(m_SelectedApp, m_SelectedAppType);
    }
}

void MainWindow::on_btnRun_clicked() {
    QString path = m_pInstaller->GetRunPath(m_SelectedApp, m_SelectedAppType);
    if (ProcessHelper::IsProcessRuning(path)) {
        ProcessHelper::SetActiveProcess(path);
    } else {
        int lastPos = path.lastIndexOf('/');
        QString workingDir = path.left(lastPos);
        QProcess::startDetached(path, QStringList(), workingDir);
    }
}

void MainWindow::on_btnReinstall_clicked() {
    if (m_pInstaller->Delete(m_SelectedApp, m_SelectedAppType)) {
        m_pInstaller->Install(m_SelectedApp, m_SelectedAppVersion, m_SelectedAppType);
    }
}

void MainWindow::OnDownloadStarted() {
    m_bBusy = true;
    ui->downloadProgress->setValue(0);
    ui->downloadProgress->setVisible(true);
    UpdateBtn();
}

void MainWindow::OnDownloadProgress(int nPercent) {
    ui->downloadProgress->setValue(nPercent);
}

void MainWindow::OnDownloadFinished() {
    m_bBusy = false;
    ui->downloadProgress->setVisible(false);
    UpdateBtn();
}
