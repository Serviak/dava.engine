#ifndef __LOGMODEL_H__
#define __LOGMODEL_H__


#include <QObject>
#include <QStandardItemModel>
#include <QMap>


#include "FileSystem/Logger.h"


class LogModel
    : public QStandardItemModel
    , public DAVA::LoggerOutput
{
    Q_OBJECT

public:
    enum Roles
    {
        LEVEL_ROLE = Qt::UserRole,
    };

public:
    explicit LogModel(QObject* parent = NULL);
    ~LogModel();

    QPixmap GetIcon(int ll) const;

    void Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text) override;
signals:
    void Logged(int ll, const QString &text);
public slots:
    void AddMessage(int ll, const QString &text);

private:
    QStandardItem* CreateItem(int ll, const QString& text) const;
    QString normalize(const QString& text) const;

    mutable QMap<int, QPixmap> icons;
};


#endif // __LOGMODEL_H__
