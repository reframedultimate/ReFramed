#pragma once

#include "stats/StatType.hpp"
#include "stats/listeners/WebSocketServerListener.hpp"
#include <QWidget>

class SettingsModel;
class SettingsDragWidget;
class WebSocketServer;

namespace Ui {
    class ExportView;
}

class ExportView 
    : public QWidget
    , public WebSocketServerListener
{
    Q_OBJECT

public:
    explicit ExportView(SettingsModel* model, WebSocketServer* wsServer, QWidget* parent=nullptr);
    ~ExportView();

private slots:
    void onResetEachGameToggled(bool eachGame);

    void onOBSEnabledToggled(bool checked);
    void onOBSInsertNewLinesCheckBoxToggled(bool enable);
    void onOBSSpinBoxNewLinesChanged(int value);
    void onOBSBrowseFolderButtonReleased();
    void onOBSExportAfterEachGameToggled(bool checked);
    void onOBSExportIntervalValueChanged(int value);

    void onWSEnabledToggled(bool checked);
    void onWSAutoStartToggled(bool checked);
    void onWSSecureModeToggled(bool checked);
    void onWSStartStopReleased();

private:
    void onWSServerStarting() override;
    void onWSServerFailedToStart(const QString& error) override;
    void onWSServerStarted(const QString& host, uint16_t port) override;
    void onWSServerStopped() override;

    void onWSClientConnected(const QString& address, uint16_t port) override;
    void onWSClientError(const QString& error) override;

private:
    Ui::ExportView* ui_;
    SettingsModel* settings_;
    WebSocketServer* wsServer_;
};
