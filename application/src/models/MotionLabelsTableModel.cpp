#include "application/models/MotionLabelsTableModel.hpp"
#include "application/models/Protocol.hpp"

#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"

#include <QBrush>
#include <QColor>
#include <QSet>

namespace rfapp {

// ----------------------------------------------------------------------------
MotionLabelsTableModel::MotionLabelsTableModel(
        rfcommon::FighterID fighterID,
        rfcommon::MotionLabels::Category category,
        rfcommon::MotionLabels* labels,
        Protocol* protocol)
    : protocol_(protocol)
    , labels_(labels)
    , category_(category)
    , fighterID_(fighterID)
{
    repopulateEntries();
    protocol_->dispatcher.addListener(this);
    labels_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
MotionLabelsTableModel::~MotionLabelsTableModel()
{
    if (session_.notNull())
        session_->tryGetFrameData()->dispatcher.removeListener(this);

    labels_->dispatcher.removeListener(this);
    protocol_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::setFighter(rfcommon::FighterID fighterID)
{
    PROFILE(MotionLabelsEditorGlobal, setFighter);

    beginResetModel();
        fighterID_ = fighterID;
        repopulateEntries();
    endResetModel();
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::setCategory(const QSet<int>& rows, rfcommon::MotionLabels::Category category)
{
    PROFILE(MotionLabelsEditorGlobal, setCategory);

    // Change categories
    for (int row : rows)
    {
        // Have to map from table row to motion labels row
        const int motionRow = table_[row].row;
        const auto motion = labels_->motionAt(fighterID_, motionRow);
        labels_->changeCategory(fighterID_, motionRow, category);
    }
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::setLabels(const QModelIndexList& indexes, const QString& label)
{
    for (const QModelIndex& index : indexes)
    {
        const int row = table_[index.row()].row;
        const int layerIdx = index.column() - 2;
        if (layerIdx < 0)
            continue;
        const QByteArray labelUtf8 = label.toUtf8();
        labels_->changeLabel(fighterID_, row, layerIdx, labelUtf8.constData());
    }
}

// ----------------------------------------------------------------------------
int MotionLabelsTableModel::propagateLabels(const QModelIndexList& indexes, bool replaceExisting, bool forceCreation)
{
    int updated = 0;
    for (const QModelIndex& index : indexes)
    {
        const int row = table_[index.row()].row;
        const int layerIdx = index.column() - 2;
        if (layerIdx < 0)
            continue;
        updated += labels_->propagateLabel(fighterID_, row, layerIdx);
    }
    return updated;
}

// ----------------------------------------------------------------------------
int MotionLabelsTableModel::findNextConflict(int tableIdx, int direction) const
{
    if (direction > 0)
    {
        for (int idx = tableIdx + 1; idx < table_.count(); ++idx)
            for (const QString& label : table_[idx].labels)
                if (label.contains("|"))
                    return idx;
    }
    else
    {
        if (tableIdx < 0)
            tableIdx = table_.count();
        for (int idx = tableIdx - 1; idx >= 0; --idx)
            for (const QString& label : table_[idx].labels)
                if (label.contains("|"))
                    return idx;
    }
    return -1;
}

// ----------------------------------------------------------------------------
int MotionLabelsTableModel::findHighlightedMotionRow() const
{
    if (highlightedMotions_.count() == 0)
        return -1;

    rfcommon::FighterMotion motion = highlightedMotions_.begin()->key();
    int row = labels_->lookupRow(fighterID_, motion);
    if (row == -1)
        return -1;

    return rowIdxToTableIdx_[row];
}

// ----------------------------------------------------------------------------
int MotionLabelsTableModel::rowCount(const QModelIndex& parent) const
{
    return table_.count();
}

// ----------------------------------------------------------------------------
int MotionLabelsTableModel::columnCount(const QModelIndex& parent) const
{
    return labels_->layerCount() + 2;
}

// ----------------------------------------------------------------------------
QVariant MotionLabelsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
        switch (orientation)
        {
        case Qt::Horizontal:
            switch (section)
            {
            case 0: return "Hash40";
            case 1: return "String";
            default:
                return labels_->layerName(section - 2);
            }
            break;

        case Qt::Vertical:
            return QString::number(section + 1);
            break;
        }
        break;
    }

    return QVariant();
}

// ----------------------------------------------------------------------------
QVariant MotionLabelsTableModel::data(const QModelIndex& index, int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column())
        {
        case 0: return table_[index.row()].hash40;
        case 1: return table_[index.row()].name;
        default:
            return table_[index.row()].labels[index.column() - 2];
        }
        break;

    case Qt::TextAlignmentRole:
        return Qt::AlignHCenter + Qt::AlignVCenter;

    case Qt::CheckStateRole:
    case Qt::DecorationRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
    case Qt::StatusTipRole:
    case Qt::WhatsThisRole:
    case Qt::SizeHintRole:
        break;

    case Qt::BackgroundRole: {
        const QColor rowHighlight(255, 255, 230);
        const QColor notationColor(240, 255, 255);
        const QColor readableColor(240, 240, 255);
        const QColor categorizationColor(255, 250, 240);
        const QColor hash40Color(230, 230, 230);

        const rfcommon::FighterMotion motion = labels_->motionAt(fighterID_, table_[index.row()].row);
        if (highlightedMotions_.findKey(motion) != highlightedMotions_.end())
            return QBrush(rowHighlight);

        if (index.column() == 0) return QBrush(hash40Color);
        if (index.column() == 1) return QBrush(hash40Color);
        switch (labels_->layerUsage(index.column() - 2))
        {
        case rfcommon::MotionLabels::NOTATION: return QBrush(notationColor);
        case rfcommon::MotionLabels::READABLE: return QBrush(readableColor);
        case rfcommon::MotionLabels::CATEGORIZATION: return QBrush(categorizationColor);
        }
    } break;
    }
    return QVariant();
}

// ----------------------------------------------------------------------------
bool MotionLabelsTableModel::setData(const QModelIndex& mindex, const QVariant& value, int role)
{
    switch (role)
    {
    case Qt::EditRole: {
        const int tableIdx = mindex.row();
        const int layerIdx = mindex.column() - 2;
        if (layerIdx < 0)
            return false;
        const int row = table_[tableIdx].row;

        QByteArray ba = value.toString().toUtf8();
        const auto motion = labels_->motionAt(fighterID_, row);
        const char* newLabel = ba.constData();

        labels_->changeLabel(fighterID_, row, layerIdx, newLabel);
        table_[tableIdx].labels[layerIdx] = newLabel;
        emit dataChanged(index(tableIdx, layerIdx + 2), index(tableIdx, layerIdx + 2));
        return true;
    } break;
    }

    return false;
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::sort(int column, Qt::SortOrder order)
{
    PROFILE(MotionLabelsEditorGlobal, sortTable);

    emit layoutAboutToBeChanged({}, QAbstractItemModel::VerticalSortHint);

        if (order == Qt::AscendingOrder)
        {
            std::sort(table_.begin(), table_.end(), [column](const Entry& a, const Entry& b) {
                if (column == 0) return a.hash40 < b.hash40;
                if (column == 1) return a.name < b.name;
                return a.labels[column - 2] < b.labels[column - 2];
            });
        }
        else
        {
            std::sort(table_.begin(), table_.end(), [column](const Entry& a, const Entry& b) {
                if (column == 0) return a.hash40 > b.hash40;
                if (column == 1) return a.name > b.name;
                return a.labels[column - 2] > b.labels[column - 2];
            });
        }

        for (int tableIdx = 0; tableIdx != table_.count(); ++tableIdx)
            rowIdxToTableIdx_[table_[tableIdx].row] = tableIdx;

    emit layoutChanged({}, QAbstractItemModel::VerticalSortHint);
}

// ----------------------------------------------------------------------------
Qt::ItemFlags MotionLabelsTableModel::flags(const QModelIndex& index) const
{
    if (index.column() < 2)
        return QAbstractTableModel::flags(index);

    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::repopulateEntries()
{
    PROFILE(MotionLabelsEditorGlobal, repopulateEntries);

    table_.clearCompact();

    for (int row = 0; row != labels_->rowCount(fighterID_); ++row)
    {
        if (labels_->categoryAt(fighterID_, row) != category_)
            continue;

        auto motion = labels_->motionAt(fighterID_, row);
        auto& entry = table_.emplace();
        entry.row = row;
        entry.hash40 = "0x" + QString::number(motion.value(), 16);
        entry.name = labels_->lookupHash40(motion);
        for (int layerIdx = 0; layerIdx != labels_->layerCount(); ++layerIdx)
            entry.labels.push_back(QString::fromUtf8(labels_->labelAt(fighterID_, layerIdx, row)));
    }

    rowIdxToTableIdx_.resize(labels_->rowCount(fighterID_));
    for (int tableIdx = 0; tableIdx != table_.count(); ++tableIdx)
        rowIdxToTableIdx_[table_[tableIdx].row] = tableIdx;
}

// ----------------------------------------------------------------------------
int MotionLabelsTableModel::findTableInsertIdx(const Entry& entry)
{
    PROFILE(MotionLabelsEditorGlobal, findTableInsertIdx);

    auto insertIt = std::lower_bound(table_.begin(), table_.end(), entry, [](const Entry& a, const Entry& b) {
        return a.name < b.name;
    });

    return insertIt - table_.begin();
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::onMotionLabelsLoaded()
{
    beginResetModel();
        repopulateEntries();
    endResetModel();
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::onMotionLabelsHash40sUpdated()
{
    for (int row = 0; row != labels_->rowCount(fighterID_); ++row)
        if (labels_->categoryAt(fighterID_, row) == category_)
        {
            auto motion = labels_->motionAt(fighterID_, row);
            int tableIdx = rowIdxToTableIdx_[row];
            table_[tableIdx].name = labels_->lookupHash40(motion);
        }

    emit dataChanged(index(0, 1), index(table_.count(), 1));
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::onMotionLabelsLayerInserted(int layerIdx)
{
    // Row count could have changed
    if (table_.count() != labels_->rowCount(fighterID_))
    {
        beginResetModel();
            repopulateEntries();
        endResetModel();
        return;
    }

    beginInsertColumns(QModelIndex(), layerIdx + 2, layerIdx + 2);
        for (int rowIdx = 0; rowIdx != labels_->rowCount(fighterID_); ++rowIdx)
        {
            if (labels_->categoryAt(fighterID_, rowIdx) != category_)
                continue;

            const char* label = labels_->labelAt(fighterID_, layerIdx, rowIdx);
            int tableIdx = rowIdxToTableIdx_[rowIdx];
            table_[tableIdx].labels.insert(layerIdx, QString::fromUtf8(label));
        }
        for (const Entry& entry : table_)
            assert(entry.labels.size() == labels_->layerCount());
    endInsertColumns();
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::onMotionLabelsLayerRemoved(int layerIdx)
{
    beginResetModel();
        for (auto& row : table_)
            row.labels.erase(row.labels.begin() + layerIdx);
        for (const Entry& entry : table_)
            assert(entry.labels.size() == labels_->layerCount());
    endResetModel();
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::onMotionLabelsLayerNameChanged(int layerIdx)
{
    emit headerDataChanged(Qt::Horizontal, layerIdx, layerIdx);
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage)
{
    emit dataChanged(index(0, layerIdx + 2), index(table_.count(), layerIdx + 2));
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::onMotionLabelsLayerMoved(int fromIdx, int toIdx)
{

}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::onMotionLabelsLayerMerged(int layerIdx)
{
    emit dataChanged(index(0, layerIdx + 2), index(table_.count(), layerIdx + 2));
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row)
{
    if (fighterID_ != fighterID)
        return;
    if (labels_->categoryAt(fighterID_, row) != category_)
        return;

    rfcommon::FighterMotion motion = labels_->motionAt(fighterID, row);

    Entry entry;
    entry.row = row;
    entry.hash40 = "0x" + QString::number(motion.value(), 16);
    entry.name = labels_->lookupHash40(motion);
    for (int layerIdx = 0; layerIdx != labels_->layerCount(); ++layerIdx)
        entry.labels.push_back(labels_->labelAt(fighterID_, layerIdx, row));
    const int tableIdx = findTableInsertIdx(entry);

    beginInsertRows(QModelIndex(), tableIdx, tableIdx);
        table_.insert(tableIdx, std::move(entry));

        // All table indices above insertion point need to be increased
        for (int i = 0; i != rowIdxToTableIdx_.count(); ++i)
            if (rowIdxToTableIdx_[i] >= tableIdx)
                rowIdxToTableIdx_[i]++;

        rowIdxToTableIdx_.resize(labels_->rowCount(fighterID_));
        rowIdxToTableIdx_[row] = tableIdx;

        for (const Entry& entry : table_)
            assert(entry.labels.size() == labels_->layerCount());
    endInsertRows();
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx)
{
    if (fighterID_ != fighterID)
        return;
    if (labels_->categoryAt(fighterID_, row) != category_)
        return;

    const int tableIdx = rowIdxToTableIdx_[row];
    for (int layerIdx = 0; layerIdx != labels_->layerCount(); ++layerIdx)
        table_[tableIdx].labels[layerIdx] = labels_->labelAt(fighterID_, layerIdx, row);

    emit dataChanged(index(tableIdx, 2), index(tableIdx, labels_->layerCount() + 1));
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory)
{
    if (fighterID_ != fighterID)
        return;

    if (oldCategory == category_)
    {
        const int tableIdx = rowIdxToTableIdx_[row];
        beginRemoveRows(QModelIndex(), tableIdx, tableIdx);
            table_.erase(tableIdx);

            // All table indices above deletion point need to be decreased
            for (int i = 0; i != rowIdxToTableIdx_.count(); ++i)
                if (rowIdxToTableIdx_[i] > tableIdx)
                    rowIdxToTableIdx_[i]--;
        endRemoveRows();
    }
    else if (labels_->categoryAt(fighterID_, row) == category_)
    {
        MotionLabelsTableModel::onMotionLabelsRowInserted(fighterID, row);
    }
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::setActiveSession(rfcommon::Session* session)
{
    assert(session_.isNull());

    session_ = session;
    session_->tryGetFrameData()->dispatcher.addListener(this);
}
void MotionLabelsTableModel::clearActiveSession(rfcommon::Session* session)
{
    assert(session_.notNull());
    assert(session_ == session);

    session_->tryGetFrameData()->dispatcher.removeListener(this);
    session_.drop();

    clearHighlightedMotions();
}

void MotionLabelsTableModel::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void MotionLabelsTableModel::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}
void MotionLabelsTableModel::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) {}
void MotionLabelsTableModel::onProtocolDisconnectedFromServer() {}

void MotionLabelsTableModel::onProtocolTrainingStarted(rfcommon::Session* training) { setActiveSession(training); }
void MotionLabelsTableModel::onProtocolTrainingResumed(rfcommon::Session* training) { setActiveSession(training); }
void MotionLabelsTableModel::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    clearActiveSession(oldTraining);
    setActiveSession(newTraining);
}
void MotionLabelsTableModel::onProtocolTrainingEnded(rfcommon::Session* training) { clearActiveSession(training); }
void MotionLabelsTableModel::onProtocolGameStarted(rfcommon::Session* game) { setActiveSession(game); }
void MotionLabelsTableModel::onProtocolGameResumed(rfcommon::Session* game) { setActiveSession(game); }
void MotionLabelsTableModel::onProtocolGameEnded(rfcommon::Session* game) { clearActiveSession(game); }

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    clearHighlightedMotions();

    // Add motion to list of highlighted motions if the fighterID and categories
    // match our table model
    for (int fighterIdx = 0; fighterIdx != frame.count(); ++fighterIdx)
    {
        if (session_->tryGetMetadata()->playerFighterID(fighterIdx) != fighterID_)
            continue;

        int row = labels_->lookupRow(fighterID_, frame[fighterIdx].motion());
        if (row < 0)
            continue;

        if (labels_->categoryAt(fighterID_, row) == category_)
            highlightedMotions_.insertIfNew(frame[fighterIdx].motion(), 0);
    }

    refreshHighlightedMotions();
}
void MotionLabelsTableModel::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) {}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::clearHighlightedMotions()
{
    rfcommon::SmallVector<int, 4> tableIdxs;
    for (auto it : highlightedMotions_)
        if (int row = labels_->lookupRow(fighterID_, it.key()) >= 0)
            tableIdxs.push(rowIdxToTableIdx_[row]);

    highlightedMotions_.clear();

    for (int tableIdx : tableIdxs)
        emit dataChanged(index(tableIdx, 0), index(tableIdx, labels_->layerCount() + 2), { Qt::BackgroundRole });
}

// ----------------------------------------------------------------------------
void MotionLabelsTableModel::refreshHighlightedMotions()
{
    for (auto it : highlightedMotions_)
        if (int row = labels_->lookupRow(fighterID_, it.key()) >= 0)
        {
            int tableIdx = rowIdxToTableIdx_[row];
            emit dataChanged(index(tableIdx, 0), index(tableIdx, labels_->layerCount() + 2), { Qt::BackgroundRole });
        }
}

}