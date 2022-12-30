#pragma once

#include <QDialog>

class QVBoxLayout;
class QToolButton;

namespace rfcommon {
    class FilePathResolver;
}

namespace rfapp {

class ReplayManager;

class PathManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PathManagerDialog(ReplayManager* replayManager, rfcommon::FilePathResolver* pathResolver, QWidget* parent);
    ~PathManagerDialog();

private slots:
    void onFindReplaysReleased();
    void onRemoveUnusedReplayPathsReleased();
    void onFindMissingVideosReleased();
    void onRemoveUnusedVideoPathsReleased();
    void onSaveReleased();

private:
    void addReplayPath(const QString& path);
    void addVideoPath(const QString& path);

private:
    ReplayManager* replayManager_;
    rfcommon::FilePathResolver* pathResolver_;
    QVBoxLayout* replayPathsLayout_;
    QVBoxLayout* videoPathsLayout_;
    QStringList replayPaths_;
    QStringList videoPaths_;
    QString defaultReplayPath_;
};

}
