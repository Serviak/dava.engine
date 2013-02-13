#ifndef __Q_RESOURSE_EDITOR_PROGRESS_DIALOG_H__
#define __Q_RESOURSE_EDITOR_PROGRESS_DIALOG_H__

#include <QtGui/qprogressdialog.h>

class QProgressBar;

class  QResourceEditorProgressDialog : public QProgressDialog
{

	Q_OBJECT

public:

	QResourceEditorProgressDialog( QWidget * parent = 0, Qt::WindowFlags f = 0, bool isCycled = false );

	bool getCycledFlag()
	{ 
		return isCycled;
	}

	unsigned int getTimeIntervalForParcent() 
	{ 
		return timeIntervalForParcent;
	}
	
	void getTimeIntervalForParcent(unsigned int value) 
	{ 
		timeIntervalForParcent = value;
	}

protected:

	virtual void	showEvent ( QShowEvent * e );

	bool				isCycled;
	unsigned int		timeIntervalForParcent;

private slots:
	
	void UpdateProgress();
};

#endif // __Q_RESOURSE_EDITOR_PROGRESS_DIALOG_H__