#pragma once

#include <QWidget>
#include "application/listeners/ActiveSessionManagerListener.hpp"
#include "application/models/ConfigAccessor.hpp"

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
class MetadataEditWidget;
class PlayerDetails;
class PluginManager;
class Protocol;

class ActiveSessionView
        : public QWidget
        , public ActiveSessionManagerListener
        , public ConfigAccessor
{
    Q_OBJECT
public:
    ActiveSessionView(
            Config* config,
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
    QVector<MetadataEditWidget*> metadataEditWidgets_;
    int size0_, size1_;
};

}
