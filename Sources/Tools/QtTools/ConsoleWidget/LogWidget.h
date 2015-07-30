#ifndef __LOGWIDGET_H__
#define __LOGWIDGET_H__


#include <QWidget>
#include <QPointer>
#include "Base/Result.h"
#include "LogModel.h"

class QTimer;
class LogFilterModel;
class LogModel;

namespace Ui
{
    class LogWidget;
};

class LogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget* parent = NULL);
    ~LogWidget();
    void SetConvertFunction(LogModel::ConvertFunc func); //provide mechanism to convert data string to string to be displayed
    LogModel *Model() const;
    QByteArray Serialize() const;
    void Deserialize(const QByteArray &data);
    void AddMessage(DAVA::Logger::eLogLevel ll, const char* msg);
signals:
    void ItemClicked(const QString &data);
public slots:
    void AddResultList(const DAVA::ResultList &resultList);
private slots:
    void OnTextFilterChanged(const QString& text);
    void OnCopy();
    void OnClear();
    void OnBeforeAdded();
    void OnRowAdded();
    void OnItemClicked(const QModelIndex &index);
private:
    void FillFiltersCombo();
    bool eventFilter( QObject* watched, QEvent* event ) override;

    QPointer<LogModel> logModel;
    QPointer<LogFilterModel> logFilterModel;
    bool onBottom;
    Ui::LogWidget *ui;
};


#endif // __LOGWIDGET_H__
