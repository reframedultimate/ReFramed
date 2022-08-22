#pragma once

#include "rfcommon/Reference.hpp"
#include <QDialog>

namespace Ui {
    class VideoAssociatorDialog;
}

namespace rfcommon {
    class MappedFile;
    class Plugin;
    class Session;
}

namespace rfapp {

class PluginManager;
class ReplayManager;

class VideoAssociatorDialog : public QDialog
{
    Q_OBJECT

public:
    VideoAssociatorDialog(
            PluginManager* pluginManager,
            ReplayManager* replayManager,
            rfcommon::Session* session,
            const QString& currentFileName,
            QWidget* parent=nullptr);
    ~VideoAssociatorDialog();

private slots:
    void onSaveReleased();
    void onChooseFileReleased();

private:
    Ui::VideoAssociatorDialog* ui_;
    PluginManager* pluginManager_;
    ReplayManager* replayManager_;
    rfcommon::Plugin* videoPlugin_;
    QWidget* videoView_;
    rfcommon::Reference<rfcommon::Session> session_;
    rfcommon::Reference<rfcommon::MappedFile> currentVideoFile_;
    const QString currentFileName_;
};

}