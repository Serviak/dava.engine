#include "LogModel.h"

#include <QPainter>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QRegularExpression>
#include "Utils/UTF8Utils.h"
#include "Base/GlobalEnum.h"
#include "Debug/DVAssert.h"

LogModel::LogModel(QObject* parent)
    : QAbstractListModel(parent)
{
    createIcons();
    func = [](const DAVA::String &str)
    {
        return str;
    };
    qRegisterMetaType<DAVA::Logger::eLogLevel>("DAVA::Logger::eLogLevel");
    DAVA::Logger::AddCustomOutput(this);
    timer = new QTimer(this);
    timer->setInterval(1);
    connect(timer, &QTimer::timeout, this, &LogModel::OnTimeout, Qt::QueuedConnection);
}

LogModel::~LogModel()
{
    DAVA::Logger::RemoveCustomOutput(this);
}

void LogModel::SetConvertFunction(ConvertFunc func_)
{
    func = func_;
}

void LogModel::Output(DAVA::Logger::eLogLevel ll, const DAVA::char8* text)
{
    AddMessage(ll, text);
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    QMutexLocker locker(&mutex);
    const auto &item = items.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return item.text;

    case Qt::DecorationRole:
        return GetIcon(item.ll);

    case LEVEL_ROLE:
        return item.ll;
    case INTERNAL_DATA_ROLE:
        return item.data;
    default:
        return QVariant();
    }
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    QMutexLocker locker(&mutex);
    return items.size();
}

void LogModel::AddMessage(DAVA::Logger::eLogLevel ll, const QString& text)
{
    {
        QMutexLocker locker(&mutex);
        items.append(LogItem(ll,
            QString::fromStdString(func(text.toStdString())),
            text));
    }
    QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);
}

void LogModel::OnTimeout()
{
    if (rowCount() != registerCount)
    {
        emit beginInsertRows(QModelIndex(), registerCount, rowCount() - 1);
        registerCount = rowCount();
        emit endInsertRows();
    }
}

void LogModel::createIcons()
{
    const auto &logMap = GlobalEnumMap<DAVA::Logger::eLogLevel>::Instance();
    for (size_t i = 0; i < logMap->GetCount(); ++i)
    {
        int value;
        bool ok = logMap->GetValue(i, value);
        if (!ok)
        {
            DVASSERT_MSG(ok, "wrong enum used to create eLogLevel list");
            break;
        }
        QPixmap pix(16, 16);
        pix.fill(Qt::transparent);
        QPainter p(&pix);

        QColor bg = Qt::transparent;
        QColor fg1 = Qt::gray;

        switch (value)
        {
        case DAVA::Logger::LEVEL_FRAMEWORK:
            bg = Qt::lightGray;
            break;
        case DAVA::Logger::LEVEL_DEBUG:
            bg = Qt::blue;
            break;
        case DAVA::Logger::LEVEL_INFO:
            bg = Qt::green;
            break;
        case DAVA::Logger::LEVEL_WARNING:
            bg = Qt::yellow;
            break;
        case DAVA::Logger::LEVEL_ERROR:
            bg = Qt::red;
            break;
        default:
            break;
        }

        const int ofs = 3;

        p.setBrush(bg);
        QRect rc = QRect(QPoint(0, 0), pix.size()).adjusted(ofs, ofs, -ofs, -ofs);
        p.setPen(fg1);
        p.drawEllipse(rc);

        icons.append(pix);
    }
}

const QPixmap &LogModel::GetIcon(int ll) const
{
    return icons.at(ll);
}

LogModel::LogItem::LogItem(DAVA::Logger::eLogLevel ll_, const QString& text_, const QString &data_)
    : ll(ll_), text(text_), data(data_)
{
    text = text.split('\n', QString::SkipEmptyParts).join("\n");
}