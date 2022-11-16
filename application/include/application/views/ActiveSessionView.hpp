#pragma once

#include <QWidget>
#include "application/listeners/ActiveSessionManagerListener.hpp"

class QGroupBox;
class QLabel;

namespace rfcommon {
    class Session;
}

namespace rfapp {

class ActiveSessionManager;
class MetaDataEditModel;
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
            QWidget* parent=nullptr);
    ~ActiveSessionView();

private:
    void onActiveSessionManagerGameStarted(rfcommon::Session* game) override;
    void onActiveSessionManagerGameEnded(rfcommon::Session* game) override;
    void onActiveSessionManagerTrainingStarted(rfcommon::Session* training) override;
    void onActiveSessionManagerTrainingEnded(rfcommon::Session* training) override;

private:
    ActiveSessionManager* activeSessionManager_;
    std::unique_ptr<MetaDataEditModel> metaDataEditModel_;
};

}
