#include "application/models/FighterIcon.hpp"
#include "application/models/ReplayListModel.hpp"
#include "application/models/ReplayGroup.hpp"

#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/FilePathResolver.hpp"

#include <algorithm>

#include <QBrush>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayListModel::ReplayListModel(ReplayMetadataCache* metadataCache)
    : metadataCache_(metadataCache)
{
}

// ----------------------------------------------------------------------------
ReplayListModel::~ReplayListModel()
{
    clearReplayGroup();
}

// ----------------------------------------------------------------------------
void ReplayListModel::setReplayGroup(ReplayGroup* group)
{
    PROFILE(ReplayListModel, setReplayGroup);

    beginResetModel();
        if (currentGroup_)
        {
            days_.clear();
            currentGroup_->dispatcher.removeListener(this);
            currentGroup_ = nullptr;
        }

        for (const auto& fileName : group->files())
            addReplay(fileName);

        currentGroup_ = group;
        currentGroup_->dispatcher.addListener(this);
    endResetModel();
}

// ----------------------------------------------------------------------------
void ReplayListModel::clearReplayGroup()
{
    PROFILE(ReplayListModel, clearReplayGroup);

    if (currentGroup_ == nullptr)
        return;

    beginResetModel();
        days_.clear();

        currentGroup_->dispatcher.removeListener(this);
        currentGroup_ = nullptr;
    endResetModel();
}

// ----------------------------------------------------------------------------
QModelIndex ReplayListModel::index(int row, int column, const QModelIndex& parent) const
{
    PROFILE(ReplayListModel, index);

    if (parent.isValid() == false)
        return createIndex(row, column, quintptr(-1));
    return createIndex(row, column, parent.row());
}

// ----------------------------------------------------------------------------
QModelIndex ReplayListModel::parent(const QModelIndex& index) const
{
    PROFILE(ReplayListModel, parent);

    if (index.isValid() == false)
        return QModelIndex();
    if (index.internalId() == quintptr(-1))
        return QModelIndex();
    return createIndex(index.internalId(), 0, quintptr(-1));
}

// ----------------------------------------------------------------------------
int ReplayListModel::rowCount(const QModelIndex& parent) const
{
    PROFILE(ReplayListModel, rowCount);

    if (parent.isValid() == false)
        return days_.size();
    if (parent.internalId() == quintptr(-1))
        return days_[parent.row()].replays.size();
    return 0;
}

// ----------------------------------------------------------------------------
int ReplayListModel::columnCount(const QModelIndex& parent) const
{
    PROFILE(ReplayListModel, columnCount);

    return ColumnCount;
}

// ----------------------------------------------------------------------------
QVariant ReplayListModel::data(const QModelIndex& index, int role) const
{
    PROFILE(ReplayListModel, data);

    switch (role)
    {
        case Qt::DisplayRole: {
            if (index.internalId() == quintptr(-1))
            {
                if (index.column() == 0)
                    return days_[index.row()].date;
            }
            else
            {
                const ReplaysOnDay& day = days_[index.internalId()];
                const auto& replay = day.replays[index.row()];

                switch (index.column())
                {
                    case Time: return replay.cache.time;
                    case P1: return replay.cache.p1name;
                    case P2: return replay.cache.p2name;
                    case Stage: return replay.cache.stage;
                    case Round: return replay.cache.round;
                    case Format: return replay.cache.format;
                    case Score: return replay.cache.score;
                    case Game: return replay.cache.game;
                }
            }
        } break;

        case Qt::DecorationRole: {
            if (index.internalId() == quintptr(-1))
                break;

            const ReplaysOnDay& day = days_[index.internalId()];
            const auto& replay = day.replays[index.row()];

            if (index.column() == P1)
                return FighterIcon::fromFighterID(replay.cache.p1fighterID, replay.cache.p1costume).scaledToHeight(20);
            if (index.column() == P2)
                return FighterIcon::fromFighterID(replay.cache.p2fighterID, replay.cache.p2costume).scaledToHeight(20);
        } break;
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant ReplayListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    PROFILE(ReplayListModel, headerData);

    switch (role)
    {
        case Qt::DisplayRole: {
            if (orientation == Qt::Horizontal)
            {
                switch (section)
                {
                    case Time: return "Time";
                    case P1: return "Player 1";
                    case P2: return "Player 2";
                    case Stage: return "Stage";
                    case Round: return "Round";
                    case Format: return "Format";
                    case Score: return "Score";
                    case Game: return "Game";
                }
            }
        } break;
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QString ReplayListModel::indexFileName(const QModelIndex& index) const
{
    PROFILE(ReplayListModel, indexFileName);

    if (index.isValid() == false)
        return "";
    if (index.internalId() == quintptr(-1))
        return "";

    const auto& replay = days_[index.internalId()].replays[index.row()];
    return replay.fileName;
}

// ----------------------------------------------------------------------------
void ReplayListModel::addReplay(const QString& fileName)
{
    PROFILE(ReplayListModel, addReplay);

    const ReplayMetadataCache::Entry* entry = metadataCache_->lookupFilename(fileName);

    ReplaysOnDay day;
    day.date = entry->date;
    auto it = std::lower_bound(days_.begin(), days_.end(), day, [](const ReplaysOnDay& a, const ReplaysOnDay& b) {
        return a.date > b.date;
    });

    int rootRow = it - days_.begin();
    if (it == days_.end() || it->date != day.date)
    {
        beginInsertRows(QModelIndex(), rootRow, rootRow);
            it = days_.insert(it, day);
            it->replays.push_back(Replay{ fileName, *entry });
        endInsertRows();
    }
    else
    {
        beginInsertRows(index(rootRow, 0), it->replays.size(), it->replays.size());
            it->replays.push_back(Replay{ fileName, *entry });
        endInsertRows();
    }

}

// ----------------------------------------------------------------------------
void ReplayListModel::removeReplay(const QString& fileName)
{
    PROFILE(ReplayListModel, removeReplay);

    QVector<ReplaysOnDay>::iterator dayIt;
    QVector<Replay>::iterator replayIt;
    for (dayIt = days_.begin(); dayIt != days_.end(); ++dayIt)
    {
        for (replayIt = dayIt->replays.begin(); replayIt != dayIt->replays.end(); ++replayIt)
            if (replayIt->fileName == fileName)
                goto found;
    }
    return;
found:;

    int rootRow = dayIt - days_.begin();
    int replayRow = replayIt - dayIt->replays.begin();
    beginRemoveRows(index(rootRow, 0), replayRow, replayRow);
    dayIt->replays.erase(replayIt);
    endRemoveRows();

    if (dayIt->replays.size() == 0)
    {
        beginRemoveRows(QModelIndex(), rootRow, rootRow);
        days_.erase(dayIt);
        endRemoveRows();
    }
}

// ----------------------------------------------------------------------------
void ReplayListModel::onReplayGroupFileAdded(ReplayGroup* group, const QString& file)
{
    PROFILE(ReplayListModel, onReplayGroupFileAdded);

    if (currentGroup_ && currentGroup_ == group)
        addReplay(file);
}

// ----------------------------------------------------------------------------
void ReplayListModel::onReplayGroupFileRemoved(ReplayGroup* group, const QString& file)
{
    PROFILE(ReplayListModel, onReplayGroupFileRemoved);

    if (currentGroup_ && currentGroup_ == group)
        removeReplay(file);
}

}
