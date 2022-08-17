#include "application/views/UserMotionLabelsEditor.hpp"
#include "application/models/UserMotionLabelsManager.hpp"

#include "rfcommon/FighterState.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Hash40Strings.hpp"
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

#include "rfcommon/hash40.hpp"

namespace rfapp {

namespace {

class TableModel 
    : public QAbstractTableModel
    , public rfcommon::UserMotionLabelsListener
{
public:
    TableModel(
            rfcommon::FighterID fighterID, 
            rfcommon::UserMotionLabels* userMotionLabels,
            rfcommon::Hash40Strings* hash40Strings,
            rfcommon::FighterUserMotionLabels::Category category)
        : labels_(userMotionLabels)
        , hash40Strings_(hash40Strings)
        , category_(category)
        , fighterID_(fighterID)
    {
        repopulateEntries();
        sortTable();
        labels_->dispatcher.addListener(this);
    }

    ~TableModel()
    {
        labels_->dispatcher.removeListener(this);
    }

    void setFighter(rfcommon::FighterID fighterID)
    {
        beginResetModel();
            fighterID_ = fighterID;
            repopulateEntries();
            sortTable();
        endResetModel();
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

    bool setData(const QModelIndex& mindex, const QVariant& value, int role=Qt::EditRole) override
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
                const auto category = labels_->categoryAt(fighterID_, entryIdx);
                const char* oldLabel = labels_->userLabelAt(fighterID_, layerIdx, entryIdx);
                const char* newLabel = ba.constData();

                if (labels_->modifyEntry(fighterID_, layerIdx, motion, oldLabel, newLabel, category))
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

        return Qt::ItemIsSelectable + Qt::ItemIsEditable + Qt::ItemIsEnabled;
    }

private:
    void repopulateEntries()
    {
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
    }

    void sortTable()
    {
        std::sort(table_.begin(), table_.end(), [](const Entry& a, const Entry& b){
            return a.string < b.string;
        });

        entryIdxToTableIdx_.resize(table_.count());
        for (int tableIdx = 0; tableIdx != table_.count(); ++tableIdx)
            entryIdxToTableIdx_[table_[tableIdx].entryIdx] = tableIdx;
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

        beginInsertRows(QModelIndex(), entryIdx, entryIdx);
            auto entry = table_.emplace();
            entry.entryIdx = entryIdx;
            entry.value = QString::number(labels_->motionAt(fighterID, entryIdx).value());
            entry.string = hash40Strings_->toString(labels_->motionAt(fighterID, entryIdx));
            for (int layerIdx = 0; layerIdx != labels_->layerCount(); ++layerIdx)
                entry.labels.push_back(labels_->userLabelAt(fighterID_, layerIdx, entryIdx));
            sortTable();
        endInsertRows();
    }

    void onUserMotionLabelsEntryChanged(rfcommon::FighterID fighterID, int entryIdx) override 
    {
        if (fighterID_ != fighterID)
            return;

        const int tableIdx = entryIdxToTableIdx_[entryIdx];
        for (int layerIdx = 0; layerIdx != labels_->layerCount(); ++layerIdx)
            table_[tableIdx].labels[layerIdx] = labels_->userLabelAt(fighterID_, layerIdx, entryIdx);
        emit dataChanged(index(tableIdx, 2), index(tableIdx, labels_->layerCount() + 1));
    }

    void onUserMotionLabelsEntryRemoved(rfcommon::FighterID fighterID, int entryIdx) override 
    {
        if (fighterID_ != fighterID)
            return;

        beginRemoveRows(QModelIndex(), entryIdx, entryIdx);
            int tableIdx = entryIdxToTableIdx_[entryIdx];
            table_.erase(tableIdx);
            sortTable();
        endRemoveRows();
    }

private:
    struct Entry
    {
        int entryIdx;
        QString value;
        QString string;
        QVector<QString> labels;
    };

    rfcommon::UserMotionLabels* labels_;
    rfcommon::Hash40Strings* hash40Strings_;
    rfcommon::Vector<Entry> table_;
    rfcommon::Vector<int> entryIdxToTableIdx_;
    const rfcommon::FighterUserMotionLabels::Category category_;
    rfcommon::FighterID fighterID_;
};

}

// ----------------------------------------------------------------------------
UserMotionLabelsEditor::UserMotionLabelsEditor(
        UserMotionLabelsManager* manager, 
        rfcommon::Hash40Strings* hash40Strings, 
        QWidget* parent)
    : QDialog(parent)
    , manager_(manager)
    , hash40Strings_(hash40Strings)
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
    QTabWidget* tabWidget = new QTabWidget;
#define X(category, name) \
        tableViews_.push(new QTableView); \
        tableModels_.push(new TableModel(rfcommon::FighterID::makeInvalid(), manager_->userMotionLabels(), hash40Strings, rfcommon::FighterUserMotionLabels::category)); \
        tableViews_.back()->setModel(tableModels_.back()); \
        tabWidget->addTab(tableViews_.back(), name);
    RFCOMMON_USER_LABEL_CATEGORIES_LIST
#undef X
    mainLayout->addWidget(tabWidget);

