#include "application/ui_MetaDataEditWidget_Game.h"
#include "application/widgets/MetaDataEditWidget_Game.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Game::MetaDataEditWidget_Game(QWidget* parent)
    : MetaDataEditWidget(parent)
    , ui_(new Ui::MetaDataEditWidget_Game)
{
    ui_->setupUi(contentWidget());
    updateSize();

    setTitle("Game");
    setExpanded(true);
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_Game::~MetaDataEditWidget_Game()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Game::onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) {}
void MetaDataEditWidget_Game::onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_Game::onMetaDataPlayerNameChanged(int fighterIdx, const char* name) {}
void MetaDataEditWidget_Game::onMetaDataSponsorChanged(int fighterIdx, const char* sponsor) {}
void MetaDataEditWidget_Game::onMetaDataTournamentNameChanged(const char* name) {}
void MetaDataEditWidget_Game::onMetaDataEventNameChanged(const char* name) {}
void MetaDataEditWidget_Game::onMetaDataRoundNameChanged(const char* name) {}
void MetaDataEditWidget_Game::onMetaDataCommentatorsChanged(const rfcommon::SmallVector<rfcommon::String, 2>& names) {}
void MetaDataEditWidget_Game::onMetaDataSetNumberChanged(rfcommon::SetNumber number) {}
void MetaDataEditWidget_Game::onMetaDataGameNumberChanged(rfcommon::GameNumber number) {}
void MetaDataEditWidget_Game::onMetaDataSetFormatChanged(const rfcommon::SetFormat& format) {}
void MetaDataEditWidget_Game::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Game::onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number) {}

}
