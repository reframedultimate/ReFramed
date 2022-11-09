#include "application/widgets/MetaDataEditWidget_Players.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Players::MetaDataEditWidget_Players(QWidget* parent)
    : MetaDataEditWidget(parent)
    , TOLayout_(new QVBoxLayout)
    , sponsorsLayout_(new QVBoxLayout)
{
    setTitle("Players");

    QToolButton* addTO = new QToolButton;
    addTO->setText("+");
    TOLayout_->addWidget(addTO);

    QToolButton* addSponsor = new QToolButton;
    addSponsor->setText("+");
    sponsorsLayout_->addWidget(addSponsor);

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Tournament Name:", new QLineEdit);
    layout->addRow("Website:", new QLineEdit);
    layout->addRow("Organizers:", TOLayout_);
    layout->addRow("Sponsors:", sponsorsLayout_);

    contentWidget()->setLayout(layout);
    updateSize();
    setExpanded(true);

    connect(addTO, &QToolButton::released, this, &MetaDataEditWidget_Players::onAddTOReleased);
    connect(addSponsor, &QToolButton::released, this, &MetaDataEditWidget_Players::onAddSponsorReleased);
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_Players::~MetaDataEditWidget_Players()
{}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Players::onAddTOReleased()
{
    QToolButton* removeButton = new QToolButton;
    removeButton->setText("-");

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Name", new QLineEdit);
    layout->addRow("Social", new QLineEdit);
    layout->addRow(removeButton);

    QGroupBox* g = new QGroupBox;
    g->setTitle("TO #" + QString::number(TOLayout_->count()));
    g->setLayout(layout);

    QLayoutItem* addButtonItem = TOLayout_->takeAt(TOLayout_->count() - 1);
    TOLayout_->addWidget(g);
    TOLayout_->addItem(addButtonItem);

    updateSize();

    connect(removeButton, &QToolButton::released, [this, g] {
        for (int i = 0; i != TOLayout_->count(); ++i)
        {
            QLayoutItem* item = TOLayout_->itemAt(i);
            if (item->widget() == g)
            {
                item = TOLayout_->takeAt(i);
                delete item->widget();
                delete item;
                updateSize();
                return;
            }
        }
    });
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Players::onAddSponsorReleased()
{
    QToolButton* removeButton = new QToolButton;
    removeButton->setText("-");

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Name", new QLineEdit);
    layout->addRow("Website", new QLineEdit);
    layout->addRow(removeButton);

    QGroupBox* g = new QGroupBox;
    g->setTitle("Sponsor #" + QString::number(sponsorsLayout_->count()));
    g->setLayout(layout);

    QLayoutItem* addButtonItem = sponsorsLayout_->takeAt(sponsorsLayout_->count() - 1);
    sponsorsLayout_->addWidget(g);
    sponsorsLayout_->addItem(addButtonItem);

    updateSize();

    connect(removeButton, &QToolButton::released, [this, g] {
        for (int i = 0; i != TOLayout_->count(); ++i)
        {
            QLayoutItem* item = sponsorsLayout_->itemAt(i);
            if (item->widget() == g)
            {
                item = sponsorsLayout_->takeAt(i);
                delete item->widget();
                delete item;
                updateSize();
                return;
            }
        }
    });
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Players::onMetaDataTimeStartedChanged(rfcommon::TimeStamp timeStarted) {}
void MetaDataEditWidget_Players::onMetaDataTimeEndedChanged(rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_Players::onMetaDataPlayerNameChanged(int fighterIdx, const char* name) {}
void MetaDataEditWidget_Players::onMetaDataSponsorChanged(int fighterIdx, const char* sponsor) {}
void MetaDataEditWidget_Players::onMetaDataTournamentNameChanged(const char* name) {}
void MetaDataEditWidget_Players::onMetaDataEventNameChanged(const char* name) {}
void MetaDataEditWidget_Players::onMetaDataRoundNameChanged(const char* name) {}
void MetaDataEditWidget_Players::onMetaDataCommentatorsChanged(const rfcommon::SmallVector<rfcommon::String, 2>& names) {}
void MetaDataEditWidget_Players::onMetaDataSetNumberChanged(rfcommon::SetNumber number) {}
void MetaDataEditWidget_Players::onMetaDataGameNumberChanged(rfcommon::GameNumber number) {}
void MetaDataEditWidget_Players::onMetaDataSetFormatChanged(const rfcommon::SetFormat& format) {}
void MetaDataEditWidget_Players::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Players::onMetaDataTrainingSessionNumberChanged(rfcommon::GameNumber number) {}

}
