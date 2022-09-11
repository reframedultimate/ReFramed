#pragma once

#include <QDialog>

namespace Ui {
    class ImportReplayPackDialog;
}

namespace rfapp {

class ReplayManager;

class ImportReplayPackDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportReplayPackDialog(ReplayManager* replayManager, QWidget* parent=nullptr);
    ~ImportReplayPackDialog();

private:
    void checkEnableImportButton();

private slots:
    void onSelectReplayPack();
    void onSelectReplayDir();
    void onSelectVideoDir();
    void onExtractVideoSettingChanged();
    void onTargetReplayGroupChanged(int index);
    void onNewGroupNameChanged(const QString& name);

    void onImport();

private:
    Ui::ImportReplayPackDialog* ui_;
    ReplayManager* replayManager_;
};

}
