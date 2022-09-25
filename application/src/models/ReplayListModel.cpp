#include "application/models/ReplayListModel.hpp"

#include "rfcommon/Profiler.hpp"

#include <algorithm>

namespace rfapp {

// ----------------------------------------------------------------------------
void ReplayListModel::addReplay(const QString& fileName)
{
    PROFILE(ReplayListModel, addReplay);

    ReplaysOnDay day;
    auto parts1 = fileName.splitRef(" - ");
    if (parts1.size() > 0)
        day.date = parts1[0].toString();
    else
        day.date = "No Date";

    QRegExp rx("^(\\d+)-(\\d+)-(\\d+) - (.*) \\((\\d+)\\) - (.*) \\((.*)\\) vs (.*) \\((.*)\\) Game (\\d+)\\..*");
    Replay replay;
    if (rx.indexIn(fileName) > -1)
    {
        auto groups = rx.capturedTexts();
        if (groups.size() > 4)
            replay.setFormat = groups[4];
        if (groups.size() > 5)
            replay.setNumber = groups[5];
        if (groups.size() > 6)
            replay.p1 = groups[6];
        if (groups.size() > 7)
            replay.p1char = groups[7];
        if (groups.size() > 8)
            replay.p2 = groups[8];
        if (groups.size() > 9)
            replay.p2char = groups[9];
        if (groups.size() > 10)
            replay.gameNumber = groups[10];
    }
    replay.fileName = fileName;

    auto it = std::lower_bound(days_.begin(), days_.end(), day, [](const ReplaysOnDay& a, const ReplaysOnDay& b) {
        return a.date < b.date;
    });
    if (it == days_.end() || it->date != day.date)
        it = days_.insert(it, day);
    it->replays.append(replay);
}

// ----------------------------------------------------------------------------
void ReplayListModel::removeReplay(const QString& fileName)
{
    PROFILE(ReplayListModel, removeReplay);

    ReplaysOnDay day;
    day.date = "2022-08-20";
    auto it = std::lower_bound(days_.begin(), days_.end(), day, [](const ReplaysOnDay& a, const ReplaysOnDay& b) {
        return a.date < b.date;
    });
    if (it == days_.end() || it->date != day.date)
        return;
    for (auto it2 = it->replays.begin(); it2 != it->replays.end(); ++it2)
        if (it2->fileName == fileName)
        {
            it->replays.erase(it2);
            break;
        }

    if (it->replays.size() == 0)
        days_.erase(it);
}

// ----------------------------------------------------------------------------
void ReplayListModel::clear()
{
    PROFILE(ReplayListModel, clear);

    days_.clear();
}

// ----------------------------------------------------------------------------
QString ReplayListModel::fileName(const QModelIndex& index) const
{
    if (index.isValid() == false || index.internalId() == quintptr(-1))
        return "";
    return days_[index.internalId()].replays[index.row()].fileName;
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
                const Replay& r = days_[index.internalId()].replays[index.row()];
                switch (index.column())
                {
                    case P1: return r.p1;
                    case P1Char: return r.p1char;
                    case P2: return r.p2;
                    case P2Char: return r.p2char;
                    case SetFormat: return r.setFormat;
                    case SetNumber: return r.setNumber;
                    case GameNumber: return r.gameNumber;
                    case DateTime: return r.time;
                    case Stage: return r.stage;
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
                    case P1: return "P1";
                    case P1Char: return "Character";
                    case P2: return "P2";
                    case P2Char: return "Character";
                    case SetFormat: return "Format";
                    case SetNumber: return "Set";
                    case GameNumber: return "Game";
                    case DateTime: return "Date";
                    case Stage: return "Stage";
                }
            }
        } break;
    }

    return QVariant();
}

}
