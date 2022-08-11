#include "application/views/ReplayViewer.hpp"
#include "application/models/PluginManager.hpp"
#include "application/models/Protocol.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/MetaData.hpp"
#include <QTabBar>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QStyle>
#include <QApplication>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayViewer::ReplayViewer(PluginManager* pluginManager, QWidget* parent)
    : QTabWidget(parent)
    , protocol_(nullptr)
    , pluginManager_(pluginManager)
    , sessionState_(DISCONNECTED)
    , activeSessionState_(NO_ACTIVE_SESSION)
    , replayState_(NONE_LOADED)
    , previousTab_(0)
{
    addTab(new QWidget, "+");

    connect(this, &QTabWidget::tabBarClicked, this, &ReplayViewer::onTabBarClicked);
    connect(this, &QTabWidget::currentChanged, this, &ReplayViewer::onCurrentTabChanged);
}

// ----------------------------------------------------------------------------
ReplayViewer::ReplayViewer(Protocol* protocol, PluginManager* pluginManager, QWidget* parent)
    : QTabWidget(parent)
    , protocol_(protocol)
    , pluginManager_(pluginManager)
    , sessionState_(DISCONNECTED)
    , activeSessionState_(NO_ACTIVE_SESSION)
    , replayState_(NONE_LOADED)
    , previousTab_(0)
{
    addTab(new QWidget, "+");

    protocol_->dispatcher.addListener(this);

    connect(this, &QTabWidget::tabBarClicked, this, &ReplayViewer::onTabBarClicked);
    connect(this, &QTabWidget::currentChanged, this, &ReplayViewer::onCurrentTabChanged);
}

// ----------------------------------------------------------------------------
ReplayViewer::~ReplayViewer()
{
    if (protocol_)
        protocol_->dispatcher.removeListener(this);

    clearActiveSession();
    clearReplays();

    for (const auto& data : plugins_)
    {
        data.view->setParent(nullptr);
        data.plugin->uiInterface()->destroyView(data.view);
        pluginManager_->destroy(data.plugin);
    }
}

// ----------------------------------------------------------------------------
int ReplayViewer::findInCache(const QString& fileName) const
{
    for (int i = 0; i != replayCache_.size(); ++i)
        if (replayCache_[i].fileName == fileName)
            return i;
    return replayCache_.count();
}

// ----------------------------------------------------------------------------
void ReplayViewer::loadGameReplays(const QStringList& fileNames)
{
    clearReplays();
    pendingReplays_ = fileNames;

    QStringList loadedFileNames;
    QVector<rfcommon::Session*> loadedSessions;
    for (auto fileName : pendingReplays_)
    {
        int idx = findInCache(fileName);
        if (idx == replayCache_.size())
        {
            QByteArray ba = fileName.toUtf8();
            if (auto session = rfcommon::Session::load(ba.constData()))
            {
                loadedFileNames.push_back(fileName);
                loadedSessions.push_back(session);
            }
        }
    }

    // TODO background loading
    onGameReplaysLoaded(loadedFileNames, loadedSessions);
}

// ----------------------------------------------------------------------------
void ReplayViewer::onGameReplaysLoaded(const QStringList& fileNames, const QVector<rfcommon::Session*>& sessions)
{
    assert(fileNames.size() == sessions.size());

    // Go through cache and find the replays that were already loaded, and add
    // them to the active list of replays
    for (const auto& fileName : pendingReplays_)
    {
        int idx = findInCache(fileName);
        if (idx != replayCache_.size())
            activeReplays_.push_back(replayCache_[idx].session);
    }

    // Add the newly loaded replays to the active list of replays and also
    // to the cache
    for (int i = 0; i != fileNames.size(); ++i)
    {
        activeReplays_.push_back(sessions[i]);
        replayCache_.push_back({ fileNames[i], sessions[i] });
    }

    if (activeReplays_.size() == 1)
    {
        for (const auto& data : plugins_)
            if (auto i = data.plugin->replayInterface())
                i->onGameSessionLoaded(activeReplays_[0]);
    }
    else if (activeReplays_.size() > 1)
    {
        for (const auto& data : plugins_)
            if (auto i = data.plugin->replayInterface())
                i->onGameSessionSetLoaded(activeReplays_.data(), activeReplays_.size());
    }
    
    replayState_ = GAME_LOADED;
}

// ----------------------------------------------------------------------------
void ReplayViewer::clearReplays()
{
    if (activeReplays_.size() == 1)
    {
        for (const auto& data : plugins_)
            if (auto i = data.plugin->replayInterface())
                i->onGameSessionUnloaded(activeReplays_[0]);
    }
    else if (activeReplays_.size() > 1)
    {
        for (const auto& data : plugins_)
            if (auto i = data.plugin->replayInterface())
                i->onGameSessionSetUnloaded(activeReplays_.data(), activeReplays_.size());
    }

    activeReplays_.clear();

    // Subtle bug alert: Cache holds references to replays, but the "activeReplay"
    // vector does not. This means that when no plugins are loaded, the cache
    // is the only thing holding a reference to each loaded session. This is
    // why we remove old sessions from the cache *after* updating the
    // plugins, so they can grab references to each session before we drop
    // ours.
    while (replayCache_.size() > 20)
        replayCache_.pop_front();

    replayState_ = NONE_LOADED;
}

