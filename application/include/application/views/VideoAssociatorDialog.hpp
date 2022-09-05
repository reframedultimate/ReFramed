#pragma once

#include "rfcommon/Reference.hpp"
#include <QDialog>
#include <QTimer>

class QShortcut;

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
            const QString& currentSessionFileName,
            QWidget* parent=nullptr);
    ~VideoAssociatorDialog();

private slots:
    void onSaveReleased();
    void onChooseFileReleased();
    void onPlayToggled();
    void onNextFrame();
    void onPrevFrame();

    void updateTimesFromVideo();

private:
    Ui::VideoAssociatorDialog* ui_;
    PluginManager* pluginManager_;
    ReplayManager* replayManager_;
    rfcommon::Plugin* videoPlugin_;
    QWidget* videoView_;
    rfcommon::Reference<rfcommon::Session> session_;
    const QString currentSessionFileName_;

    rfcommon::Reference<rfcommon::MappedFile> currentVideoFile_;
    QString currentVideoFileName_;
    QString currentVideoFilePath_;

    QShortcut* togglePlayShortcut_ = nullptr;
    QShortcut* nextFrameShortcut_ = nullptr;
    QShortcut* prevFrameShortcut_ = nullptr;
    QTimer timer_;
};

}
