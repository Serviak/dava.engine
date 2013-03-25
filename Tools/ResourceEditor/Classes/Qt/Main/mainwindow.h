#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressDialog>
#include "Base/Singleton.h"
#include "QtPosSaver/QtPosSaver.h"
#include "ui_mainwindow.h"

class LibraryModel;
class QtMainWindow : public QMainWindow, public DAVA::Singleton<QtMainWindow>
{
    Q_OBJECT
    
public:
	explicit QtMainWindow(QWidget *parent = 0);
	~QtMainWindow();

	Ui::MainWindow* GetUI();
    
    virtual bool eventFilter(QObject *, QEvent *);
    
private:
	void OpenLastProject();

	void SetupActions();
    void SetupMainMenu();
	void SetupToolBars();
    void SetupDocks();

    void SetupCustomColorsDock();
	void SetupVisibilityToolDock();
    
    void SetCustomColorsDockControlsEnabled(bool enabled);

	void UpdateLibraryFileTypes();
	void UpdateLibraryFileTypes(bool showDAEFiles, bool showSC2Files);

public slots:
	void ShowActionWithText(QToolBar *toolbar, QAction *action, bool showText);


	void ChangeParticleDockVisible(bool visible);
	void ChangeParticleDockTimeLineVisible(bool visible);
	void returnToOldMaxMinSizesForDockSceneGraph();

	//return true if conversion has been started
	bool TextureCheckConvetAndWait(bool forceConvertAll = false);
	void UpdateParticleSprites();

	void RepackAndReloadScene();

private slots:
	void ProjectOpened(const QString &path);
    void MenuFileWillShow();
	void LibraryFileTypesChanged();
	
	//reference
	void ApplyReferenceNodeSuffix();
	void ConvertWaitDone(QObject *destroyed);
	void ConvertWaitStatus(const QString &curPath, int curJob, int jobCount);

	void RepackSpritesWaitDone(QObject *destroyed);

signals:
	// Library File Types.
	void LibraryFileTypesChanged(bool showDAEFiles, bool showSC2Files);
	void RepackAndReloadFinished();

private:
    Ui::MainWindow *ui;
	QtPosSaver posSaver;

	QProgressDialog *convertWaitDialog;
	QProgressDialog *repackSpritesWaitDialog;
    
	QSize oldDockSceneGraphMaxSize;
	QSize oldDockSceneGraphMinSize;

	bool emitRepackAndReloadFinished;
};


#endif // MAINWINDOW_H
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QtPosSaver/QtPosSaver.h"

namespace Ui {
class MainWindow;
}

class QtMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
   explicit QtMainWindow(QWidget *parent = 0);
   ~QtMainWindow();
    
    virtual bool eventFilter(QObject *, QEvent *);
    
private:
    void SetupMainMenu();
    
	void SetupToolBar();
	void DecorateWithIcon(QAction *decoratedAction, const QString &iconFilename);

    void OpenLastProject();
    void SetupDocks();
        
private slots:

    void MenuFileWillShow();
	
	//reference
	void ApplyReferenceNodeSuffix();
        
private:
    Ui::MainWindow *ui;
	QtPosSaver posSaver;
};


#endif // MAINWINDOW_H
