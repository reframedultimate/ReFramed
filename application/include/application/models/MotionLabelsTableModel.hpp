#pragma once

#include "rfcommon/FighterID.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/LinearMap.hpp"
#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/MotionLabelsListener.hpp"
#include "rfcommon/ProtocolListener.hpp"
#include "rfcommon/Reference.hpp"

#include <QAbstractTableModel>

namespace rfcommon {
    class Session;
}

namespace rfapp {

class Protocol;

class MotionLabelsTableModel
        : public QAbstractTableModel
        , public rfcommon::MotionLabelsListener
        , public rfcommon::ProtocolListener
        , public rfcommon::FrameDataListener
{
public:
    struct Entry
    {
        int row;
        QString hash40;
        QString name;
        QVector<QString> labels;
    };

    MotionLabelsTableModel(
        rfcommon::FighterID fighterID,
        rfcommon::MotionLabels::Category category,
        rfcommon::MotionLabels* labels,
        Protocol* protocol);

    ~MotionLabelsTableModel();

    void setFighter(rfcommon::FighterID fighterID);
    void setCategory(const QSet<int>& rows, rfcommon::MotionLabels::Category category);
    void setLabels(const QModelIndexList& indexes, const QString& label);
    int propagateLabels(const QModelIndexList& indexes, bool replaceExisting, bool forceCreation);
    int findNextConflict(int tableIdx, int direction) const;
    int findHighlightedMotionRow() const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& mindex, const QVariant& value, int role) override;
    void sort(int column, Qt::SortOrder order=Qt::AscendingOrder) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    void repopulateEntries();
    int findTableInsertIdx(const Entry& entry);

private:
    void onMotionLabelsLoaded() override;
    void onMotionLabelsHash40sUpdated() override;

    void onMotionLabelsLayerInserted(int layerIdx) override;
    void onMotionLabelsLayerRemoved(int layerIdx) override;
    void onMotionLabelsLayerNameChanged(int layerIdx) override;
    void onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) override;
    void onMotionLabelsLayerMoved(int fromIdx, int toIdx) override;
    void onMotionLabelsLayerMerged(int layerIdx) override;

    void onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) override;
    void onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) override;
    void onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) override;

private:
    void setActiveSession(rfcommon::Session* session);
    void clearActiveSession(rfcommon::Session* session);

    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override;
    void onProtocolDisconnectedFromServer() override;

    void onProtocolTrainingStarted(rfcommon::Session* training) override;
    void onProtocolTrainingResumed(rfcommon::Session* training) override;
    void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) override;
    void onProtocolTrainingEnded(rfcommon::Session* training) override;
    void onProtocolGameStarted(rfcommon::Session* game) override;
    void onProtocolGameResumed(rfcommon::Session* game) override;
    void onProtocolGameEnded(rfcommon::Session* game) override;

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) override;

    void clearHighlightedMotions();
    void refreshHighlightedMotions();

private:
    Protocol* protocol_;
    rfcommon::Reference<rfcommon::Session> session_;
    rfcommon::MotionLabels* labels_;
    rfcommon::Vector<Entry> table_;
    rfcommon::Vector<int> rowIdxToTableIdx_;
    rfcommon::SmallLinearMap<rfcommon::FighterMotion, char, 4> highlightedMotions_;
    const rfcommon::MotionLabels::Category category_;
    rfcommon::FighterID fighterID_;
};

}
