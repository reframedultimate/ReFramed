#include "application/ui_MotionLabelsEditor.h"
#include "application/models/Protocol.hpp"
#include "application/models/MotionLabelsManager.hpp"
#include "application/views/MotionLabelsEditor.hpp"
#include "application/views/MainWindow.hpp"

#include "rfcommon/FighterState.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTableView>
#include <QAbstractTableModel>
#include <QPushButton>
#include <QComboBox>
#include <QSpacerItem>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QKeyEvent>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

#include <QDebug>

namespace rfapp {

namespace {

class TableModel
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

    TableModel(
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

    ~TableModel()
    {
        labels_->dispatcher.removeListener(this);
        protocol_->dispatcher.removeListener(this);
    }

    void setFighter(rfcommon::FighterID fighterID)
    {
        PROFILE(MotionLabelsEditorGlobal, setFighter);

        beginResetModel();
            fighterID_ = fighterID;
            repopulateEntries();
        endResetModel();
    }

    void setCategory(const QSet<int>& rows, rfcommon::MotionLabels::Category category)
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

    void setLabels(const QModelIndexList& indexes, const QString& label)
    {
        for (const QModelIndex& index : indexes)
        {
            const int row = table_[index.row()].row;
            const int layerIdx = index.column() - 2;
            const QByteArray labelUtf8 = label.toUtf8();
            labels_->changeLabel(fighterID_, row, layerIdx, labelUtf8.constData());
        }
    }

    int findNextConflict(int tableIdx, int direction)
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

    int rowCount(const QModelIndex& parent=QModelIndex()) const override
    {
        return table_.count();
    }

    int columnCount(const QModelIndex& parent=QModelIndex()) const override
    {
        return labels_->layerCount() + 2;
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override
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

    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override
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

    bool setData(const QModelIndex& mindex, const QVariant& value, int role) override
    {
        switch (role)
        {
            case Qt::EditRole: {
                const int tableIdx = mindex.row();
                const int layerIdx = mindex.column() - 2;
                const int row = table_[tableIdx].row;
                if (layerIdx < 0)
                    return false;

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

    Qt::ItemFlags flags(const QModelIndex& index) const override
    {
        if (index.column() < 2)
            return QAbstractTableModel::flags(index);

        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }

private:
    void repopulateEntries()
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

        sortTable();
    }

    void sortTable()
    {
        PROFILE(MotionLabelsEditorGlobal, sortTable);

        std::sort(table_.begin(), table_.end(), [](const Entry& a, const Entry& b){
            return a.name < b.name;
        });

        rowIdxToTableIdx_.resize(labels_->rowCount(fighterID_));
        for (int tableIdx = 0; tableIdx != table_.count(); ++tableIdx)
            rowIdxToTableIdx_[table_[tableIdx].row] = tableIdx;
    }

    int findTableInsertIdx(const Entry& entry)
    {
        PROFILE(MotionLabelsEditorGlobal, findTableInsertIdx);

        auto insertIt = std::lower_bound(table_.begin(), table_.end(), entry, [](const Entry& a, const Entry& b){
            return a.name < b.name;
        });

        return insertIt - table_.begin();
    }

private:
    void onMotionLabelsLoaded() override
    {
        beginResetModel();
            repopulateEntries();
        endResetModel();
    }

    void onMotionLabelsHash40sUpdated() override
    {
        emit dataChanged(index(0, 1), index(table_.count(), 1));
    }

    void onMotionLabelsLayerInserted(int layerIdx) override
    {
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

    void onMotionLabelsLayerRemoved(int layerIdx) override
    {
        beginResetModel();
            for (auto& row : table_)
                row.labels.erase(row.labels.begin() + layerIdx);
            for (const Entry& entry : table_)
                assert(entry.labels.size() == labels_->layerCount());
        endResetModel();
    }

    void onMotionLabelsLayerNameChanged(int layerIdx) override
    {
        emit headerDataChanged(Qt::Horizontal, layerIdx, layerIdx);
    }

    void onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) override
    {
        emit dataChanged(index(0, layerIdx + 2), index(table_.count(), layerIdx + 2));
    }

    void onMotionLabelsLayerMoved(int fromIdx, int toIdx) override
    {

    }

    void onMotionLabelsLayerMerged(int layerIdx) override
    {
        emit dataChanged(index(0, layerIdx + 2), index(table_.count(), layerIdx + 2));
    }

    void onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) override
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

    void onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) override
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

    void onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) override
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
            TableModel::onMotionLabelsRowInserted(fighterID, row);
        }
    }

private:
    void setActiveSession(rfcommon::Session* session)
    {
        session->tryGetFrameData()->dispatcher.addListener(this);
        mdata_ = session->tryGetMetadata();
    }
    void clearActiveSession(rfcommon::Session* session)
    {
        mdata_.drop();
        session->tryGetFrameData()->dispatcher.removeListener(this);

        clearHighlightedMotions();
    }

    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override {}
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override {}
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override {}
    void onProtocolDisconnectedFromServer() override {}

