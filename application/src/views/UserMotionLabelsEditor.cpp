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

namespace rfapp {

namespace {

class TableModel : public QAbstractTableModel
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
    }

    void setFighter(rfcommon::FighterID fighterID)
    {
        beginResetModel();
            fighterID_ = fighterID;
            repopulateEntries();
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
                            case 2:
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
                    case 2: return table_[index.row()].label;
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

private:
    void repopulateEntries()
    {
        table_.clear();
        if (fighterID_.isValid() == false)
            return;

        for (int i = 0; i != labels_->entryCount(fighterID_); ++i)
        {
            if (labels_->categoryAt(fighterID_, i) != category_)
                continue;

            auto motion = labels_->motionAt(fighterID_, i);
            auto& entry = table_.emplace();
            entry.value = "0x" + QString::number(motion.value(), 16);
            entry.string = hash40Strings_->toString(motion);
            if (labels_->layerCount() > 0)
                entry.label = labels_->userLabelAt(fighterID_, 0, i);
        }

        std::sort(table_.begin(), table_.end(), [](const Entry& a, const Entry& b){
            return a.string < b.string;
        });
    }

private:
    struct Entry
    {
        QString value;
        QString string;
        QString label;
    };

    rfcommon::UserMotionLabels* labels_;
    rfcommon::Hash40Strings* hash40Strings_;
    rfcommon::Vector<Entry> table_;
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

    connect(closeButton, &QPushButton::released, this, &UserMotionLabelsEditor::close);
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

    auto model = static_cast<TableModel*>(tableModels_[rfcommon::FighterUserMotionLabels::UNLABELED]);
    model->setFighter(indexToFighterID_[0]);
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
    for (int i = 0; i != sessionCount; ++i)
        if (const auto mdata = loadedSessions[i]->tryGetMetaData())
            if (const auto map = loadedSessions[i]->tryGetMappingInfo())
                for (int f = 0; f != mdata->fighterCount(); ++f)
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

    for (int i = 0; i != sessionCount; ++i)
        if (const auto mdata = loadedSessions[i]->tryGetMetaData())
            if (const auto fdata = loadedSessions[i]->tryGetFrameData())
                for (int fighterIdx = 0; fighterIdx != fdata->fighterCount(); ++fighterIdx)
                    for (int frameIdx = 0; frameIdx != fdata->frameCount(); ++frameIdx)
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

}
