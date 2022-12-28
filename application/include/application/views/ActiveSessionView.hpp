#pragma once

#include <QWidget>
#include "application/listeners/ActiveSessionManagerListener.hpp"

class QGroupBox;
class QLabel;
class QScrollArea;

namespace rfcommon {
    class Session;
}

namespace rfapp {

class ActiveSessionManager;
class CollapsibleSplitter;
class MetadataEditModel;
class PlayerDetails;
class PluginManager;
class Protocol;

class ActiveSessionView
    : public QWidget
    , public ActiveSessionManagerListener
{
    Q_OBJECT
public:
    ActiveSessionView(
            ActiveSessionManager* activeSessionManager,
            PluginManager* pluginManager,
            PlayerDetails* playerDetails,
            QWidget* parent=nullptr);
    ~ActiveSessionView();

    void toggleSideBar();

private:
    void onActiveSessionManagerGameStarted(rfcommon::Session* game) override;
    void onActiveSessionManagerGameEnded(rfcommon::Session* game) override;
    void onActiveSessionManagerTrainingStarted(rfcommon::Session* training) override;
    void onActiveSessionManagerTrainingEnded(rfcommon::Session* training) override;

private:
    ActiveSessionManager* activeSessionManager_;
    std::unique_ptr<MetadataEditModel> metadataEditModel_;
    QScrollArea* scrollArea_;
    CollapsibleSplitter* splitter_;
    int size0_, size1_;
};

}
