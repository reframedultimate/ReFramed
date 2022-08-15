#include "application/views/UserLabelsEditor.hpp"

#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/UserLabels.hpp"

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

// ----------------------------------------------------------------------------
UserLabelsEditor::UserLabelsEditor(rfcommon::UserLabels* userLabels, QWidget* parent)
    : QDialog(parent)
    , userLabels_(userLabels)
    , comboBox_fighters(new QComboBox)
{
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
#define X(name, str) \
        tables_.push(new QTableView); \
        tabWidget->addTab(tables_.back(), str);
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

    connect(closeButton, &QPushButton::released, this, &UserLabelsEditor::close);
}

// ----------------------------------------------------------------------------
void UserLabelsEditor::populateFromGlobalData(rfcommon::MappingInfo* globalMappingInfo)
{
    // Create sorted list of all fighters
    auto fighterNames = globalMappingInfo->fighter.names();
    std::sort(fighterNames.begin(), fighterNames.end());

    // Add to dropdown
    for (const auto& name : fighterNames)
        comboBox_fighters->addItem(name.cStr());
}

// ----------------------------------------------------------------------------
void UserLabelsEditor::populateFromSessions(rfcommon::Session** loadedSessions, int sessionCount)
{
    // Create sorted list of all fighters
    rfcommon::Vector<rfcommon::String> fighterNames;
    for (int i = 0; i != sessionCount; ++i)
        if (const auto mdata = loadedSessions[i]->tryGetMappingInfo())
            fighterNames.push(mdata->fighter.names());

    std::sort(fighterNames.begin(), fighterNames.end());

    // Add to dropdown
    for (const auto& name : fighterNames)
        comboBox_fighters->addItem(name.cStr());
}

// ----------------------------------------------------------------------------
UserLabelsEditor::~UserLabelsEditor()
{}

// ----------------------------------------------------------------------------
void UserLabelsEditor::closeEvent(QCloseEvent* event)
{

}

}
