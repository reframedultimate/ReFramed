#include "application/models/UserMotionLabelsManager.hpp"
#include "application/views/UserMotionLabelsEditor.hpp"
#include "application/views/MainWindow.hpp"

#include "rfcommon/FighterState.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/UserMotionLabels.hpp"

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

#include "rfcommon/hash40.hpp"

namespace rfapp {

namespace {

class TableModel
    : public QAbstractTableModel
    , public rfcommon::UserMotionLabelsListener
{
public:
    struct Entry
    {
        int entryIdx;
        QString value;
        QString string;
        QVector<QString> labels;
    };

    TableModel(
            rfcommon::FighterID fighterID,
            rfcommon::UserMotionLabels* userMotionLabels,
            rfcommon::Hash40Strings* hash40Strings,
            rfcommon::UserMotionLabelsCategory category)
        : labels_(userMotionLabels)
        , hash40Strings_(hash40Strings)
        , category_(category)
        , fighterID_(fighterID)
    {
        repopulateEntries();
        labels_->dispatcher.addListener(this);
    }

    ~TableModel()
    {
        labels_->dispatcher.removeListener(this);
    }

    void setFighter(rfcommon::FighterID fighterID)
    {
    PROFILE(UserMotionLabelsEditorGlobal, setFighter);

        beginResetModel();
            fighterID_ = fighterID;
            repopulateEntries();
        endResetModel();
    }

    void setCategory(const QModelIndexList& indexes, rfcommon::UserMotionLabelsCategory category)
    {
    PROFILE(UserMotionLabelsEditorGlobal, setCategory);

        // Create a list of unique entry indices, as changing categories is comparitively expensive
        QSet<int> entryIndices;
        for (const auto& index : indexes)
            entryIndices.insert(table_[index.row()].entryIdx);

        // Change categories
        for (int entryIdx : entryIndices)
        {
            const auto motion = labels_->motionAt(fighterID_, entryIdx);
            labels_->changeCategory(fighterID_, motion, category);
        }
    }

    int rowCount(const QModelIndex& parent=QModelIndex()) const override { return table_.count(); }
    int columnCount(const QModelIndex& parent=QModelIndex()) const override { return labels_->layerCount() + 2; }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
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
                    case 0: return table_[index.row()].value;
                    case 1: return table_[index.row()].string;
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
                const int entryIdx = table_[tableIdx].entryIdx;
                if (layerIdx < 0)
                    return false;

                QByteArray ba = value.toString().toUtf8();
                const auto motion = labels_->motionAt(fighterID_, entryIdx);
                const char* newLabel = ba.constData();

                if (labels_->changeUserLabel(fighterID_, layerIdx, motion, newLabel))
                {
                    table_[tableIdx].labels[layerIdx] = newLabel;
                    emit dataChanged(index(tableIdx, layerIdx + 2), index(tableIdx, layerIdx + 2));
                    return true;
                }
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
        PROFILE(UserMotionLabelsEditorGlobal, repopulateEntries);

        table_.clearCompact();
        if (fighterID_.isValid() == false)
            return;

        for (int entryIdx = 0; entryIdx != labels_->entryCount(fighterID_); ++entryIdx)
        {
            if (labels_->categoryAt(fighterID_, entryIdx) != category_)
                continue;

            auto motion = labels_->motionAt(fighterID_, entryIdx);
            auto& entry = table_.emplace();
            entry.entryIdx = entryIdx;
            entry.value = "0x" + QString::number(motion.value(), 16);
            entry.string = hash40Strings_->toString(motion);
            for (int layerIdx = 0; layerIdx != labels_->layerCount(); ++layerIdx)
                entry.labels.push_back(labels_->userLabelAt(fighterID_, layerIdx, entryIdx));
        }

        sortTable();
    }

    void sortTable()
    {
        PROFILE(UserMotionLabelsEditorGlobal, sortTable);

        std::sort(table_.begin(), table_.end(), [](const Entry& a, const Entry& b){
            return a.string < b.string;
        });

        entryIdxToTableIdx_.resize(labels_->entryCount(fighterID_));
        for (int tableIdx = 0; tableIdx != table_.count(); ++tableIdx)
            entryIdxToTableIdx_[table_[tableIdx].entryIdx] = tableIdx;
    }

    int findTableInsertIdx(const Entry& entry)
    {
        PROFILE(UserMotionLabelsEditorGlobal, findTableInsertIdx);

        auto insertIt = std::lower_bound(table_.begin(), table_.end(), entry, [](const Entry& a, const Entry& b){
            return a.string < b.string;
        });

        return insertIt - table_.begin();
    }

private:
    void onUserMotionLabelsLayerAdded(int layerIdx, const char* name) override
    {
        beginInsertColumns(QModelIndex(), layerIdx + 2, layerIdx + 2);
            for (auto& row : table_)
                row.labels.insert(layerIdx, "");
        endInsertColumns();
    }

