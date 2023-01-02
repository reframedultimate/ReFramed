#include "application/models/ActiveSessionManager.hpp"
#include "application/models/MetadataEditModel.hpp"
#include "application/views/ActiveSessionView.hpp"
#include "application/views/PluginDockView.hpp"
#include "application/widgets/CollapsibleSplitter.hpp"
#include "application/widgets/MetadataEditWidget_AutoAssociateVideo.hpp"
#include "application/widgets/MetadataEditWidget_Commentators.hpp"
#include "application/widgets/MetadataEditWidget_Event.hpp"
#include "application/widgets/MetadataEditWidget_Game.hpp"
#include "application/widgets/MetadataEditWidget_Tournament.hpp"

#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"

#include <QVBoxLayout>
#include <QDateTime>
#include <QSplitter>
#include <QScrollArea>
#include <QSpacerItem>

namespace rfapp {

using nlohmann::json;

// ----------------------------------------------------------------------------
namespace {

class ScrollAreaWithSizeHint : public QScrollArea
{
public:
    QSize sizeHint() const override { return QSize(900, 100); }
};

}

// ----------------------------------------------------------------------------
ActiveSessionView::ActiveSessionView(
        Config* config,
        ActiveSessionManager* activeSessionManager,
        PluginManager* pluginManager,
        PlayerDetails* playerDetails,
        QWidget* parent)
    : QWidget(parent)
    , ConfigAccessor(config)
    , activeSessionManager_(activeSessionManager)
    , metadataEditModel_(new MetadataEditModel)
{
    metadataEditWidgets_.push_back(new MetadataEditWidget_Tournament(metadataEditModel_.get()));
    metadataEditWidgets_.push_back(new MetadataEditWidget_Commentators(metadataEditModel_.get()));
    metadataEditWidgets_.push_back(new MetadataEditWidget_Event(metadataEditModel_.get()));
    metadataEditWidgets_.push_back(new MetadataEditWidget_Game(metadataEditModel_.get(), playerDetails));
    metadataEditWidgets_.push_back(new MetadataEditWidget_AutoAssociateVideo(metadataEditModel_.get(), activeSessionManager_));

    json& cfg = configRoot();
    json& jActiveSessionView = cfg["activesessionview"];
    json& jExpanded = jActiveSessionView["expanded"];
    if (jExpanded.is_array())
        for (int i = 0; i != jExpanded.size() && i < metadataEditWidgets_.size(); ++i)
            if (jExpanded[i].is_boolean() && jExpanded[i].get<bool>())
                metadataEditWidgets_[i]->setExpanded(true);

    QVBoxLayout* metadataEditLayout = new QVBoxLayout;
    for (MetadataEditWidget* w : metadataEditWidgets_)
        metadataEditLayout->addWidget(w);
    metadataEditLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    QWidget* metadataEditContents = new QWidget;
    metadataEditContents->setLayout(metadataEditLayout);

    scrollArea_ = new ScrollAreaWithSizeHint;
    scrollArea_->setWidgetResizable(true);
    scrollArea_->setWidget(metadataEditContents);

    splitter_ = new CollapsibleSplitter(Qt::Horizontal);
    splitter_->addWidget(scrollArea_);
    splitter_->addWidget(new PluginDockView(activeSessionManager_->protocol(), pluginManager));
    splitter_->setStretchFactor(0, 0);
    splitter_->setStretchFactor(1, 1);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(splitter_);
    setLayout(l);

    activeSessionManager_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ActiveSessionView::~ActiveSessionView()
{
    json& cfg = configRoot();
    json& jActiveSessionView = cfg["activesessionview"];
    json jExpanded = json::array();
    for (MetadataEditWidget* w : metadataEditWidgets_)
        jExpanded.push_back(w->isExpanded());
    jActiveSessionView["expanded"] = jExpanded;

    // Scroll area contains widgets that are registered as listeners to
    // metadataEditModel_. Have to delete them explicitly, otherwise the model
    // is deleted before the widgets are deleted.
    delete scrollArea_;

    activeSessionManager_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void ActiveSessionView::toggleSideBar()
{
    PROFILE(ActiveSessionView, toggleSideBar);

    splitter_->toggleCollapse();
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerGameStarted(rfcommon::Session* game)
{
    PROFILE(ActiveSessionView, onActiveSessionManagerGameStarted);

    auto* map = game->tryGetMappingInfo();
    auto* mdata = game->tryGetMetadata();
    assert(map);
    assert(mdata);

    metadataEditModel_->setAndOverwrite({map}, {mdata});
    metadataEditModel_->startNextGame();
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerGameEnded(rfcommon::Session* game)
{
    PROFILE(ActiveSessionView, onActiveSessionManagerGameEnded);

    metadataEditModel_->clear();
}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerTrainingStarted(rfcommon::Session* training)
{
    PROFILE(ActiveSessionView, onActiveSessionManagerTrainingStarted);

}

// ----------------------------------------------------------------------------
void ActiveSessionView::onActiveSessionManagerTrainingEnded(rfcommon::Session* training)
{
    PROFILE(ActiveSessionView, onActiveSessionManagerTrainingEnded);

}

}