    void onProtocolTrainingStarted(rfcommon::Session* training) override { setActiveSession(training); }
    void onProtocolTrainingResumed(rfcommon::Session* training) override { setActiveSession(training); }
    void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) override
    {
        clearActiveSession(oldTraining);
        setActiveSession(newTraining);
    }
    void onProtocolTrainingEnded(rfcommon::Session* training) override { clearActiveSession(training); }
    void onProtocolGameStarted(rfcommon::Session* game) override { setActiveSession(game); }
    void onProtocolGameResumed(rfcommon::Session* game) override { setActiveSession(game); }
    void onProtocolGameEnded(rfcommon::Session* game) override { clearActiveSession(game); }

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) override
    {
        clearHighlightedMotions();
        for (int fighterIdx = 0; fighterIdx != frame.count(); ++fighterIdx)
            if (mdata_->playerFighterID(fighterIdx) == fighterID_)
                highlightedMotions_.insertIfNew(frame[fighterIdx].motion(), 0);
        refreshHighlightedMotions();
    }
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) override {}

    void clearHighlightedMotions()
    {
        rfcommon::SmallVector<int, 4> tableIdxs;
        for (auto it : highlightedMotions_)
            if (int row = labels_->lookupRow(fighterID_, it.key()) >= 0)
                tableIdxs.push(rowIdxToTableIdx_[row]);

        highlightedMotions_.clear();

        for (int tableIdx : tableIdxs)
            emit dataChanged(index(tableIdx, 0), index(tableIdx, labels_->layerCount() + 2), { Qt::BackgroundRole });
    }

    void refreshHighlightedMotions()
    {
        for (auto it : highlightedMotions_)
            if (int row = labels_->lookupRow(fighterID_, it.key()) >= 0)
            {
                int tableIdx = rowIdxToTableIdx_[row];
                emit dataChanged(index(tableIdx, 0), index(tableIdx, labels_->layerCount() + 2), { Qt::BackgroundRole });
            }
    }

private:
    Protocol* protocol_;
    rfcommon::Reference<rfcommon::Metadata> mdata_;
    rfcommon::MotionLabels* labels_;
    rfcommon::Vector<Entry> table_;
    rfcommon::Vector<int> rowIdxToTableIdx_;
    rfcommon::SmallLinearMap<rfcommon::FighterMotion, char, 4> highlightedMotions_;
    const rfcommon::MotionLabels::Category category_;
    rfcommon::FighterID fighterID_;
};

class TableView : public QTableView
{
public:
    void keyPressEvent(QKeyEvent* event) override
    {
        PROFILE(TableView, keyPressEvent);

        // If Ctrl-C typed
        // Or use event->matches(QKeySequence::Copy)
        if (event->key() == Qt::Key_C && (event->modifiers() & Qt::ControlModifier))
        {
            QModelIndexList cells = selectedIndexes();
            std::sort(cells.begin(), cells.end()); // Necessary, otherwise they are in column order

            QString text;
            int currentRow = 0; // To determine when to insert newlines
            foreach(const QModelIndex & cell, cells) {

                if (text.length() == 0) {
                    // First item
                }
                else if (cell.row() != currentRow) {
                    // New row
                    text += '\n';
                }
                else {
                    // Next cell
                    text += '\t';
                }
                currentRow = cell.row();
                text += cell.data().toString();
            }

            QApplication::clipboard()->setText(text);
        }
    }
};

}

