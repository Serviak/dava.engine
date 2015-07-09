/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

#include "ApplicationSettings.h"
#include "RemoteAssetCacheServer.h"

class QMenu;
class QVBoxLayout;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void SetSettings(ApplicationSettings *settings);

protected:
    void closeEvent(QCloseEvent *e) override;
    void SaveSettings();

private slots:
    void OnAddNewServerWidget();
    void OnRemoveServerWidget();
    
    void OnSelectFolder();

    void CheckEnableClearButton();
    void OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void OnServerParametersChanged();
    void OnOpenAction();

    void OnSaveButtonClicked();
    void OnCancelButtonClicked();

    void SetFolder(QString &folderPath);
    void SetFolderSize(qreal folderSize);
    void SetFilesCount(quint32 filesCounts);
    void AddServers(QVector<ServerData> &newServers);
    void AddServer(const ServerData & newServer);
    
private:
    void CreateTrayIconActions();
    void CreateTrayIcon();
    void VerifyData();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    QMenu *trayActionsMenu;

    QList<RemoteAssetCacheServer *> servers;

    QVBoxLayout *boxLayout;
    
    ApplicationSettings *settings;
};

#endif // MAINWINDOW_H
