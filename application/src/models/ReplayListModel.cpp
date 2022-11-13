#include "application/models/FighterIcon.hpp"
#include "application/models/ReplayListModel.hpp"
#include "application/models/ReplayGroup.hpp"

#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/FilePathResolver.hpp"

#include <algorithm>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayListModel::ReplayListModel(rfcommon::FilePathResolver* filePathResolver)
    : replayPathResolver_(filePathResolver)
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
                        case P2: return r.playerName(1).cStr();
                        case SetFormat: return r.setFormat().shortDescription();
                        case SetNumber: return r.round().number().value();
                        case GameNumber: return r.score().gameNumber().value();
                        case Stage: return r.stage().cStr();

                        case P1Char: return r.fighterName(0).cStr();
                        case P2Char: return r.fighterName(1).cStr();
                    }
                }
                else if (r.playerCount() == 4)
                {
                    // TODO
                }
            }
        } break;

        case Qt::DecorationRole: {
            if (index.internalId() == quintptr(-1))
                break;

            const auto& r = days_[index.internalId()].replays[index.row()];
            if (r.playerCount() == 2)
            {
                if (index.column() == P1)
                    return FighterIcon::fromFighterName(r.fighterName(0).cStr(), 0).scaledToHeight(20);
                if (index.column() == P2)
                    return FighterIcon::fromFighterName(r.fighterName(1).cStr(), 0).scaledToHeight(20);
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
                    case P1: return "Player 1";
                    case P2: return "Player 2";
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
QString ReplayListModel::indexFileName(const QModelIndex& index) const
{
    if (index.isValid() == false)
        return "";
    if (index.internalId() == quintptr(-1))
        return "";

    const auto& fileParts = days_[index.internalId()].replays[index.row()];
    return QString::fromUtf8(fileParts.originalFileName().cStr());
}

// ----------------------------------------------------------------------------
void ReplayListModel::addReplay(const QString& fileName)
{
    PROFILE(ReplayListModel, addReplay);

    auto parts = rfcommon::ReplayFileParts::fromFileName(fileName.toUtf8().constData());
    if (parts.hasMissingInfo())
    {
        rfcommon::String filePathUtf8 = replayPathResolver_->resolveGameFile(fileName.toUtf8().constData());
        if (filePathUtf8.length() > 0)
        {
            rfcommon::Reference<rfcommon::Session> session = rfcommon::Session::load(replayPathResolver_, filePathUtf8.cStr());
            if (session)
                if (auto map = session->tryGetMappingInfo())
                    if (auto mdata = session->tryGetMetaData())
                        parts.updateFromMetaData(map, mdata);
        }
    }

    ReplaysOnDay day;
    day.date = parts.date().cStr();

    auto it = std::lower_bound(days_.begin(), days_.end(), day, [](const ReplaysOnDay& a, const ReplaysOnDay& b) {
        return a.date > b.date;
    });

    int rootRow = it - days_.begin();
    if (it == days_.end() || it->date != day.date)
    {
        beginInsertRows(QModelIndex(), rootRow, rootRow);
        it = days_.insert(it, day);
        endInsertRows();
    }

    beginInsertRows(index(rootRow, 0), it->replays.size(), it->replays.size());
    it->replays.append(parts);
    endInsertRows();
}

// ----------------------------------------------------------------------------
void ReplayListModel::removeReplay(const QString& fileName)
{
    PROFILE(ReplayListModel, removeReplay);

    auto parts = rfcommon::ReplayFileParts::fromFileName(fileName.toUtf8().constData());

    ReplaysOnDay day;
    day.date = parts.date().cStr();
    auto it = std::lower_bound(days_.begin(), days_.end(), day, [](const ReplaysOnDay& a, const ReplaysOnDay& b) {
        return a.date > b.date;
    });
    if (it == days_.end() || it->date != day.date)
        return;

    int rootRow = it - days_.begin();
    for (auto it2 = it->replays.begin(); it2 != it->replays.end(); ++it2)
        if (it2->originalFileName() == parts.originalFileName())
        {
            int replayRow = it2 - it->replays.begin();
            beginRemoveRows(index(rootRow, 0), replayRow, replayRow);
            it->replays.erase(it2);
            endRemoveRows();
            break;
        }

    if (it->replays.size() == 0)
    {
        beginRemoveRows(QModelIndex(), rootRow, rootRow);
        days_.erase(it);
        endRemoveRows();
    }
}

// ----------------------------------------------------------------------------
void ReplayListModel::onReplayGroupFileAdded(ReplayGroup* group, const QString& file)
{
    if (currentGroup_ && currentGroup_ == group)
        addReplay(file);
}

// ----------------------------------------------------------------------------
void ReplayListModel::onReplayGroupFileRemoved(ReplayGroup* group, const QString& file)
{
    if (currentGroup_ && currentGroup_ == group)
        removeReplay(file);
}

}
