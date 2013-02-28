#include "QtPosSaver/QtPosSaver.h"
#include <QHeaderView>

bool QtPosSaver::settingsArchiveIsLoaded = false;
DAVA::KeyedArchive QtPosSaver::settingsArchive;

QtPosSaver::QtPosSaver()
	: attachedWidget(NULL)
{
	if(!settingsArchiveIsLoaded)
	{
		settingsArchiveIsLoaded = true;
		settingsArchive.Load("~doc:/ResourceEditorPos.archive");
	}
	else
	{
		settingsArchive.Retain();
	}
}

QtPosSaver::~QtPosSaver()
{
	if(settingsArchiveIsLoaded)	
	{
		if(NULL != attachedWidget && !attachedWidgetName.isEmpty())
		{
			SaveGeometry(attachedWidget);
		}

		if(1 == settingsArchive.GetRetainCount())
		{
			settingsArchive.Save("~doc:/ResourceEditorPos.archive");
		}
		else
		{
			settingsArchive.Release();
		}
	}
}

void QtPosSaver::Attach(QWidget *widget, const QString &name)
{
	attachedWidget = widget;

	if(NULL != attachedWidget)
	{
		if(name.isEmpty())
		{
			attachedWidgetName = attachedWidget->objectName();
		}
		else
		{
			attachedWidgetName = name;
		}

		LoadGeometry(attachedWidget);
	}
}

void QtPosSaver::SaveGeometry(QWidget *widget)
{
	if(NULL != widget && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-geometry-" + widget->objectName();
		Save(key, widget->saveGeometry());

		key = attachedWidgetName + "-maximized-" + widget->objectName();
		QByteArray mState(1, (char) widget->isMaximized());
		Save(key, mState);
	}
}

void QtPosSaver::LoadGeometry(QWidget *widget)
{
	if(NULL != widget && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-maximized-" + widget->objectName();
		QByteArray mState = Load(key);
		if(!mState.isEmpty() && mState.at(0))
		{
			widget->showMaximized();
		}

		key = attachedWidgetName + "-geometry-" + widget->objectName();
		widget->restoreGeometry(Load(key));
	}
}

void QtPosSaver::SaveState(QSplitter *splitter)
{
	if(NULL != splitter && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-splitter-" + splitter->objectName();
		Save(key, splitter->saveState());
	}
}

void QtPosSaver::LoadState(QSplitter *splitter)
{
	if(NULL != splitter && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-splitter-" + splitter->objectName();
		splitter->restoreState(Load(key));
	}
}

void QtPosSaver::SaveState(QMainWindow *mainwindow)
{
	if(NULL != mainwindow && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-mainwindow-" + mainwindow->objectName();
		Save(key, mainwindow->saveState());
	}
}

void QtPosSaver::LoadState(QMainWindow *mainwindow)
{
	if(NULL != mainwindow && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-mainwindow-" + mainwindow->objectName();
		mainwindow->restoreState(Load(key));
	}
}

void QtPosSaver::SaveState(QTreeView *treeView)
{
	if(NULL != treeView && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-treeview-header" + treeView->objectName();
		QByteArray arr = treeView->header()->saveState();
		Save(key, arr);
	}
}

void QtPosSaver::LoadState(QTreeView *treeView)
{
	if(NULL != treeView && !attachedWidgetName.isEmpty())
	{
		QString key = attachedWidgetName + "-treeview-header" + treeView->objectName();
		QByteArray arr = Load(key);
		treeView->header()->restoreState(arr);
	}
}

void QtPosSaver::Save(const QString &key, const QByteArray &data)
{
	if(settingsArchiveIsLoaded && !key.isEmpty() && !data.isEmpty())
	{
		settingsArchive.SetByteArray(key.toStdString(), (const DAVA::uint8 *) data.constData(), data.size());
	}
}

QByteArray QtPosSaver::Load(const QString &key)
{
	QByteArray data;

	if(settingsArchiveIsLoaded && !key.isEmpty())
	{
		int sz = settingsArchive.GetByteArraySize(key.toStdString());
		const DAVA::uint8 *dt = settingsArchive.GetByteArray(key.toStdString());

		if(NULL != dt)
		{
			data.append((const char *) dt, sz);
		}
	}

	return data;
}