#ifndef __LOGWIDGET_H__
#define __LOGWIDGET_H__


#include <QWidget>
#include <QPointer>
#include <QScopedPointer>


namespace Ui
{
    class LogWidget;
};


class LogModel;
class LogFilterModel;

class LogWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget *parent = NULL);
    ~LogWidget();

private slots:
    void OnFilterChanged();
    void OnTextFilterChanged(const QString& text);
    void OnCopy();

private:
    void FillFiltersCombo();
    void LoadSettings();
    void SaveSettings();
    bool eventFilter( QObject * watched, QEvent * event );

    QScopedPointer<Ui::LogWidget> ui;
    QPointer<LogModel> logModel;
    QPointer<LogFilterModel> logFilterModel;
};



#endif // __LOGWIDGET_H__
