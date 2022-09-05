#include "ui_ExportView.h"
#include "stats/config.hpp"
#include "stats/models/SettingsModel.hpp"
#include "stats/models/WebSocketServer.hpp"
#include "stats/views/ExportView.hpp"

#include <QFileDialog>

// ----------------------------------------------------------------------------
ExportView::ExportView(SettingsModel* model, WebSocketServer* wsServer, QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::ExportView)
    , settings_(model)
    , wsServer_(wsServer)
{
    ui_->setupUi(this);

    // Set OBS UI state based on settings
    ui_->radioButton_resetStatsEveryGame->setChecked(settings_->resetBehavior() == SettingsModel::RESET_EACH_GAME);
    ui_->radioButton_resetStatsEverySet->setChecked(settings_->resetBehavior() == SettingsModel::RESET_EACH_SET);
    ui_->checkBox_obsExport->setChecked(settings_->obsEnabled());
    ui_->lineEdit_obsDestFolder->setText(settings_->obsDestinationFolder().absolutePath());
    ui_->checkBox_obsInsertNewlines->setChecked(settings_->obsAdditionalNewlines() > 0);
    ui_->spinBox_obsNewlines->setValue(settings_->obsAdditionalNewlines());
    ui_->radioButton_obsExportAfterEachGame->setChecked(settings_->obsExportInterval() == 0);
    ui_->radioButton_obsExportAfterInterval->setChecked(settings_->obsExportInterval() > 0);
    ui_->spinBox_obsSeconds->setValue(settings_->obsExportInterval());

    // Enable OBS UI elements based on settings
    ui_->label_obsDestFolder->setEnabled(settings_->obsEnabled());
    ui_->lineEdit_obsDestFolder->setEnabled(settings_->obsEnabled());
    ui_->toolButton_obsBrowseFolder->setEnabled(settings_->obsEnabled());
    ui_->checkBox_obsInsertNewlines->setEnabled(settings_->obsEnabled());
    ui_->spinBox_obsNewlines->setEnabled(settings_->obsEnabled() && settings_->obsAdditionalNewlines() > 0);
    ui_->radioButton_obsExportAfterEachGame->setEnabled(settings_->obsEnabled());
    ui_->radioButton_obsExportAfterInterval->setEnabled(settings_->obsEnabled());
    ui_->spinBox_obsSeconds->setEnabled(settings_->obsEnabled() && settings_->obsExportInterval() > 0);
    ui_->label_obsSeconds->setEnabled(settings_->obsEnabled() && settings_->obsExportInterval() > 0);

#if defined(STATS_WEBSOCKET_SERVER)
    // Set WebSocket UI state based on settings
    ui_->checkBox_wsEnable->setChecked(settings_->wsEnabled());
    ui_->checkBox_wsAutoStart->setChecked(settings_->wsAutoStart());
    ui_->lineEdit_wsAddress->setText(settings_->wsHostName());
    ui_->spinBox_wsPort->setValue(settings_->wsPort());
    ui_->checkBox_wsSecureMode->setChecked(settings_->wsSecureMode());

    // Enable WebSocket UI elements based on settings
    ui_->checkBox_wsAutoStart->setEnabled(settings_->wsEnabled());
    ui_->label_wsAddress->setEnabled(settings_->wsEnabled());
    ui_->label_wsPort->setEnabled(settings_->wsEnabled());
    ui_->lineEdit_wsAddress->setEnabled(settings_->wsEnabled());
    ui_->spinBox_wsPort->setEnabled(settings_->wsEnabled());
    ui_->checkBox_wsSecureMode->setEnabled(settings_->wsEnabled());
    ui_->pushButton_wsStartStop->setEnabled(settings_->wsEnabled());
#else
    ui_->groupBox_webSocketServer->setVisible(false);
#endif

    // Stats calculator UI events
    connect(ui_->radioButton_resetStatsEveryGame, &QRadioButton::toggled, this, &ExportView::onResetEachGameToggled);

    // OBS UI events
    connect(ui_->checkBox_obsExport, &QCheckBox::toggled, this, &ExportView::onOBSEnabledToggled);
    connect(ui_->checkBox_obsInsertNewlines, &QCheckBox::toggled, this, &ExportView::onOBSInsertNewLinesCheckBoxToggled);
    connect(ui_->spinBox_obsNewlines, qOverload<int>(&QSpinBox::valueChanged), this, &ExportView::onOBSSpinBoxNewLinesChanged);
    connect(ui_->toolButton_obsBrowseFolder, &QToolButton::released, this, &ExportView::onOBSBrowseFolderButtonReleased);
    connect(ui_->radioButton_obsExportAfterEachGame, &QRadioButton::toggled, this, &ExportView::onOBSExportAfterEachGameToggled);
    connect(ui_->spinBox_obsSeconds, qOverload<int>(&QSpinBox::valueChanged), this, &ExportView::onOBSExportIntervalValueChanged);

#if defined(STATS_WEBSOCKET_SERVER)
    // WebSocket UI events
    connect(ui_->checkBox_wsEnable, &QCheckBox::toggled, this, &ExportView::onWSEnabledToggled);
    connect(ui_->checkBox_wsAutoStart, &QCheckBox::toggled, this, &ExportView::onWSAutoStartToggled);
    connect(ui_->checkBox_wsSecureMode, &QCheckBox::toggled, this, &ExportView::onWSSecureModeToggled);
    connect(ui_->pushButton_wsStartStop, &QPushButton::released, this, &ExportView::onWSStartStopReleased);

    wsServer_->dispatcher.addListener(this);

    if (settings_->wsAutoStart())
        ExportView::onWSStartStopReleased();
#endif
}