    void onUserMotionLabelsLayerRemoved(int layerIdx, const char* name) override
    {
        beginRemoveColumns(QModelIndex(), layerIdx + 2, layerIdx + 2);
            for (auto& row : table_)
                row.labels.erase(row.labels.begin() + layerIdx);
        endRemoveColumns();
    }

    void onUserMotionLabelsNewEntry(rfcommon::FighterID fighterID, int entryIdx) override
    {
        if (fighterID_ != fighterID)
            return;
        if (labels_->categoryAt(fighterID_, entryIdx) != category_)
            return;

        Entry entry;
        entry.entryIdx = entryIdx;
        entry.value = "0x" + QString::number(labels_->motionAt(fighterID, entryIdx).value(), 16);
        entry.string = hash40Strings_->toString(labels_->motionAt(fighterID, entryIdx));
        for (int layerIdx = 0; layerIdx != labels_->layerCount(); ++layerIdx)
            entry.labels.push_back(labels_->userLabelAt(fighterID_, layerIdx, entryIdx));
        const int tableIdx = findTableInsertIdx(entry);

        beginInsertRows(QModelIndex(), tableIdx, tableIdx);
            table_.insert(tableIdx, std::move(entry));

            // All table indices above insertion point need to be increased
            for (int i = 0; i != entryIdxToTableIdx_.count(); ++i)
                if (entryIdxToTableIdx_[i] >= tableIdx)
                    entryIdxToTableIdx_[i]++;

            entryIdxToTableIdx_.resize(labels_->entryCount(fighterID_));
            entryIdxToTableIdx_[entryIdx] = tableIdx;
        endInsertRows();
    }

    void onUserMotionLabelsUserLabelChanged(rfcommon::FighterID fighterID, int entryIdx, const char* oldLabel, const char* newLabel) override
    {
        if (fighterID_ != fighterID)
            return;
        if (labels_->categoryAt(fighterID_, entryIdx) != category_)
            return;

        const int tableIdx = entryIdxToTableIdx_[entryIdx];
        for (int layerIdx = 0; layerIdx != labels_->layerCount(); ++layerIdx)
            table_[tableIdx].labels[layerIdx] = labels_->userLabelAt(fighterID_, layerIdx, entryIdx);
        emit dataChanged(index(tableIdx, 2), index(tableIdx, labels_->layerCount() + 1));
    }

    void onUserMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int entryIdx, rfcommon::UserMotionLabelsCategory oldCategory, rfcommon::UserMotionLabelsCategory newCategory) override
    {
        if (fighterID_ != fighterID)
            return;

        if (oldCategory == category_)
        {
            const int tableIdx = entryIdxToTableIdx_[entryIdx];
            beginRemoveRows(QModelIndex(), tableIdx, tableIdx);
                table_.erase(tableIdx);

                // All table indices above deletion point need to be decreased
                for (int i = 0; i != entryIdxToTableIdx_.count(); ++i)
                    if (entryIdxToTableIdx_[i] > tableIdx)
                        entryIdxToTableIdx_[i]--;
            endRemoveRows();
        }
        else if (newCategory == category_)
        {
            TableModel::onUserMotionLabelsNewEntry(fighterID, entryIdx);
        }
    }

private:
    rfcommon::UserMotionLabels* labels_;
    rfcommon::Hash40Strings* hash40Strings_;
    rfcommon::Vector<Entry> table_;
    rfcommon::Vector<int> entryIdxToTableIdx_;
    const rfcommon::UserMotionLabelsCategory category_;
    rfcommon::FighterID fighterID_;
};

