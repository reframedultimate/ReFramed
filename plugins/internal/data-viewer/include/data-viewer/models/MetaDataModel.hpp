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
    void onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) override;
    void onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) override;

    // Game related events
    void onMetaDataPlayerNameChanged(int fighterIdx, const rfcommon::SmallString<15>& name) override;
    void onMetaDataSetNumberChanged(rfcommon::SetNumber number) override;
    void onMetaDataGameNumberChanged(rfcommon::GameNumber number) override;
    void onMetaDataSetFormatChanged(const rfcommon::SetFormat& format) override;
    void onMetaDataWinnerChanged(int winnerPlayerIdx) override;

    // In training mode this increments every time a new training room is loaded
    void onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number)  override;

private:
    rfcommon::Reference<rfcommon::MetaData> meta_;
    rfcommon::Reference<rfcommon::MappingInfo> map_;
};
