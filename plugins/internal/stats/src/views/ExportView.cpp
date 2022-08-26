#include "ui_ExportView.h"
#include "stats/models/SettingsModel.hpp"
#include "stats/views/ExportView.hpp"

#include <QFileDialog>

// ----------------------------------------------------------------------------
ExportView::ExportView(SettingsModel* model, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::ExportView)
    , settings_(model)
{
    ui_->setupUi(this);

    // Set UI state based on settings
    ui_->radioButton_resetStatsEveryGame->setChecked(settings_->resetBehavior() == SettingsModel::RESET_EACH_GAME);
    ui_->radioButton_resetStatsEverySet->setChecked(settings_->resetBehavior() == SettingsModel::RESET_EACH_SET);
    ui_->checkBox_obsExport->setChecked(settings_->exportToOBS());
    ui_->lineEdit_obsDestFolder->setText(settings_->destinationFolderOBS().absolutePath());
    ui_->checkBox_obsInsertNewlines->setChecked(settings_->additionalNewlinesOBS() > 0);
    ui_->spinBox_obsNewlines->setValue(settings_->additionalNewlinesOBS());
    ui_->radioButton_obsExportAfterEachGame->setChecked(settings_->exportIntervalOBS() == 0);
    ui_->radioButton_obsExportAfterInterval->setChecked(settings_->exportIntervalOBS() > 0);
    ui_->spinBox_obsSeconds->setValue(settings_->exportIntervalOBS());

    // Enable UI elements based on settings
    ui_->label_obsDestFolder->setEnabled(settings_->exportToOBS());
    ui_->lineEdit_obsDestFolder->setEnabled(settings_->exportToOBS());
    ui_->toolButton_obsBrowseFolder->setEnabled(settings_->exportToOBS());
    ui_->checkBox_obsInsertNewlines->setEnabled(settings_->exportToOBS());
    ui_->spinBox_obsNewlines->setEnabled(settings_->exportToOBS() && settings_->additionalNewlinesOBS() > 0);
    ui_->radioButton_obsExportAfterEachGame->setEnabled(settings_->exportToOBS());
    ui_->radioButton_obsExportAfterInterval->setEnabled(settings_->exportToOBS());
    ui_->spinBox_obsSeconds->setEnabled(settings_->exportToOBS() && settings_->exportIntervalOBS() > 0);
    ui_->label_obsSeconds->setEnabled(settings_->exportToOBS() && settings_->exportIntervalOBS() > 0);

    connect(ui_->radioButton_resetStatsEveryGame, &QRadioButton::toggled, this, &ExportView::onResetEachGameToggled);

    connect(ui_->checkBox_obsExport, &QCheckBox::toggled, this, &ExportView::onOBSCheckBoxToggled);
    connect(ui_->checkBox_obsInsertNewlines, &QCheckBox::toggled, this, &ExportView::onOBSInsertNewLinesCheckBoxToggled);
    connect(ui_->spinBox_obsNewlines, qOverload<int>(&QSpinBox::valueChanged), this, &ExportView::onOBSSpinBoxNewLinesChanged);
    connect(ui_->toolButton_obsBrowseFolder, &QToolButton::released, this, &ExportView::onOBSBrowseFolderButtonReleased);
    connect(ui_->radioButton_obsExportAfterEachGame, &QRadioButton::toggled, this, &ExportView::onOBSExportAfterEachGameToggled);
    connect(ui_->spinBox_obsSeconds, qOverload<int>(&QSpinBox::valueChanged), this, &ExportView::onOBSExportIntervalValueChanged);
}

// ----------------------------------------------------------------------------
ExportView::~ExportView()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void ExportView::onResetEachGameToggled(bool eachGame)
{
    settings_->setResetBehavior(eachGame ?
        SettingsModel::RESET_EACH_GAME :
        SettingsModel::RESET_EACH_SET);
}

// ----------------------------------------------------------------------------
void ExportView::onOBSCheckBoxToggled(bool enable)
{
    ui_->label_obsDestFolder->setEnabled(enable);
    ui_->lineEdit_obsDestFolder->setEnabled(enable);
    ui_->toolButton_obsBrowseFolder->setEnabled(enable);
    ui_->checkBox_obsInsertNewlines->setEnabled(enable);
    ui_->spinBox_obsNewlines->setEnabled(enable && ui_->checkBox_obsInsertNewlines->isChecked());
    ui_->radioButton_obsExportAfterEachGame->setEnabled(enable);
    ui_->radioButton_obsExportAfterInterval->setEnabled(enable);
    ui_->spinBox_obsSeconds->setEnabled(enable && ui_->radioButton_obsExportAfterInterval->isChecked());
    ui_->label_obsSeconds->setEnabled(enable && ui_->radioButton_obsExportAfterInterval->isChecked());

    settings_->setExportToOBS(enable);
}

// ----------------------------------------------------------------------------
void ExportView::onOBSInsertNewLinesCheckBoxToggled(bool enable)
{
    ui_->spinBox_obsNewlines->setEnabled(enable);

    settings_->setAdditionalNewlinesOBS(enable ? ui_->spinBox_obsNewlines->value() : 0);
}

// ----------------------------------------------------------------------------
void ExportView::onOBSSpinBoxNewLinesChanged(int value)
{
    settings_->setAdditionalNewlinesOBS(value);
}

// ----------------------------------------------------------------------------
void ExportView::onOBSBrowseFolderButtonReleased()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Destination Folder");
    ui_->lineEdit_obsDestFolder->setText(dir);

    settings_->setDestinationFolderOBS(dir);
}

// ----------------------------------------------------------------------------
void ExportView::onOBSExportAfterEachGameToggled(bool checked)
{
    ui_->spinBox_obsSeconds->setEnabled(!checked);

    settings_->setExportIntervalOBS(checked ?
        0 :
        ui_->spinBox_obsSeconds->value());
}

// ----------------------------------------------------------------------------
void ExportView::onOBSExportIntervalValueChanged(int value)
{
    settings_->setExportIntervalOBS(value);
}
