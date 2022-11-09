#include "application/widgets/MetaDataEditWidget_Event.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>
#include <QComboBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Event::MetaDataEditWidget_Event(QWidget* parent)
    : MetaDataEditWidget(parent)
{
    setTitle("Event");

    QComboBox* bracketType = new QComboBox;
    bracketType->addItem("Singles Bracket");
    bracketType->addItem("Doubles Bracket");
    bracketType->addItem("Side Bracket");
    bracketType->addItem("Amateurs Bracket");
    bracketType->addItem("Friendlies");
    bracketType->addItem("Money Match");

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Bracket Type:", bracketType);
    layout->addWidget(new QLineEdit);
    layout->addRow("Bracket URL:", new QLineEdit);

    contentWidget()->setLayout(layout);
    updateSize();
    setExpanded(true);
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_Event::~MetaDataEditWidget_Event()
{}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Event::onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) {}
void MetaDataEditWidget_Event::onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_Event::onMetaDataPlayerNameChanged(int fighterIdx, const char* name) {}
void MetaDataEditWidget_Event::onMetaDataSponsorChanged(int fighterIdx, const char* sponsor) {}
void MetaDataEditWidget_Event::onMetaDataTournamentNameChanged(const char* name) {}
void MetaDataEditWidget_Event::onMetaDataEventNameChanged(const char* name) {}
void MetaDataEditWidget_Event::onMetaDataRoundNameChanged(const char* name) {}
void MetaDataEditWidget_Event::onMetaDataCommentatorsChanged(const rfcommon::SmallVector<rfcommon::String, 2>& names) {}
void MetaDataEditWidget_Event::onMetaDataSetNumberChanged(rfcommon::SetNumber number) {}
void MetaDataEditWidget_Event::onMetaDataGameNumberChanged(rfcommon::GameNumber number) {}
void MetaDataEditWidget_Event::onMetaDataSetFormatChanged(const rfcommon::SetFormat& format) {}
void MetaDataEditWidget_Event::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Event::onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number) {}

}