// ----------------------------------------------------------------------------
void ReplayViewer::clearActiveSession()
{
    if (activeSessionState_ != NO_ACTIVE_SESSION)
    {
        assert(activeSession_.notNull());
        assert(activeSession_->tryGetMetaData());

        switch (activeSession_->tryGetMetaData()->type())
        {
            case rfcommon::MetaData::GAME:
                for (const auto& data : plugins_)
                    if (auto i = data.plugin->realtimeInterface())
                        i->onProtocolGameEnded(activeSession_);
                break;

            case rfcommon::MetaData::TRAINING:
                for (const auto& data : plugins_)
                    if (auto i = data.plugin->realtimeInterface())
                        i->onProtocolTrainingEnded(activeSession_);
                break;
        }

        activeSession_.drop();
        activeSessionState_ = NO_ACTIVE_SESSION;
    }
}

// ----------------------------------------------------------------------------
void ReplayViewer::onTabBarClicked(int index)
{
    if (index < count() - 1 || index < 0)
        return;

    auto pluginLoaded = [this](const QString& name) -> bool {
        for (const auto& data : plugins_)
            if (data.name == name)
                return true;
        return false;
    };

    // Create a list of all available plugins that can be loaded. There are
    // two modes here. Either this class was created with a valid protocol
    // object, in which case we look for plugins that implement the realtime
    // API, or we were not given a protocol (nullptr), in which case we
    // assume we're in replay mode and look for plugins that implement the
    // replay API
    auto pluginNames = protocol_ ?
            pluginManager_->availableFactoryNames(RFPluginType::UI | RFPluginType::REALTIME) :
            pluginManager_->availableFactoryNames(RFPluginType::UI | RFPluginType::REPLAY);
    qSort(pluginNames.begin(), pluginNames.end());

    // Open popup menu with all of the plugins that aren't loaded yet
    QMenu popup;
    for (const auto& name : pluginNames)
    {
        if (pluginLoaded(name))
            continue;
        popup.addAction(name);
    }

    QAction* action = popup.exec(QCursor::pos());
    if (action == nullptr)
        return;

    // Create plugin
    PluginData data;
    data.name = action->text();
    data.plugin = pluginManager_->create(data.name);
    if (data.plugin == nullptr)
        return;

    data.view = nullptr;
    if (auto i = data.plugin->uiInterface())
        data.view = i->createView();
    if (data.view == nullptr)
    {
        pluginManager_->destroy(data.plugin);
        return;
    }
    plugins_.push_back(data);

    // Create tab and place view from plugin inside it
    QStyle* style = qApp->style();
    QIcon closeIcon = style->standardIcon(QStyle::SP_TitleBarCloseButton);
    QToolButton* closeButton = new QToolButton;
    closeButton->setStyleSheet("border: none;");
    closeButton->setIcon(closeIcon);
    insertTab(index, data.view, data.name);
    tabBar()->setTabButton(index, QTabBar::RightSide, closeButton);

    QWidget* view = data.view;
    connect(closeButton, &QToolButton::released, [this, view]() {
        closeTabWithView(view);
    });

    previousTab_ = index;

    // See if there is an active session we can give to the new plugin
    switch (activeSessionState_)
    {
    case TRAINING_STARTED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolTrainingStarted(activeSession_);
        break;

    case TRAINING_RESUMED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolTrainingResumed(activeSession_);
        break;

    case TRAINING_STARTED_ENDED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
        {
            i->onProtocolTrainingStarted(activeSession_);
            i->onProtocolTrainingEnded(activeSession_);
        }
        break;

    case TRAINING_RESUMED_ENDED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
        {
            i->onProtocolTrainingResumed(activeSession_);
            i->onProtocolTrainingEnded(activeSession_);
        }
        break;

    case GAME_STARTED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolGameStarted(activeSession_);
        break;

    case GAME_RESUMED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolGameResumed(activeSession_);
        break;

    case GAME_STARTED_ENDED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
        {
            i->onProtocolGameStarted(activeSession_);
            i->onProtocolGameEnded(activeSession_);
        }
        break;

    case GAME_RESUMED_ENDED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
        {
            i->onProtocolGameResumed(activeSession_);
            i->onProtocolGameEnded(activeSession_);
        }
        break;

    case NO_ACTIVE_SESSION:
        switch (replayState_)
        {
        case GAME_LOADED:
            assert(activeReplays_.size() > 0);
            if (activeReplays_.size() == 1)
            {
                if (auto i = data.plugin->replayInterface())
                    i->onGameSessionLoaded(activeReplays_[0]);
            }
            else
            {
                if (auto i = data.plugin->replayInterface())
                    i->onGameSessionSetLoaded(activeReplays_.data(), activeReplays_.size());
            }
            break;

        case TRAINING_LOADED:
            assert(activeReplays_.size() == 1);
            if (auto i = data.plugin->replayInterface())
                i->onTrainingSessionLoaded(activeReplays_[0]);
            break;

        case NONE_LOADED: break;
        }
    }
}

