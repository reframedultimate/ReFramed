#pragma once

#include "rfcommon/Reference.hpp"
#include "rfcommon/MetadataListener.hpp"
#include <QAbstractItemModel>

namespace rfcommon {
    class Metadata;
    class GameMetadata;
    class TrainingMetadata;
    class MappingInfo;
}

class MetadataModel
        : public QAbstractItemModel
        , public rfcommon::MetadataListener
{
public:
    ~MetadataModel();

    void setMetadata(rfcommon::MappingInfo* mappingInfo, rfcommon::Metadata* metadata);

    QModelIndex index(int row, int column, const QModelIndex& parent=QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent=QModelIndex()) const override;
    int columnCount(const QModelIndex& parent=QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;

private:
    int gameRowCount(const rfcommon::GameMetadata* meta, const QModelIndex& parent) const;
    int trainingRowCount(const rfcommon::TrainingMetadata* meta, const QModelIndex& parent) const;
    QVariant gameData(const rfcommon::GameMetadata* meta, const QModelIndex& index, int role) const;
    QVariant trainingData(const rfcommon::TrainingMetadata* meta, const QModelIndex& index, int role) const;

private:
    void onMetadataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) override;
    void onMetadataTournamentDetailsChanged() override;
    void onMetadataEventDetailsChanged() override;
    void onMetadataCommentatorsChanged() override;
    void onMetadataGameDetailsChanged() override;
    void onMetadataPlayerDetailsChanged() override;
    void onMetadataWinnerChanged(int winnerPlayerIdx) override;
    void onMetadataTrainingSessionNumberChanged(rfcommon::SessionNumber number) override;

private:
    rfcommon::Reference<rfcommon::Metadata> meta_;
    rfcommon::Reference<rfcommon::MappingInfo> map_;
};
