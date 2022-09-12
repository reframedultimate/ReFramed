#include "application/views/ReplayViewer.hpp"
#include "application/models/PluginManager.hpp"
#include "application/models/Protocol.hpp"
#include "application/models/ReplayManager.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/VisualizerContext.hpp"
#include <QTabBar>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QStyle>
#include <QApplication>

namespace rfapp {

// ----------------------------------------------------------------------------
ReplayViewer::ReplayViewer(ReplayManager* replayManager, PluginManager* pluginManager, QWidget* parent)
    : QTabWidget(parent)
    , protocol_(nullptr)
    , replayManager_(replayManager)
    , pluginManager_(pluginManager)
    , visCtx_(new rfcommon::VisualizerContext)
    , sessionState_(DISCONNECTED)
    , activeSessionState_(NO_ACTIVE_SESSION)
    , replayState_(NONE_LOADED)
    , previousTab_(0)
{
    addTab(new QWidget, "+");

    replayManager_->dispatcher.addListener(this);
    replayManager_->allReplayGroup()->dispatcher.addListener(this);
    for (int i = 0; i != replayManager_->replayGroupCount(); ++i)
        replayManager_->replayGroup(i)->dispatcher.addListener(this);

    connect(this, &QTabWidget::tabBarClicked, this, &ReplayViewer::onTabBarClicked);
    connect(this, &QTabWidget::currentChanged, this, &ReplayViewer::onCurrentTabChanged);
}

// ----------------------------------------------------------------------------
ReplayViewer::ReplayViewer(Protocol* protocol, PluginManager* pluginManager, QWidget* parent)
    : QTabWidget(parent)
    , protocol_(protocol)
    , replayManager_(nullptr)
    , pluginManager_(pluginManager)
    , visCtx_(new rfcommon::VisualizerContext)
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

    if (replayManager_)
    {
        for (int i = 0; i != replayManager_->replayGroupCount(); ++i)
            replayManager_->replayGroup(i)->dispatcher.removeListener(this);
        replayManager_->allReplayGroup()->dispatcher.removeListener(this);

        replayManager_->dispatcher.removeListener(this);
    }

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
    PROFILE(ReplayViewer, findInCache);

    for (int i = 0; i != replayCache_.size(); ++i)
        if (replayCache_[i].fileName == fileName)
            return i;
    return replayCache_.count();
}

