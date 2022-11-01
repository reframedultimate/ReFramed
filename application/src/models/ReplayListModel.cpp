#include "application/models/ReplayListModel.hpp"
#include "application/models/ReplayGroup.hpp"

#include "rfcommon/Profiler.hpp"

#include <algorithm>

namespace rfapp {

// ----------------------------------------------------------------------------
void ReplayListModel::setReplayGroup(ReplayGroup* group)
{
    for (const auto& parts : group->files())
    {
        addReplay(parts);
    }
}

// ----------------------------------------------------------------------------
void ReplayListModel::clearReplayGroup(ReplayGroup* group)
{

}

// ----------------------------------------------------------------------------
QModelIndex ReplayListModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() == false)
        return createIndex(row, column, quintptr(-1));
    return createIndex(row, column, parent.row());
}

// ----------------------------------------------------------------------------
QModelIndex ReplayListModel::parent(const QModelIndex& index) const
{
    if (index.isValid() == false)
        return QModelIndex();
    if (index.internalId() == quintptr(-1))
        return QModelIndex();
    return createIndex(index.internalId(), 0, quintptr(-1));
}

// ----------------------------------------------------------------------------
int ReplayListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() == false)
        return days_.size();
    if (parent.internalId() == quintptr(-1))
        return days_[parent.row()].replays.size();
    return 0;
}

// ----------------------------------------------------------------------------
int ReplayListModel::columnCount(const QModelIndex& parent) const
{
    return ColumnCount;
}

// ----------------------------------------------------------------------------
QVariant ReplayListModel::data(const QModelIndex& index, int role) const
{
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
                const auto& r = days_[index.internalId()].replays[index.row()];
                if (r.playerCount() == 2)
                {
                    switch (index.column())
                    {
                        case Time: return r.time().cStr();
                        case P1: return r.playerName(0).cStr();
                        case P1Char: return r.characterName(0).cStr();
                        case P2: return r.playerName(1).cStr();
                        case P2Char: return r.characterName(0).cStr();
                        case SetFormat: return r.setFormat().shortDescription();
                        case SetNumber: return r.setNumber().value();
                        case GameNumber: return r.gameNumber().value();
                        case Stage: return r.stage().cStr();
                    }
                }
                else if (r.playerCount() == 4)
                {

                }
            }
        } break;
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant ReplayListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
        case Qt::DisplayRole: {
            if (orientation == Qt::Horizontal)
            {
                switch (section)
                {
                    case Time: return "Time";
                    case P1: return "P1";
                    case P2: return "P2";
                    case P1Char: return "Char";
                    case P2Char: return "Char";
                    case SetFormat: return "Format";
                    case SetNumber: return "Set";
                    case GameNumber: return "Game";
                    case Stage: return "Stage";
                }
            }
        } break;
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
void ReplayListModel::addReplay(const rfcommon::ReplayFileParts& replay)
{
    PROFILE(ReplayListModel, addReplay);

    ReplaysOnDay day;
    day.date = replay.date().cStr();

    auto it = std::lower_bound(days_.begin(), days_.end(), day, [](const ReplaysOnDay& a, const ReplaysOnDay& b) {
        return a.date < b.date;
    });
    if (it == days_.end() || it->date != day.date)
        it = days_.insert(it, day);
    it->replays.append(replay);
}

// ----------------------------------------------------------------------------
void ReplayListModel::removeReplay(const rfcommon::ReplayFileParts& replay)
{
    PROFILE(ReplayListModel, removeReplay);

    ReplaysOnDay day;
    day.date = replay.date().cStr();
    auto it = std::lower_bound(days_.begin(), days_.end(), day, [](const ReplaysOnDay& a, const ReplaysOnDay& b) {
        return a.date < b.date;
    });
    if (it == days_.end() || it->date != day.date)
        return;
    for (auto it2 = it->replays.begin(); it2 != it->replays.end(); ++it2)
        if (*it2 == replay)
        {
            it->replays.erase(it2);
            break;
        }

    if (it->replays.size() == 0)
        days_.erase(it);
}

// ----------------------------------------------------------------------------
void ReplayListModel::onReplayGroupFileAdded(ReplayGroup* group, const rfcommon::ReplayFileParts& file)
{

}

// ----------------------------------------------------------------------------
void ReplayListModel::onReplayGroupFileRemoved(ReplayGroup* group, const rfcommon::ReplayFileParts& file)
{

}

}
