#pragma once

#include "application/listeners/ReplayGroupListener.hpp"
#include "rfcommon/ReplayFileParts.hpp"
#include <QAbstractItemModel>
#include <QDate>

namespace rfcommon {
    class FilePathResolver;
}

namespace rfapp {

class ReplayListModel
        : public QAbstractItemModel
        , public ReplayGroupListener
{
public:
    enum ColumnType
    {
        Time,
        P1, P2,
        P1Char, P2Char,
        SetFormat,
        SetNumber,
        GameNumber,
        Stage,

        ColumnCount
    };

    ReplayListModel(rfcommon::FilePathResolver* filePathResolver);
    ~ReplayListModel();

    /*!
     * \brief Updates the view with data from the specified group. If the group
     * changes (files added/removed) the view will automatically update. If
     * the group is deleted the view will clear itself.
     */
    void setReplayGroup(ReplayGroup* group);
    void clearReplayGroup(ReplayGroup* group);

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
    void onReplayGroupFileAdded(ReplayGroup* group, const QString& file) override;
    void onReplayGroupFileRemoved(ReplayGroup* group, const QString& file) override;

private:
    struct ReplaysOnDay
    {
        QString date;
        QVector<rfcommon::ReplayFileParts> replays;
    };

    rfcommon::FilePathResolver* replayPathResolver_;
    QVector<ReplaysOnDay> days_;
    ReplayGroup* currentGroup_ = nullptr;
};

}