// ----------------------------------------------------------------------------
void ReplayViewer::loadGameReplays(const QStringList& fileNames)
{
    PROFILE(ReplayViewer, loadGameReplays);

    clearReplays();
    pendingReplays_ = fileNames;

    QStringList loadedFileNames;
    QVector<rfcommon::Session*> loadedSessions;
    for (auto fileName : pendingReplays_)
    {
        assert(QDir(fileName).isAbsolute());
        int idx = findInCache(fileName);
        if (idx == replayCache_.size())
        {
            QByteArray ba = fileName.toLocal8Bit();
            if (auto session = rfcommon::Session::load(replayManager_, ba.constData()))
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
void ReplayViewer::reloadReplays()
{
    QStringList files = pendingReplays_;
    clearReplays();
    replayCache_.clear();
    loadGameReplays(files);
}

// ----------------------------------------------------------------------------
void ReplayViewer::onGameReplaysLoaded(const QStringList& fileNames, const QVector<rfcommon::Session*>& sessions)
{
    PROFILE(ReplayViewer, onGameReplaysLoaded);

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
    PROFILE(ReplayViewer, clearReplays);

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
    pendingReplays_.clear();

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
    PROFILE(ReplayViewer, clearActiveSession);

    if (activeSessionState_ != NO_ACTIVE_SESSION
        && activeSessionState_ != TRAINING_STARTED_ENDED
        && activeSessionState_ != TRAINING_RESUMED_ENDED
        && activeSessionState_ != GAME_STARTED_ENDED
        && activeSessionState_ != GAME_RESUMED_ENDED)
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
    PROFILE(ReplayViewer, onTabBarClicked);

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
    data.plugin = pluginManager_->create(data.name, visCtx_);
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

    switch (sessionState_)
    {
    case ATTEMPT_CONNECT:
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
        break;
    case CONNECTED:
        if (auto i = data.plugin->realtimeInterface())
        {
            i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
            i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
        }
        break;
    case DISCONNECTED:
        break;
    }

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
            switch (sessionState_)
            {
            case DISCONNECTED:
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolTrainingStarted(activeSession_);
                i->onProtocolTrainingEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                break;
            case ATTEMPT_CONNECT:
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolTrainingStarted(activeSession_);
                i->onProtocolTrainingEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                break;
            case CONNECTED:
                i->onProtocolTrainingStarted(activeSession_);
                i->onProtocolTrainingEnded(activeSession_);
                break;
            }
        }
        break;

    case TRAINING_RESUMED_ENDED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
        {
            switch (sessionState_)
            {
            case DISCONNECTED:
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolTrainingResumed(activeSession_);
                i->onProtocolTrainingEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                break;
            case ATTEMPT_CONNECT:
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolTrainingResumed(activeSession_);
                i->onProtocolTrainingEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                break;
            case CONNECTED:
                i->onProtocolTrainingResumed(activeSession_);
                i->onProtocolTrainingEnded(activeSession_);
                break;
            }
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
            switch (sessionState_)
            {
            case DISCONNECTED:
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolGameStarted(activeSession_);
                i->onProtocolGameEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                break;
            case ATTEMPT_CONNECT:
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolGameStarted(activeSession_);
                i->onProtocolGameEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                break;
            case CONNECTED:
                i->onProtocolGameStarted(activeSession_);
                i->onProtocolGameEnded(activeSession_);
                break;
            }
        }
        break;

    case GAME_RESUMED_ENDED:
        assert(activeSession_.notNull());
        if (auto i = data.plugin->realtimeInterface())
        {
            switch (sessionState_)
            {
            case DISCONNECTED:
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolGameResumed(activeSession_);
                i->onProtocolGameEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                break;
            case ATTEMPT_CONNECT:
                i->onProtocolConnectedToServer(ipAddress_.toUtf8().constData(), port_);
                i->onProtocolGameResumed(activeSession_);
                i->onProtocolGameEnded(activeSession_);
                i->onProtocolDisconnectedFromServer();
                i->onProtocolAttemptConnectToServer(ipAddress_.toUtf8().constData(), port_);
                break;
            case CONNECTED:
                i->onProtocolGameResumed(activeSession_);
                i->onProtocolGameEnded(activeSession_);
                break;
            }
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
    PROFILE(ReplayViewer, onCurrentTabChanged);

    if (index >= 0 && index < count() - 1)
        previousTab_ = index;
    else
        setCurrentIndex(previousTab_);
}

// ----------------------------------------------------------------------------
void ReplayViewer::closeTabWithView(QWidget* view)
{
    PROFILE(ReplayViewer, closeTabWithView);

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

        switch (activeSessionState_)
        {
            case TRAINING_STARTED:
            case TRAINING_RESUMED:
                assert(activeSession_.notNull());
                assert(activeSession_->tryGetMetaData());
                assert(activeSession_->tryGetMetaData()->type() == rfcommon::MetaData::TRAINING);
                if (auto i = it->plugin->realtimeInterface())
                    i->onProtocolTrainingEnded(activeSession_);
                break;

            case GAME_STARTED:
            case GAME_RESUMED:
                assert(activeSession_.notNull());
                assert(activeSession_->tryGetMetaData());
                assert(activeSession_->tryGetMetaData()->type() == rfcommon::MetaData::GAME);
                if (auto i = it->plugin->realtimeInterface())
                    i->onProtocolGameEnded(activeSession_);
                break;

            case NO_ACTIVE_SESSION:
            case TRAINING_STARTED_ENDED:
            case TRAINING_RESUMED_ENDED:
            case GAME_STARTED_ENDED:
            case GAME_RESUMED_ENDED:
                break;
        }

        if (activeReplays_.size() == 1)
        {
            if (auto i = it->plugin->replayInterface())
                i->onGameSessionUnloaded(activeReplays_[0]);
        }
        else if (activeReplays_.size() > 1)
        {
            if (auto i = it->plugin->replayInterface())
                i->onGameSessionSetUnloaded(activeReplays_.data(), activeReplays_.size());
        }

        switch (sessionState_)
        {
        case ATTEMPT_CONNECT:
            if (auto i = it->plugin->realtimeInterface())
                i->onProtocolFailedToConnectToServer("Plugin is being destroyed", ipAddress_.toUtf8().constData(), port_);
            break;
        case CONNECTED:
            if (auto i = it->plugin->realtimeInterface())
                i->onProtocolDisconnectedFromServer();
        case DISCONNECTED:
            break;
        }

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
    PROFILE(ReplayViewer, onProtocolAttemptConnectToServer);

    sessionState_ = ATTEMPT_CONNECT;
    ipAddress_ = ipAddress;
    port_ = port;

    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolAttemptConnectToServer(ipAddress, port);
}
void ReplayViewer::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port)
{
    PROFILE(ReplayViewer, onProtocolFailedToConnectToServer);

    sessionState_ = DISCONNECTED;

    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolFailedToConnectToServer(errormsg, ipAddress, port);
}
void ReplayViewer::onProtocolConnectedToServer(const char* ipAddress, uint16_t port)
{
    PROFILE(ReplayViewer, onProtocolConnectedToServer);

    sessionState_ = CONNECTED;

    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolConnectedToServer(ipAddress, port);
}
void ReplayViewer::onProtocolDisconnectedFromServer()
{
    PROFILE(ReplayViewer, onProtocolDisconnectedFromServer);

    sessionState_ = DISCONNECTED;

    for (const auto& data : plugins_)
        if (auto i = data.plugin->realtimeInterface())
            i->onProtocolDisconnectedFromServer();
}

// ----------------------------------------------------------------------------
void ReplayViewer::onProtocolTrainingStarted(rfcommon::Session* training)
{
    PROFILE(ReplayViewer, onProtocolTrainingStarted);

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
    PROFILE(ReplayViewer, onProtocolTrainingResumed);

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
    PROFILE(ReplayViewer, onProtocolTrainingReset);

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
    PROFILE(ReplayViewer, onProtocolTrainingEnded);

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
    PROFILE(ReplayViewer, onProtocolGameStarted);

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
    PROFILE(ReplayViewer, onProtocolGameResumed);

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
    PROFILE(ReplayViewer, onProtocolGameEnded);

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

// ----------------------------------------------------------------------------
void ReplayViewer::onReplayManagerDefaultGamePathChanged(const QDir& path) {}
void ReplayViewer::onReplayManagerGroupAdded(ReplayGroup* group)
{
    group->dispatcher.addListener(this);
}
void ReplayViewer::onReplayManagerGroupNameChanged(ReplayGroup* group, const QString& oldName, const QString& newName) {}
void ReplayViewer::onReplayManagerGroupRemoved(ReplayGroup* group)
{
    group->dispatcher.removeListener(this);
}
void ReplayViewer::onReplayManagerGamePathAdded(const QDir& path) {}
void ReplayViewer::onReplayManagerGamePathRemoved(const QDir& path) {}
void ReplayViewer::onReplayManagerVideoPathAdded(const QDir& path) {}
void ReplayViewer::onReplayManagerVideoPathRemoved(const QDir& path) {}

// ----------------------------------------------------------------------------
void ReplayViewer::onReplayGroupFileAdded(ReplayGroup* group, const QString& fileName)
{
}
void ReplayViewer::onReplayGroupFileRemoved(ReplayGroup* group, const QString& fileName)
{
    for (auto it = replayCache_.begin(); it != replayCache_.end(); ++it)
        if (fileName == QFileInfo(it->fileName).fileName())
        {
            replayCache_.erase(it);
            break;
        }
}

}