    // Create Close button
    QPushButton* closeButton = new QPushButton("Close");
    QHBoxLayout* closeLayout = new QHBoxLayout;
    closeLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
    closeLayout->addWidget(closeButton);
    mainLayout->addLayout(closeLayout);

    setLayout(mainLayout);

    for (int i {}; i != tableViews_.count(); ++i)
    {
        tableViews_[i]->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tableViews_[i], &QTableView::customContextMenuRequested, [this, i](const QPoint& pos) {
            onCustomContextMenuRequested(i, tableViews_[i]->mapToGlobal(pos));
        });
    }

    //connect(closeButton, &QPushButton::released, this, &UserMotionLabelsEditor::close);
    connect(comboBox_fighters, qOverload<int>(&QComboBox::currentIndexChanged), this, &UserMotionLabelsEditor::onFighterSelected);
}

// ----------------------------------------------------------------------------
UserMotionLabelsEditor::~UserMotionLabelsEditor()
{}

// ----------------------------------------------------------------------------
void UserMotionLabelsEditor::populateFromGlobalData(rfcommon::MappingInfo* globalMappingInfo)
{
    // Create sorted list of all fighters
    struct FighterData {
        rfcommon::FighterID id;
        rfcommon::String name;
    };
    rfcommon::Vector<FighterData> fighterData;
    auto ids = globalMappingInfo->fighter.IDs();
    auto names = globalMappingInfo->fighter.names();
    for (int i {}; i != ids.count(); ++i)
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
    auto idAlreadyAdded = [this](rfcommon::FighterID fighterID) -> bool {
        for (auto entry : indexToFighterID_)
            if (entry == fighterID)
                return true;
        return false;
    };

    // Create sorted list of all fighters
    rfcommon::Vector<rfcommon::String> fighterNames;
    for (int i {}; i != sessionCount; ++i)
        if (const auto mdata = loadedSessions[i]->tryGetMetaData())
            if (const auto map = loadedSessions[i]->tryGetMappingInfo())
                for (int f {}; f != mdata->fighterCount(); ++f)
                {
                    auto fighterID = mdata->fighterID(f);
                    if (idAlreadyAdded(fighterID))
                        continue;
                    fighterNames.push(map->fighter.toName(fighterID));
                    indexToFighterID_.push(fighterID);
                }

    std::sort(fighterNames.begin(), fighterNames.end());

    // Add to dropdown
    for (const auto& name : fighterNames)
        comboBox_fighters->addItem(name.cStr());

    for (int i {}; i != sessionCount; ++i)
        if (const auto mdata = loadedSessions[i]->tryGetMetaData())
            if (const auto fdata = loadedSessions[i]->tryGetFrameData())
                for (int fighterIdx {}; fighterIdx != fdata->fighterCount(); ++fighterIdx)
                    for (int frameIdx {}; frameIdx != fdata->frameCount(); ++frameIdx)
                    {
                        const auto motion = fdata->stateAt(fighterIdx, frameIdx).motion();
                        manager_->userMotionLabels()->addUnknownMotion(mdata->fighterID(fighterIdx), motion);
                    }

    auto model = static_cast<TableModel*>(tableModels_[rfcommon::FighterUserMotionLabels::UNLABELED]);
    if (const auto mdata = loadedSessions[0]->tryGetMetaData())
        model->setFighter(mdata->fighterID(0));
}

// ----------------------------------------------------------------------------
void UserMotionLabelsEditor::closeEvent(QCloseEvent* event)
{

}

// ----------------------------------------------------------------------------
void UserMotionLabelsEditor::onFighterSelected(int index)
{
    auto model = static_cast<TableModel*>(tableModels_[rfcommon::FighterUserMotionLabels::UNLABELED]);
    model->setFighter(indexToFighterID_[index]);
}

// ----------------------------------------------------------------------------
void UserMotionLabelsEditor::onCustomContextMenuRequested(int tabIdx, const QPoint& globalPos)
{
    QMenu menu;
    QAction* newLayer = menu.addAction("Create New Layer");
    QAction* a = menu.exec(globalPos);

    if (a == newLayer)
    {
        QString name = QInputDialog::getText(this, "Enter Name", "Layer Name:");
        if (name.length() == 0)
            return;
        QByteArray ba = name.toUtf8();
        manager_->userMotionLabels()->newEmptyLayer(ba.constData());
    }
}

}
