#pragma once

#include "application/listeners/ReplayGroupListener.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/GameNumber.hpp"
#include "rfcommon/SetNumber.hpp"
#include <QAbstractItemModel>
#include <QDate>

namespace rfapp {

class ReplayListModel
        : public QAbstractItemModel
        , public ReplayGroupListener
{
public:
    enum ColumnType
    {
        P1, P1Char,
        P2, P2Char,
        SetFormat,
        SetNumber,
        GameNumber,
        DateTime,
        Stage,

        ColumnCount
    };

    /*!
     * \brief Updates the view with data from the specified group. If the group
     * changes (files added/removed) the view will automatically update. If
     * the group is deleted the view will clear itself.
     */
    void setReplayGroup(ReplayGroup* group);
    void clearReplayGroup(ReplayGroup* group);

    QString fileName(const QModelIndex& index) const;

    QModelIndex index(int row, int column, const QModelIndex& parent=QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;

private:
    void addReplay(const QString& fileName);
    void removeReplay(const QString& fileName);

private:
    void onReplayGroupFileAdded(ReplayGroup* group, const QString& fileName) override;
    void onReplayGroupFileRemoved(ReplayGroup* group, const QString& fileName) override;

private:
    struct ReplaysOnDay;
    struct Replay
    {
        QString fileName;
        QString p1, p1char;
        QString p2, p2char;
        QString setFormat;
        QString gameNumber;
        QString setNumber;
        QString time;
        QString stage;
    };

    struct ReplaysOnDay
    {
        QString date;
        QVector<Replay> replays;
    };

    QVector<ReplaysOnDay> days_;
    ReplayGroup* currentGroup_ = nullptr;
};

}
