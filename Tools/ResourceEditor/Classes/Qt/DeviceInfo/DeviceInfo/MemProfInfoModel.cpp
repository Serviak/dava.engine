#include "Classes/Qt/DeviceInfo/DeviceInfo/MemProfInfoModel.h"

MemProfInfoModel::~MemProfInfoModel()
{

}
MemProfInfoModel::MemProfInfoModel()
{

}

int MemProfInfoModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{

    if (timedData.size() == 0 || timedData.last().statData.size() == 0)
        return 0;
    DAVA::uint32 tagDepth = timedData.last().statData[0].size();

    return  tagDepth;

}

int MemProfInfoModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (timedData.size() == 0)
        return 0;
    DAVA::uint32 poolCount = timedData.last().statData.size();
    return poolCount;
}

QVariant MemProfInfoModel::data(const QModelIndex& index, int role/* = Qt::DisplayRole*/) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();
    if (timedData.size() == 0)
        return QVariant();

    int pool = index.row();
    int tag = index.column();
    if (this->latestData)
    {
        const TagsStat & latestData = timedData.last();
        if (tag >= latestData.statData.size() || pool >= latestData.statData[tag].size())
            return QVariant();
        return latestData.statData[tag][pool].allocByApp;
    }
    else
    {
        const TagsStat & latestData = timedData[dataToShow];
        if (tag >= latestData.statData.size() || pool >= latestData.statData[tag].size())
            return QVariant();
        return latestData.statData[tag][pool].allocByApp;
    }
   
}
QVariant MemProfInfoModel::headerData(int section, Qt::Orientation orientation,
    int role /*= Qt::DisplayRole*/) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (timedData.size() == 0)
        return QVariant();

   

    if (orientation == Qt::Horizontal && tagNames.size() > section)
    {
        const TagsStat & stat = timedData.last();
        auto tagName = stat.tagNames[section ];
        return tagNames[tagName];
    }
    else if (poolNames.size() > section)
        return poolNames[section];


    if (orientation == Qt::Horizontal) {
        return QString("tag_") + std::to_string(section).c_str();
    }
    else
        return QString("pool_") + std::to_string(section).c_str();
    return QVariant();
}
void MemProfInfoModel::addMoreData(const DAVA::MMStat * data)
{

    DAVA::int32 rows = rowCount(), columns = columnCount();

    beginResetModel();
    TagsStat tagsStat;
    tagsStat.statData.resize(data->registredLabelCount);
    tagsStat.tagNames.resize(data->registredLabelCount);
    size_t offset = data->allocPoolCount*(data->tags.depth+1);
    for (size_t i = 0; i < tagsStat.statData.size(); i++)
    {
        auto tagID = data->tags.stack[i];
       
        tagsStat.tagNames[i] = i;
        tagsStat.statData[i].resize(data->allocPoolCount);
        for (size_t u = 0; u < tagsStat.statData[i].size(); u++)
        {
           
            tagsStat.statData[i][u].allocByApp = data->poolStat[offset + data->allocPoolCount*i + u].allocByApp;
            tagsStat.statData[i][u].allocTotal = data->poolStat[offset + data->allocPoolCount*i + u].allocTotal;
        }
    }

    timedData[data->timestamp] = tagsStat;
    endResetModel();
   

}
void MemProfInfoModel::showDataToClosest(size_t closest)
{
    latestData = false;
    auto it = timedData.begin();
    dataToShow = 0;
    while (it != timedData.end())
    {
        if (it.key() > closest)
            break;
        dataToShow = it.key();
        it++;
    }
    beginResetModel();
    endResetModel();
}
void MemProfInfoModel::showLatestData()
{
    latestData = true;
    beginResetModel();
    endResetModel();
}
void MemProfInfoModel::setConfig(const DAVA::MMStatConfig* statConfig)
{
    if (statConfig == nullptr) return;
    std::list<std::string> list;
    for (int i = statConfig->tagCount + statConfig->allocPoolCount; i > 0; i--)
    {
        list.push_back(statConfig->names[i-1].name);
    }
    tagNames.resize(statConfig->markCount);
    for (size_t i = 0; i < tagNames.size(); i++)
    {
        tagNames[i] = QString(statConfig->names[i + statConfig->tagCount + statConfig->allocPoolCount].name);
    }
    poolNames.resize(statConfig->allocPoolCount);
    for (size_t j = 0; j < poolNames.size(); j++)
    {
        poolNames[j] = QString(statConfig->names[j + statConfig->tagCount].name);
    }
    emit headerDataChanged(Qt::Horizontal, 0, columnCount());
    emit headerDataChanged(Qt::Vertical, 0, rowCount());
}
void MemProfInfoModel::forTagStats(std::function<void(int, const TagsStat&)> onStat)
{
    auto it = timedData.begin();
    while (it != timedData.end())
    {
        onStat(it.key(), it.value());
        it++;
    }
}