class TableView : public QTableView
{
public:
    void keyPressEvent(QKeyEvent* event) override
    {
        // If Ctrl-C typed
        // Or use event->matches(QKeySequence::Copy)
        if (event->key() == Qt::Key_C && (event->modifiers() & Qt::ControlModifier))
        {
            QModelIndexList cells = selectedIndexes();
            qSort(cells); // Necessary, otherwise they are in column order

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
UserMotionLabelsEditor::UserMotionLabelsEditor(
        MainWindow* mainWindow,
        UserMotionLabelsManager* manager,
        rfcommon::Hash40Strings* hash40Strings,
        rfcommon::MappingInfo* globalMappingInfo)
    : QWidget(nullptr)
    , mainWindow_(mainWindow)
    , manager_(manager)
    , hash40Strings_(hash40Strings)
    , globalMappingInfo_(globalMappingInfo)
    , comboBox_fighters(new QComboBox)
{
    // Window icon and title
    setWindowTitle("User Motion Labels Editor");
    setWindowIcon(QIcon(":/icons/reframed-icon.ico"));

    QVBoxLayout* mainLayout = new QVBoxLayout;

    // Search box
    QLabel* fightersDropdownLabel = new QLabel("Fighter:");
    QLabel* searchLabel = new QLabel("Search:");
    QLineEdit* searchBox = new QLineEdit;

    // Lay out fighter dropdown and search box
    QHBoxLayout* fighterSelectLayout = new QHBoxLayout;
    fighterSelectLayout->addWidget(fightersDropdownLabel);
    fighterSelectLayout->addWidget(comboBox_fighters);
    fighterSelectLayout->addWidget(searchLabel);
    fighterSelectLayout->addWidget(searchBox);
    fighterSelectLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
    mainLayout->addLayout(fighterSelectLayout);

    // Tabs with different categories
    tabWidget_categories = new QTabWidget;
#define X(category, name) \
        tableViews_.push(new TableView); \
        tableModels_.push(new TableModel(rfcommon::FighterID::makeInvalid(), manager_->userMotionLabels(), hash40Strings, rfcommon::UserMotionLabelsCategory::category)); \
        tableViews_.back()->setModel(tableModels_.back()); \
        tabWidget_categories->addTab(tableViews_.back(), name);
    RFCOMMON_USER_LABEL_CATEGORIES_LIST
#undef X
    mainLayout->addWidget(tabWidget_categories);

    // Create Close button
    QPushButton* closeButton = new QPushButton("Close");
    QHBoxLayout* closeLayout = new QHBoxLayout;
    closeLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
    closeLayout->addWidget(closeButton);
    mainLayout->addLayout(closeLayout);

    setLayout(mainLayout);

    // Fill dropdown with characters that have a non-zero amount of data
    for (auto fighterID : globalMappingInfo_->fighter.IDs())
    {
        if (manager_->userMotionLabels()->entryCount(fighterID) == 0)
            continue;
        updateFightersDropdown(fighterID);
    }

    // This causes models to update their data for the current fighter
    comboBox_fighters->setCurrentIndex(0);
    if (comboBox_fighters->count() > 0)
    {
        for (int i = 0; i != tableModels_.count(); ++i)
            static_cast<TableModel*>(tableModels_[i])->setFighter(indexToFighterID_[0]);
    }

    for (int i = 0; i != tableViews_.count(); ++i)
    {
        tableViews_[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tableViews_[i], &QTableView::customContextMenuRequested, [this, i](const QPoint& pos) {
            onCustomContextMenuRequested(i, tableViews_[i]->mapToGlobal(pos));
        });
    }

    //connect(closeButton, &QPushButton::released, this, &UserMotionLabelsEditor::close);
    connect(comboBox_fighters, qOverload<int>(&QComboBox::currentIndexChanged), this, &UserMotionLabelsEditor::onFighterSelected);

    manager_->userMotionLabels()->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
UserMotionLabelsEditor::~UserMotionLabelsEditor()
{
    manager_->userMotionLabels()->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void UserMotionLabelsEditor::populateFromGlobalData(rfcommon::MappingInfo* globalMappingInfo)
{
    PROFILE(UserMotionLabelsEditor, populateFromGlobalData);

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
        if (manager_->userMotionLabels()->entryCount(fighter.id) == 0)
            continue;

        indexToFighterID_.push(fighter.id);
        comboBox_fighters->addItem(fighter.name.cStr());
    }
}

// ----------------------------------------------------------------------------
void UserMotionLabelsEditor::populateFromSessions(rfcommon::Session** loadedSessions, int sessionCount)
{
    PROFILE(UserMotionLabelsEditor, populateFromSessions);

    auto idAlreadyAdded = [this](rfcommon::FighterID fighterID) -> bool {
        for (auto entry : indexToFighterID_)
            if (entry == fighterID)
                return true;
        return false;
    };

    // Create sorted list of all fighters
    rfcommon::Vector<rfcommon::String> fighterNames;
    for (int i = 0; i != sessionCount; ++i)
        if (const auto mdata = loadedSessions[i]->tryGetMetaData())
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
        comboBox_fighters->addItem(name.cStr());

    for (int i = 0; i != sessionCount; ++i)
        if (const auto mdata = loadedSessions[i]->tryGetMetaData())
            if (const auto fdata = loadedSessions[i]->tryGetFrameData())
                for (int fighterIdx = 0; fighterIdx != fdata->fighterCount(); ++fighterIdx)
                    for (int frameIdx = 0; frameIdx != fdata->frameCount(); ++frameIdx)
                    {
                        const auto motion = fdata->stateAt(fighterIdx, frameIdx).motion();
                        manager_->userMotionLabels()->addUnknownMotion(mdata->playerFighterID(fighterIdx), motion);
                    }

    auto model = static_cast<TableModel*>(tableModels_[rfcommon::UserMotionLabelsCategory::UNLABELED]);
    if (const auto mdata = loadedSessions[0]->tryGetMetaData())
        model->setFighter(mdata->playerFighterID(0));
}

// ----------------------------------------------------------------------------
void UserMotionLabelsEditor::closeEvent(QCloseEvent* event)
{
    PROFILE(UserMotionLabelsEditor, closeEvent);

    mainWindow_->onUserMotionLabelsEditorClosed();
    deleteLater();
}

// ----------------------------------------------------------------------------
void UserMotionLabelsEditor::onFighterSelected(int index)
{
    PROFILE(UserMotionLabelsEditor, onFighterSelected);

    for (int i = 0; i != tableModels_.count(); ++i)
        static_cast<TableModel*>(tableModels_[i])->setFighter(indexToFighterID_[index]);
}

// ----------------------------------------------------------------------------
void UserMotionLabelsEditor::onCustomContextMenuRequested(int tabIdx, const QPoint& globalPos)
{
    PROFILE(UserMotionLabelsEditor, onCustomContextMenuRequested);

    // Submenu with all of the categories that are not the current category
    QMenu categoryMenu;
#define X(category, name) \
        QAction* cat##category = nullptr; \
        if (rfcommon::UserMotionLabelsCategory::category != tabIdx) \
            cat##category = categoryMenu.addAction(name);
    RFCOMMON_USER_LABEL_CATEGORIES_LIST
#undef X

    // Main context menu
    QMenu menu;
    QAction* newLayer = menu.addAction("Create New Layer");
    QAction* changeCategory = menu.addAction("Change Category");
    changeCategory->setMenu(&categoryMenu);

    // These are the cells we may be manipulating
    auto selectedIndexes = tableViews_[tabIdx]->selectionModel()->selectedIndexes();
    if (selectedIndexes.size() == 0)
        changeCategory->setEnabled(false);

    // Execute
    QAction* a = menu.exec(globalPos);
    if (a == nullptr)
        return;

    if (a == newLayer)
    {
        QString name = QInputDialog::getText(this, "Enter Name", "Layer Name:");
        if (name.length() == 0)
            return;
        QByteArray ba = name.toUtf8();
        manager_->userMotionLabels()->newEmptyLayer(ba.constData());
    }
#define X(category, name) \
    else if (a == cat##category) \
        static_cast<TableModel*>(tableModels_[tabIdx])->setCategory(selectedIndexes, rfcommon::UserMotionLabelsCategory::category);
    RFCOMMON_USER_LABEL_CATEGORIES_LIST
#undef X
}

// ----------------------------------------------------------------------------
void UserMotionLabelsEditor::updateFightersDropdown(rfcommon::FighterID fighterID)
{
    PROFILE(UserMotionLabelsEditor, updateFightersDropdown);

    // We update the dropdown of fighters if the fighter was previously unseen
    for (auto entry : indexToFighterID_)
        if (entry == fighterID)
            return;

    // Extract the list of fighters from the dropdown, which will be a list
    // of sorted fighter names
    QVector<QString> sortedNames;
    for (int i = 0; i != comboBox_fighters->count(); ++i)
        sortedNames.push_back(comboBox_fighters->itemText(i));

    QString fighterName = globalMappingInfo_->fighter.toName(fighterID);
    auto insertIt = std::lower_bound(sortedNames.begin(), sortedNames.end(), fighterName);
    int insertPos = insertIt - sortedNames.begin();

    indexToFighterID_.insert(insertPos, fighterID);
    comboBox_fighters->insertItem(insertPos, fighterName);
}

// ----------------------------------------------------------------------------
void UserMotionLabelsEditor::onUserMotionLabelsLayerAdded(int layerIdx, const char* name) {}
void UserMotionLabelsEditor::onUserMotionLabelsLayerRemoved(int layerIdx, const char* name) {}

void UserMotionLabelsEditor::onUserMotionLabelsNewEntry(rfcommon::FighterID fighterID, int entryIdx)
{
    PROFILE(UserMotionLabelsEditor, onUserMotionLabelsNewEntry);

    updateFightersDropdown(fighterID);
}

void UserMotionLabelsEditor::onUserMotionLabelsUserLabelChanged(rfcommon::FighterID fighterID, int entryIdx, const char* oldLabel, const char* newLabel) {}
void UserMotionLabelsEditor::onUserMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int entryIdx, rfcommon::UserMotionLabelsCategory oldCategory, rfcommon::UserMotionLabelsCategory newCategory) {}

}
