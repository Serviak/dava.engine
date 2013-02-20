#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressDialog>
#include "Base/Singleton.h"
#include "QtPosSaver/QtPosSaver.h"

namespace Ui {
class MainWindow;
}

class LibraryModel;
class QtMainWindow : public QMainWindow, public DAVA::Singleton<QtMainWindow>
{
    Q_OBJECT
    
public:
   explicit QtMainWindow(QWidget *parent = 0);
   ~QtMainWindow();
    
    virtual bool eventFilter(QObject *, QEvent *);

    
private:
	void OpenLastProject();

    void SetupMainMenu();
	void SetupToolBar();
	void SetupModificationToolBar();
    void SetupDockWidgets();
    void SetupCustomColorsDock();
	void SetupVisibilityToolDock();
    
	void DecorateWithIcon(QAction *decoratedAction, const QString &iconFilename);
    void SetCustomColorsDockControlsEnabled(bool enabled);

	void UpdateLibraryFileTypes();
	void UpdateLibraryFileTypes(bool showDAEFiles, bool showSC2Files);

public slots:
	void TextureCheckConvetAndWait(bool forceConvertAll = false);
	void ChangeParticleDockVisible(bool visible);
	void ChangeParticleDockTimeLineVisible(bool visible);

	void UpdateParticleSprites();

	void returnToOldMaxMinSizesForDockSceneGraph();

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

private:
    Ui::MainWindow *ui;
	QtPosSaver posSaver;

	QProgressDialog *convertWaitDialog;
	QProgressDialog *repackSpritesWaitDialog;
    
    //LibraryModel *libraryModel;
	
	QSize oldDockSceneGraphMaxSize;
	QSize oldDockSceneGraphMinSize;

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
    void SetupDockWidgets();
        
private slots:

    void MenuFileWillShow();
	
	//reference
	void ApplyReferenceNodeSuffix();
        
private:
    Ui::MainWindow *ui;
	QtPosSaver posSaver;
};


#endif // MAINWINDOW_H