// ----------------------------------------------------------------------------
MotionLabelsEditor::MotionLabelsEditor(
        MainWindow* mainWindow,
        MotionLabelsManager* manager,
        Protocol* protocol,
        rfcommon::MappingInfo* globalMappingInfo)
    : QDialog(mainWindow)
    , ui_(new Ui::MotionLabelsEditor)
    , mainWindow_(mainWindow)
    , manager_(manager)
    , globalMappingInfo_(globalMappingInfo)
{
    ui_->setupUi(this);

    setWindowTitle("Motion Labels Editor");

    // Tabs with different categories
#define X(category, name) \
        tableViews_.push(new TableView); \
        tableModels_.push(new TableModel(rfcommon::FighterID::fromValue(0), rfcommon::MotionLabels::category, manager_->motionLabels(), protocol)); \
        tableViews_.back()->setModel(tableModels_.back()); \
        ui_->tabWidget_categories->addTab(tableViews_.back(), name);
    RFCOMMON_MOTION_LABEL_CATEGORIES_LIST
#undef X

    // Fill dropdown with characters that have a non-zero amount of data
    for (auto fighterID : globalMappingInfo_->fighter.IDs())
    {
        if (manager_->motionLabels()->rowCount(fighterID) == 0)
            continue;
        updateFightersDropdown(fighterID);
    }

    // This causes models to update their data for the current fighter
    ui_->comboBox_fighters->setCurrentIndex(62);  // Pikachu
    if (ui_->comboBox_fighters->count() > 0)
    {
        for (int i = 0; i != tableModels_.count(); ++i)
            static_cast<TableModel*>(tableModels_[i])->setFighter(indexToFighterID_[62]);
    }

    // Check if there are any merge conflicts
    if (highlightNextConflict(1) == false)
        ui_->label_conflicts->setVisible(false);

    for (int i = 0; i != tableViews_.count(); ++i)
    {
        tableViews_[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tableViews_[i], &QTableView::customContextMenuRequested, [this, i](const QPoint& pos) {
            onCustomContextMenuRequested(i, tableViews_[i]->mapToGlobal(pos));
        });
    }

    //connect(closeButton, &QPushButton::released, this, &MotionLabelsEditor::close);
    connect(ui_->comboBox_fighters, qOverload<int>(&QComboBox::currentIndexChanged), this, &MotionLabelsEditor::onFighterSelected);
    connect(ui_->pushButton_nextConflict, &QPushButton::released, [this] { highlightNextConflict(1); });
    connect(ui_->pushButton_prevConflict, &QPushButton::released, [this] { highlightNextConflict(-1); });

    manager_->motionLabels()->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
MotionLabelsEditor::~MotionLabelsEditor()
{
    manager_->motionLabels()->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void MotionLabelsEditor::populateFromGlobalData(rfcommon::MappingInfo* globalMappingInfo)
{
    PROFILE(MotionLabelsEditor, populateFromGlobalData);

    // Create sorted list of all fighters
    struct FighterData {
        rfcommon::FighterID id;
        rfcommon::String name;
    };
    rfcommon::Vector<FighterData> fighterData;
    auto ids = globalMappingInfo->fighter.IDs();
    auto names = globalMappingInfo->fighter.names();
    for (int i = 0; i != ids.count(); ++i)
        fighterData.push({ids[i], names[i]});
    std::sort(fighterData.begin(), fighterData.end(), [](const FighterData& a, const FighterData& b) {
        return a.name < b.name;
    });

    // Add to dropdown, but only if there are entries in the user motion labels
    for (const auto& fighter : fighterData)
    {
        if (manager_->motionLabels()->rowCount(fighter.id) == 0)
            continue;

        indexToFighterID_.push(fighter.id);
        ui_->comboBox_fighters->addItem(fighter.name.cStr());
    }
}

// ----------------------------------------------------------------------------
void MotionLabelsEditor::populateFromSessions(rfcommon::Session** loadedSessions, int sessionCount)
{
    PROFILE(MotionLabelsEditor, populateFromSessions);

    auto idAlreadyAdded = [this](rfcommon::FighterID fighterID) -> bool {
        for (auto entry : indexToFighterID_)
            if (entry == fighterID)
                return true;
        return false;
    };

    // Create sorted list of all fighters
    rfcommon::Vector<rfcommon::String> fighterNames;
    for (int i = 0; i != sessionCount; ++i)
        if (const auto mdata = loadedSessions[i]->tryGetMetadata())
            if (const auto map = loadedSessions[i]->tryGetMappingInfo())
                for (int f = 0; f != mdata->fighterCount(); ++f)
                {
                    auto fighterID = mdata->playerFighterID(f);
                    if (idAlreadyAdded(fighterID))
                        continue;
                    fighterNames.push(map->fighter.toName(fighterID));
                    indexToFighterID_.push(fighterID);
                }

    std::sort(fighterNames.begin(), fighterNames.end());

    // Add to dropdown
    for (const auto& name : fighterNames)
        ui_->comboBox_fighters->addItem(name.cStr());

    for (int i = 0; i != sessionCount; ++i)
        if (const auto mdata = loadedSessions[i]->tryGetMetadata())
            if (const auto fdata = loadedSessions[i]->tryGetFrameData())
                for (int fighterIdx = 0; fighterIdx != fdata->fighterCount(); ++fighterIdx)
                    for (int frameIdx = 0; frameIdx != fdata->frameCount(); ++frameIdx)
                    {
                        const auto motion = fdata->stateAt(fighterIdx, frameIdx).motion();
                        manager_->motionLabels()->addUnknownMotion(mdata->playerFighterID(fighterIdx), motion);
                    }

    auto model = static_cast<TableModel*>(tableModels_[rfcommon::MotionLabels::UNLABELED]);
    if (const auto mdata = loadedSessions[0]->tryGetMetadata())
        model->setFighter(mdata->playerFighterID(0));
}

// ----------------------------------------------------------------------------
void MotionLabelsEditor::closeEvent(QCloseEvent* event)
{
    PROFILE(MotionLabelsEditor, closeEvent);

    mainWindow_->onMotionLabelsEditorClosed();
    deleteLater();
}

// ----------------------------------------------------------------------------
void MotionLabelsEditor::onFighterSelected(int index)
{
    PROFILE(MotionLabelsEditor, onFighterSelected);

    for (int i = 0; i != tableModels_.count(); ++i)
        static_cast<TableModel*>(tableModels_[i])->setFighter(indexToFighterID_[index]);
}

// ----------------------------------------------------------------------------
void MotionLabelsEditor::onCustomContextMenuRequested(int tabIdx, const QPoint& globalPos)
{
    PROFILE(MotionLabelsEditor, onCustomContextMenuRequested);

    // Submenu with all of the categories that are not the current category
    QMenu categoryMenu;
#define X(name, str) \
        QAction* cat##name = nullptr; \
        if (rfcommon::MotionLabels::name != tabIdx) \
            cat##name = categoryMenu.addAction(str);
    RFCOMMON_MOTION_LABEL_CATEGORIES_LIST
#undef X

    // Submenu with all of the layer usages
    QMenu usageMenu;
#define X(name, str) \
        QAction* usage##name = usageMenu.addAction(str);
    RFCOMMON_MOTION_LABEL_USAGE_LIST
#undef X

    // Main context menu
    QMenu menu;
    QAction* setLabel = menu.addAction("Set label");
    QAction* changeCategory = menu.addAction("Change label category");
    changeCategory->setMenu(&categoryMenu);
    QAction* propagateLabel = menu.addAction("Copy labels to other fighters");
    menu.addSeparator();
    QAction* renameLayer = menu.addAction("Rename layers");
    QAction* mergeLayers = menu.addAction("Merge layers");
    QAction* changeUsage = menu.addAction("Change layer usage");
    changeUsage->setMenu(&usageMenu);
    menu.addSeparator();
    QAction* newLayer = menu.addAction("Create new layer");
    QAction* deleteLayer = menu.addAction("Delete layers");
    QAction* importLayer = menu.addAction("Import layers");
    QAction* exportLayer = menu.addAction("Export layers");
    menu.addSeparator();
    QAction* updateHash40 = menu.addAction("Load Hash40 CSV");
    QAction* downloadHash40 = menu.addAction("Download latest ParamLabels.csv");

    // These are the cells we may be manipulating
    auto selectedIndexes = tableViews_[tabIdx]->selectionModel()->selectedIndexes();

    QSet<int> selectedColumns;
    for (const auto& index : selectedIndexes)
        selectedColumns.insert(index.column());
    QSet<int> selectedRows;
    for (const auto& index : selectedIndexes)
        selectedRows.insert(index.row());

    // If nothing is selected, disable menu options accordingly
    if (selectedIndexes.size() == 0)
    {
        setLabel->setEnabled(false);
        changeCategory->setEnabled(false);
        propagateLabel->setEnabled(false);

        renameLayer->setEnabled(false);
        mergeLayers->setEnabled(false);
        changeUsage->setEnabled(false);
        deleteLayer->setEnabled(false);
        exportLayer->setEnabled(false);
    }
    if (selectedColumns.size() != 2)
        mergeLayers->setEnabled(false);

    // Execute
    QAction* a = menu.exec(globalPos);
    if (a == nullptr)
        return;

    if (a == setLabel)
    {
        QString name = QInputDialog::getText(this, "Change selected labels", "Enter new label:");
        if (name.length() == 0)
            return;
        static_cast<TableModel*>(tableModels_[tabIdx])->setLabels(selectedIndexes, name);
    }
    else if (a == propagateLabel)
    {
        QMessageBox::critical(this, "Error", "Feature not implemented yet");
    }
#define X(name, str) \
    else if (a == cat##name) \
        static_cast<TableModel*>(tableModels_[tabIdx])->setCategory(selectedRows, rfcommon::MotionLabels::name);
    RFCOMMON_MOTION_LABEL_CATEGORIES_LIST
#undef X
    else if (a == renameLayer)
    {
        QString name = QInputDialog::getText(this, "Enter Name", "Layer Name:");
        if (name.length() == 0)
            return;
        QByteArray ba = name.toUtf8();
        for (int column : selectedColumns)
            manager_->motionLabels()->renameLayer(column - 2, ba.constData());
    }
    else if (a == mergeLayers)
    {
        auto columns = selectedColumns.values();
        int sourceIdx = columns[0] - 2;
        int targetIdx = columns[1] - 2;
        if (targetIdx > sourceIdx)
            std::swap(targetIdx, sourceIdx);
        if (manager_->motionLabels()->mergeLayers(targetIdx, sourceIdx) >= 0)
        {
            if (highlightNextConflict(1) == false)
                ui_->label_conflicts->setVisible(false);
            return;
        }

        QMessageBox::critical(this, "Error", "Can only merge layers that have the same usage. You can change a layer's usage with right-click -> \"Change layer usage\"");
    }
#define X(name, str) \
    else if (a == usage##name) \
        for (int column : selectedColumns) \
            manager_->motionLabels()->changeUsage(column - 2, rfcommon::MotionLabels::name);
    RFCOMMON_MOTION_LABEL_USAGE_LIST
#undef X
    else if (a == newLayer)
    {
        QString name = QInputDialog::getText(this, "Enter Name", "Layer Name:");
        if (name.length() == 0)
            return;
        QByteArray ba = name.toUtf8();
        manager_->motionLabels()->newLayer(ba.constData(), rfcommon::MotionLabels::NOTATION);
    }
    else if (a == deleteLayer)
    {
        QStringList nonEmptyLayerNames;
        for (int column : selectedColumns)
            if (manager_->motionLabels()->isLayerEmpty(column - 2) == false)
                nonEmptyLayerNames.append(QString::fromUtf8(manager_->motionLabels()->layerName(column - 2)));
        if (nonEmptyLayerNames.size() > 0)
            if (QMessageBox::warning(this,
                "Layers not empty",
                "Layers " + nonEmptyLayerNames.join(", ") + " contain labels. Are you sure you want to delete them?",
                QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
            {
                return;
            }

        rfcommon::SmallVector<int, 4> layerIdxs;
        for (int column : selectedColumns)
            layerIdxs.push(column - 2);
        manager_->motionLabels()->deleteLayers(layerIdxs);
    }
    else if (a == importLayer)
    {
        QString filePath = QFileDialog::getOpenFileName(this, "Import layer", "", "Layers file (*.json)");
        if (filePath.isEmpty())
            return;

        if (manager_->motionLabels()->importLayers(filePath.toUtf8().constData()) < 0)
            QMessageBox::critical(this, "Error", "Failed to import layers");
    }
    else if (a == exportLayer)
    {
        QString filePath = QFileDialog::getSaveFileName(this, "Export layers", "", "Layers file (*.json)");
        if (filePath.isEmpty())
            return;

        rfcommon::SmallVector<int, 4> layerIdxs;
        for (int column : selectedColumns)
            layerIdxs.push(column - 2);
        manager_->motionLabels()->exportLayers(layerIdxs, filePath.toUtf8().constData());
    }
    else if (a == updateHash40)
    {
        QString filePath = QFileDialog::getOpenFileName(this, "Import Hash40", "", "ParamLabels file (*.csv)");
        if (filePath.isEmpty())
            return;

        if (int updated = manager_->motionLabels()->updateHash40FromCSV(filePath.toUtf8().constData()) >= 0)
            QMessageBox::information(this, "Success", "Updated " + QString::number(updated) + "Hash40 strings");
        else
            QMessageBox::critical(this, "Error", "Failed to load file \"" + filePath + "\"");
    }
    else if (a == downloadHash40)
    {
        QMessageBox::critical(this, "Error", "Feature not implemented yet");
    }
}

// ----------------------------------------------------------------------------
void MotionLabelsEditor::updateFightersDropdown(rfcommon::FighterID fighterID)
{
    PROFILE(MotionLabelsEditor, updateFightersDropdown);

    // We update the dropdown of fighters if the fighter was previously unseen
    for (auto entry : indexToFighterID_)
        if (entry == fighterID)
            return;

    // Extract the list of fighters from the dropdown, which will be a list
    // of sorted fighter names
    QVector<QString> sortedNames;
    for (int i = 0; i != ui_->comboBox_fighters->count(); ++i)
        sortedNames.push_back(ui_->comboBox_fighters->itemText(i));

    QString fighterName = globalMappingInfo_->fighter.toName(fighterID);
    auto insertIt = std::lower_bound(sortedNames.begin(), sortedNames.end(), fighterName);
    int insertPos = insertIt - sortedNames.begin();

    indexToFighterID_.insert(insertPos, fighterID);
    ui_->comboBox_fighters->insertItem(insertPos, fighterName);
}

// ----------------------------------------------------------------------------
bool MotionLabelsEditor::highlightNextConflict(int direction)
{
    QSignalBlocker blockFighterChanges(ui_->comboBox_fighters);

    const int firstFighterIdx = ui_->comboBox_fighters->currentIndex();
    const int firstCategoryIdx = ui_->tabWidget_categories->currentIndex();

    int fighterIdx = ui_->comboBox_fighters->currentIndex();
    int categoryIdx = ui_->tabWidget_categories->currentIndex();

    QModelIndexList selectedIndexes = tableViews_[categoryIdx]->selectionModel()->selectedIndexes();
    if (selectedIndexes.size() > 0)
        currentConflictTableIdx_ = selectedIndexes[0].row();

    while (1)
    {
        auto view = static_cast<TableView*>(tableViews_[categoryIdx]);
        auto model = static_cast<TableModel*>(view->model());

        currentConflictTableIdx_ = model->findNextConflict(currentConflictTableIdx_, direction);
        if (currentConflictTableIdx_ >= 0)
        {
            ui_->comboBox_fighters->setCurrentIndex(fighterIdx);
            onFighterSelected(fighterIdx);
            ui_->tabWidget_categories->setCurrentIndex(categoryIdx);
            view->selectionModel()->select(model->index(currentConflictTableIdx_, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            view->setFocus();

            ui_->label_conflicts->setText("Merge conflicts found");
            ui_->label_conflicts->setStyleSheet("color: red;");
            ui_->label_conflicts->setVisible(true);

            return true;
        }

        categoryIdx += direction;
        if (categoryIdx < 0 || categoryIdx >= tableViews_.count())
        {
            categoryIdx = direction > 0 ? 0 : tableViews_.count() - 1;
            fighterIdx += direction;
            if (fighterIdx < 0 || fighterIdx >= ui_->comboBox_fighters->count())
                fighterIdx = direction > 0 ? 0 : ui_->comboBox_fighters->count() - 1;
            onFighterSelected(fighterIdx);  // Trigger a fighter change without updating the UI
        }

        model = static_cast<TableModel*>(tableViews_[categoryIdx]->model());
        if (fighterIdx == firstFighterIdx &&
            categoryIdx == firstCategoryIdx &&
            model->findNextConflict(currentConflictTableIdx_, direction) < 0)
        {
            ui_->comboBox_fighters->setCurrentIndex(firstFighterIdx);
            ui_->pushButton_nextConflict->setVisible(false);
            ui_->pushButton_prevConflict->setVisible(false);

            ui_->label_conflicts->setText("Conflicts resolved!");
            ui_->label_conflicts->setStyleSheet("color: green;");
            return false;
        }
    }
}

// ----------------------------------------------------------------------------
void MotionLabelsEditor::onMotionLabelsLoaded() {}
void MotionLabelsEditor::onMotionLabelsHash40sUpdated() {}

void MotionLabelsEditor::onMotionLabelsLayerInserted(int layerIdx) {}
void MotionLabelsEditor::onMotionLabelsLayerRemoved(int layerIdx) {}
void MotionLabelsEditor::onMotionLabelsLayerNameChanged(int layerIdx) {}
void MotionLabelsEditor::onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) {}
void MotionLabelsEditor::onMotionLabelsLayerMoved(int fromIdx, int toIdx) {}
void MotionLabelsEditor::onMotionLabelsLayerMerged(int layerIdx) {}

void MotionLabelsEditor::onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) { updateFightersDropdown(fighterID); }
void MotionLabelsEditor::onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) {}
void MotionLabelsEditor::onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) {}

}