// ----------------------------------------------------------------------------
ExportView::~ExportView()
{
#if defined(STATS_WEBSOCKET_SERVER)
    wsServer_->dispatcher.removeListener(this);
#endif

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
void ExportView::onOBSEnabledToggled(bool enable)
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

    settings_->obsSetEnabled(enable);
}

// ----------------------------------------------------------------------------
void ExportView::onOBSInsertNewLinesCheckBoxToggled(bool enable)
{
    ui_->spinBox_obsNewlines->setEnabled(enable);

    settings_->obsSetAdditionalNewlines(enable ? ui_->spinBox_obsNewlines->value() : 0);
}

// ----------------------------------------------------------------------------
void ExportView::onOBSSpinBoxNewLinesChanged(int value)
{
    settings_->obsSetAdditionalNewlines(value);
}

// ----------------------------------------------------------------------------
void ExportView::onOBSBrowseFolderButtonReleased()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Destination Folder");
    if (dir.length() == 0)
        return;

    ui_->lineEdit_obsDestFolder->setText(dir);
    settings_->obsSetDestinationFolder(dir);
}

// ----------------------------------------------------------------------------
void ExportView::onOBSExportAfterEachGameToggled(bool checked)
{
    ui_->spinBox_obsSeconds->setEnabled(!checked);

    settings_->obsSetExportInterval(checked ?
        0 :
        ui_->spinBox_obsSeconds->value());
}

// ----------------------------------------------------------------------------
void ExportView::onOBSExportIntervalValueChanged(int value)
{
    settings_->obsSetExportInterval(value);
}

// ----------------------------------------------------------------------------
void ExportView::onWSEnabledToggled(bool checked)
{
    ui_->checkBox_wsAutoStart->setEnabled(checked);
    ui_->label_wsAddress->setEnabled(checked);
    ui_->label_wsPort->setEnabled(checked);
    ui_->lineEdit_wsAddress->setEnabled(checked);
    ui_->spinBox_wsPort->setEnabled(checked);
    ui_->checkBox_wsSecureMode->setEnabled(checked);
    ui_->pushButton_wsStartStop->setEnabled(checked);

    settings_->wsSetEnabled(checked);
}

// ----------------------------------------------------------------------------
void ExportView::onWSAutoStartToggled(bool checked)
{
    settings_->wsSetAutoStart(checked);
}

// ----------------------------------------------------------------------------
void ExportView::onWSSecureModeToggled(bool checked)
{
    settings_->wsSetSecureMode(checked);
}

// ----------------------------------------------------------------------------
void ExportView::onWSStartStopReleased()
{
    if (wsServer_->isRunning())
    {
        wsServer_->stopServer();
    }
    else
    {
        QString host = ui_->lineEdit_wsAddress->text();
        uint16_t port = ui_->spinBox_wsPort->value();
        settings_->wsSetHostNameAndPort(host, port);
        wsServer_->startServer(host, port, settings_->wsSecureMode());
    }
}

// ----------------------------------------------------------------------------
void ExportView::onWSServerStarting()
{
    ui_->label_wsStatus->setText("Starting server...");

    ui_->pushButton_wsStartStop->setEnabled(false);
    ui_->lineEdit_wsAddress->setReadOnly(true);
    ui_->spinBox_wsPort->setReadOnly(true);
    ui_->checkBox_wsSecureMode->setEnabled(false);
}

// ----------------------------------------------------------------------------
void ExportView::onWSServerFailedToStart(const QString& error)
{
    ui_->label_wsStatus->setText("Failed to start: " + error);

    ui_->pushButton_wsStartStop->setEnabled(true);
    ui_->lineEdit_wsAddress->setReadOnly(false);
    ui_->spinBox_wsPort->setReadOnly(false);
    ui_->checkBox_wsSecureMode->setEnabled(true);
}

// ----------------------------------------------------------------------------
void ExportView::onWSServerStarted(const QString& host, uint16_t port)
{
    ui_->label_wsStatus->setText("Server is running");

    ui_->pushButton_wsStartStop->setEnabled(true);
    ui_->pushButton_wsStartStop->setText("Stop Server");
    ui_->lineEdit_wsAddress->setText(host);
    ui_->spinBox_wsPort->setValue(port);
}

// ----------------------------------------------------------------------------
void ExportView::onWSServerStopped()
{
    ui_->label_wsStatus->setText("Stopped.");

    ui_->pushButton_wsStartStop->setText("Start Server");
    ui_->lineEdit_wsAddress->setReadOnly(false);
    ui_->spinBox_wsPort->setReadOnly(false);
    ui_->checkBox_wsSecureMode->setEnabled(true);
}

// ----------------------------------------------------------------------------
void ExportView::onWSClientConnected(const QString& address, uint16_t port)
{
    ui_->label_wsStatus->setText("Client connected: " + address + ":" + QString::number(port));
}

// ----------------------------------------------------------------------------
void ExportView::onWSClientError(const QString& error)
{
    ui_->label_wsStatus->setText(error);
}
