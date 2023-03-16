#include "application/ui_MotionLabelsEditor.h"
#include "application/Util.hpp"
#include "application/models/Protocol.hpp"
#include "application/models/MotionLabelsManager.hpp"
#include "application/models/MotionLabelsTableModel.hpp"
#include "application/views/MainWindow.hpp"
#include "application/views/MotionLabelsEditor.hpp"
#include "application/views/MotionLabelsTableView.hpp"
#include "application/views/PropagateLabelsDialog.hpp"

#include "rfcommon/FighterState.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"

#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>

namespace rfapp {

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

    // Because the motion labels editor has the option to discard changes, which
    // is implemented by reloading the motion labels file, it makes sense to
    // save any pending changes when opening the editor, because otherwise
    // we could discard data added outside of our control.
    manager_->saveChanges();

    // Tabs with different categories
#define X(category, name) \
        tableViews_.push(new MotionLabelsTableView); \
        tableViews_.back()->setModel(new MotionLabelsTableModel( \
                rfcommon::FighterID::fromValue(0), \
                rfcommon::MotionLabels::category, \
                manager_->motionLabels(), \
                protocol)); \
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
        for (auto view : tableViews_)
            static_cast<MotionLabelsTableModel*>(view->model())->setFighter(indexToFighterID_[62]);
    }

    // Sort by hash40 string by default
    if (ui_->comboBox_fighters->count() > 0)
    {
        for (auto view : tableViews_)
            view->sortByColumn(1, Qt::AscendingOrder);
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

    connect(ui_->comboBox_fighters, qOverload<int>(&QComboBox::currentIndexChanged), this, &MotionLabelsEditor::onFighterSelected);
    connect(ui_->pushButton_nextConflict, &QPushButton::released, [this] { highlightNextConflict(1); });
    connect(ui_->pushButton_prevConflict, &QPushButton::released, [this] { highlightNextConflict(-1); });

    connect(ui_->pushButton_cancel, &QPushButton::released, this, &MotionLabelsEditor::onCancelReleased);
    connect(ui_->pushButton_save, &QPushButton::released, this, &MotionLabelsEditor::onSaveReleased);
    connect(ui_->pushButton_saveAndClose, &QPushButton::released, this, &MotionLabelsEditor::onSaveAndCloseReleased);

    manager_->motionLabels()->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
MotionLabelsEditor::~MotionLabelsEditor()
{
    manager_->motionLabels()->dispatcher.removeListener(this);

    // Models are not deleted automatically with Qt's model-view
    for (auto view : tableViews_)
    {
        auto model = view->model();
        delete view;
        delete model;
    }
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

    auto model = static_cast<MotionLabelsTableModel*>(tableViews_[rfcommon::MotionLabels::UNLABELED]->model());
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

    for (int i = 0; i != tableViews_.count(); ++i)
    {
        auto model = static_cast<MotionLabelsTableModel*>(tableViews_[i]->model());
        model->setFighter(indexToFighterID_[index]);
    }
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
    QAction* propagateLabel = menu.addAction("Propagate labels to other fighters");
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

    QSet<int> selectedColumns, selectedRows;
    for (const auto& index : selectedIndexes)
        if (index.column() >= 2)  // Can't operate on hash40 or on string columns
        {
            selectedColumns.insert(index.column());
            selectedRows.insert(index.row());
        }

    // If nothing is selected, disable menu options accordingly
    if (selectedColumns.size() == 0)
    {
        setLabel->setEnabled(false);
        renameLayer->setEnabled(false);
        mergeLayers->setEnabled(false);
        changeUsage->setEnabled(false);
        deleteLayer->setEnabled(false);
        exportLayer->setEnabled(false);
    }
    if (selectedRows.size() == 0)
    {
        changeCategory->setEnabled(false);
        propagateLabel->setEnabled(false);
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
        auto model = static_cast<MotionLabelsTableModel*>(tableViews_[tabIdx]->model());
        model->setLabels(selectedIndexes, name);
    }
    else if (a == propagateLabel)
    {
        PropagateLabelsDialog dialog(this);
        dialog.setGeometry(calculatePopupGeometryKeepSize(this, &dialog, dialog.geometry()));
        if (dialog.exec() != QDialog::Accepted)
            return;

        auto model = static_cast<MotionLabelsTableModel*>(tableViews_[tabIdx]->model());
        int updated = model->propagateLabels(selectedIndexes, dialog.overwriteExisting(), dialog.forceCreation());
        QMessageBox::information(this, "Complete", "Updated " + QString::number(updated) + " labels");
    }
#define X(name, str) \
    else if (a == cat##name) \
    { \
        auto model = static_cast<MotionLabelsTableModel*>(tableViews_[tabIdx]->model()); \
        model->setCategory(selectedRows, rfcommon::MotionLabels::name); \
    }
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

        currentConflictTableIdx_ = -1;
        if (highlightNextConflict(1) == false)
            ui_->label_conflicts->setVisible(false);
    }
    else if (a == importLayer)
    {
        QString filePath = QFileDialog::getOpenFileName(this, "Import layer", "", "Layers file (*.json)");
        if (filePath.isEmpty())
            return;

        if (manager_->motionLabels()->importLayers(filePath.toUtf8().constData()) >= 0)
        {
            if (highlightNextConflict(1) == false)
                ui_->label_conflicts->setVisible(false);
        }
        else
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
void MotionLabelsEditor::onCancelReleased()
{
    // Should reload the file and overwrite any changes we made
    manager_->discardChanges();
    close();
}

// ----------------------------------------------------------------------------
void MotionLabelsEditor::onSaveReleased()
{
    manager_->saveChanges();
}

// ----------------------------------------------------------------------------
void MotionLabelsEditor::onSaveAndCloseReleased()
{
    manager_->saveChanges();
    close();
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
        auto view = tableViews_[categoryIdx];
        auto model = static_cast<MotionLabelsTableModel*>(view->model());

        currentConflictTableIdx_ = model->findNextConflict(currentConflictTableIdx_, direction);
        if (currentConflictTableIdx_ >= 0)
        {
            if (fighterIdx != ui_->comboBox_fighters->currentIndex())
            {
                ui_->comboBox_fighters->setCurrentIndex(fighterIdx);
                onFighterSelected(fighterIdx);
            }

            ui_->tabWidget_categories->setCurrentIndex(categoryIdx);
            
            QModelIndex index = model->index(currentConflictTableIdx_, 0);
            view->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            view->scrollTo(index);
            view->setFocus();

            ui_->pushButton_nextConflict->setVisible(true);
            ui_->pushButton_prevConflict->setVisible(true);

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

        model = static_cast<MotionLabelsTableModel*>(tableViews_[categoryIdx]->model());
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