// ----------------------------------------------------------------------------
void ReplayViewer::onCurrentTabChanged(int index)
{
    if (index >= 0 && index < count() - 1)
        previousTab_ = index;
    else
        setCurrentIndex(previousTab_);
}

// ----------------------------------------------------------------------------
void ReplayViewer::closeTabWithView(QWidget* view)
{
    int currentTab = currentIndex();
    int tabCount = count();
    if (currentTab == tabCount - 2 && currentTab > 0)
    {
        previousTab_ = currentTab - 1;
    }

    for (auto it = plugins_.begin(); it != plugins_.end(); ++it)
    {
        if (it->view != view)
            continue;

        it->view->setParent(nullptr);
        it->plugin->uiInterface()->destroyView(it->view);
        pluginManager_->destroy(it->plugin);
        plugins_.erase(it);
        break;
    }

    setCurrentIndex(previousTab_);
}

// ----------------------------------------------------------------------------
void ReplayViewer::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) 
{
    sessionState_ = ATTEMPT_CONNECT;
    ipAddress_ = ipAddress;
    port_ = port;

    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolAttemptConnectToServer(ipAddress, port);
}
void ReplayViewer::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) 
{
    sessionState_ = DISCONNECTED;

    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolFailedToConnectToServer(errormsg, ipAddress, port);
}
void ReplayViewer::onProtocolConnectedToServer(const char* ipAddress, uint16_t port)
{
    sessionState_ = CONNECTED;

    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolConnectedToServer(ipAddress, port);
}
void ReplayViewer::onProtocolDisconnectedFromServer() 
{
    sessionState_ = DISCONNECTED;

    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolDisconnectedFromServer();
}

// ----------------------------------------------------------------------------
void ReplayViewer::onProtocolTrainingStarted(rfcommon::Session* training)
{
    activeSessionState_ = TRAINING_STARTED;
    activeSession_ = training;

    // If there are replays being displayed currently, override it with
    // the active session. Seems to make sense from a user perspective
    clearReplays();
    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolTrainingStarted(training);
}
void ReplayViewer::onProtocolTrainingResumed(rfcommon::Session* training)
{
    activeSessionState_ = TRAINING_RESUMED;
    activeSession_ = training;

    // If there are replays being displayed currently, override it with
    // the active session. Seems to make sense from a user perspective
    clearReplays();
    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolTrainingResumed(training);
}
void ReplayViewer::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    assert(activeSession_ == oldTraining);
    activeSession_ = newTraining;

    // If there are replays being displayed currently, then those have
    // precedence. Store the active session for when the user deselects
    // replays in the UI, or until a session start event is received
    if (replayState_ == NONE_LOADED)
    {
        for (const auto& data : plugins_)
            if (auto i = data.plugin->realtimeInterface())
                i->onProtocolTrainingReset(oldTraining, newTraining);
    }
}
void ReplayViewer::onProtocolTrainingEnded(rfcommon::Session* training)
{
    assert(activeSession_ == training);
    activeSessionState_ = activeSessionState_ == TRAINING_STARTED ? TRAINING_STARTED_ENDED : TRAINING_RESUMED_ENDED;

    // If there are replays being displayed currently, then those
    // have precendence and we don't need to propagate the training
    // ended event
    if (replayState_ == NONE_LOADED)
    {
        for (const auto& data : plugins_)
            if (auto i = data.plugin->realtimeInterface())
                i->onProtocolTrainingEnded(training);
    }
}
void ReplayViewer::onProtocolGameStarted(rfcommon::Session* game)
{
    activeSessionState_ = GAME_STARTED;
    activeSession_ = game;

    // If there are replays being displayed currently, override it with
    // the active session. Seems to make sense from a user perspective
    clearReplays();
    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolGameStarted(game);
}
void ReplayViewer::onProtocolGameResumed(rfcommon::Session* game)
{
    activeSessionState_ = GAME_RESUMED;
    activeSession_ = game;

    // If there are replays being displayed currently, override it with
    // the active session. Seems to make sense from a user perspective
    clearReplays();
    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolGameResumed(game);
}
void ReplayViewer::onProtocolGameEnded(rfcommon::Session* game)
{
    assert(activeSession_ == game);
    activeSessionState_ = activeSessionState_ == GAME_STARTED ? GAME_STARTED_ENDED : GAME_RESUMED_ENDED;

    // If there are replays being displayed currently, then those
    // have precendence and we don't need to propagate the training
    // ended event
    if (replayState_ == NONE_LOADED)
    {
        for (const auto& data : plugins_)
            if (auto i = data.plugin->realtimeInterface())
                i->onProtocolGameEnded(game);
    }
}

}
