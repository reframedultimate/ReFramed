#pragma once

#include "rfcommon/Reference.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include <QAbstractTableModel>

namespace rfcommon {
    class MetaData;
    class GameMetaData;
    class TrainingMetaData;
    class MappingInfo;
}

class MetaDataModel
        : public QAbstractTableModel
        , public rfcommon::MetaDataListener
{
public:
    ~MetaDataModel();

    void setMetaData(rfcommon::MappingInfo* mappingInfo, rfcommon::MetaData* metaData);

    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;

private:
    QVariant gameData(const rfcommon::GameMetaData* meta, const QModelIndex& index, int role) const;
    QVariant trainingData(const rfcommon::TrainingMetaData* meta, const QModelIndex& index, int role) const;

private:
    void onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) override;
    void onMetaDataTournamentDetailsChanged() override;
    void onMetaDataEventDetailsChanged() override;
    void onMetaDataCommentatorsChanged() override;
    void onMetaDataGameDetailsChanged() override;
    void onMetaDataPlayerDetailsChanged() override;
    void onMetaDataWinnerChanged(int winnerPlayerIdx) override;
    void onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) override;

private:
    rfcommon::Reference<rfcommon::MetaData> meta_;
    rfcommon::Reference<rfcommon::MappingInfo> map_;
};
