#pragma once

#include "rfcommon/Reference.hpp"
#include "rfcommon/SessionMetaDataListener.hpp"
#include <QAbstractTableModel>

namespace rfcommon {
    class SessionMetaData;
    class GameSessionMetaData;
    class TrainingSessionMetaData;
    class MappingInfo;
}

class MetaDataModel
        : public QAbstractTableModel
        , public rfcommon::SessionMetaDataListener
{
public:
    ~MetaDataModel();

    void setMetaData(rfcommon::MappingInfo* mappingInfo, rfcommon::SessionMetaData* metaData);

    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;

private:
    QVariant gameData(const rfcommon::GameSessionMetaData* meta, const QModelIndex& index, int role) const;
    QVariant trainingData(const rfcommon::TrainingSessionMetaData* meta, const QModelIndex& index, int role) const;

private:
    void onSessionMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) override;
    void onSessionMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) override;

    // Game related events
    void onSessionMetaDataPlayerNameChanged(int fighterIdx, const rfcommon::SmallString<15>& name) override;
    void onSessionMetaDataSetNumberChanged(rfcommon::SetNumber number) override;
    void onSessionMetaDataGameNumberChanged(rfcommon::GameNumber number) override;
    void onSessionMetaDataSetFormatChanged(const rfcommon::SetFormat& format) override;
    void onSessionMetaDataWinnerChanged(int winnerPlayerIdx) override;

    // In training mode this increments every time a new training room is loaded
    void onSessionMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number)  override;

private:
    rfcommon::Reference<rfcommon::SessionMetaData> meta_;
    rfcommon::Reference<rfcommon::MappingInfo> map_;
};
