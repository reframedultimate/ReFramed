#pragma once

#include "application/listeners/ReplayGroupListener.hpp"
#include "application/models/ReplayMetadataCache.hpp"

#include <QAbstractItemModel>
#include <QDate>

namespace rfapp {

class ReplayMetadataCache;

class ReplayListModel
        : public QAbstractItemModel
        , public ReplayGroupListener
{
public:
    enum ColumnType
    {
        Time,
        P1, P2,
        Round,
        Format,
        Score,
        Game,
        Stage,

        ColumnCount,

        P1Char, P2Char
    };

    ReplayListModel(ReplayMetadataCache* metadataCache);
    ~ReplayListModel();

    /*!
     * \brief Updates the view with data from the specified group. If the group
     * changes (files added/removed) the view will automatically update. If
     * the group is deleted the view will clear itself.
     */
    void setReplayGroup(ReplayGroup* group);
    void clearReplayGroup();

    QModelIndex index(int row, int column, const QModelIndex& parent=QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;

    /*!
     * \brief Returns the filename of the replay at the specified index,
     * or returns an empty string if the index is not an item representing
     * a filename (such as the root items that only represent a date)
     */
    QString indexFileName(const QModelIndex& index) const;

private:
    void addReplay(const QString& fileName);
    void removeReplay(const QString& fileName);

private:
    void onReplayGroupFileAdded(ReplayGroup* group, const QString& file) override;
    void onReplayGroupFileRemoved(ReplayGroup* group, const QString& file) override;

private:
    struct Replay
    {
        QString fileName;
        ReplayMetadataCache::Entry cache;
    };
    struct ReplaysOnDay
    {
        QString date;
        QVector<Replay> replays;
    };

    ReplayMetadataCache* metadataCache_;
    QVector<ReplaysOnDay> days_;
    ReplayGroup* currentGroup_ = nullptr;
};

}
