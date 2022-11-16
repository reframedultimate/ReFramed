#include "application/widgets/MetaDataEditWidget_Commentators.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Commentators::MetaDataEditWidget_Commentators(MetaDataEditModel* model, QWidget* parent)
    : MetaDataEditWidget(model, parent)
    , commentatorsLayout_(new QVBoxLayout)
{
    setTitle("Commentators");

    QToolButton* addCommentator = new QToolButton;
    addCommentator->setText("+");
    commentatorsLayout_->addWidget(addCommentator);

    contentWidget()->setLayout(commentatorsLayout_);
    updateSize();

    connect(addCommentator, &QToolButton::released, this, &MetaDataEditWidget_Commentators::onAddCommentatorReleased);
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_Commentators::~MetaDataEditWidget_Commentators()
{}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Commentators::onAddCommentatorReleased()
{
    QToolButton* removeButton = new QToolButton;
    removeButton->setText("-");

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Name:", new QLineEdit);
    layout->addRow("Social:", new QLineEdit);
    layout->addRow("Pronouns:", new QLineEdit("he/him"));
    layout->addRow(removeButton);

    QGroupBox* g = new QGroupBox;
    g->setTitle("Commentator #" + QString::number(commentatorsLayout_->count()));
    g->setLayout(layout);

    QLayoutItem* addButtonItem = commentatorsLayout_->takeAt(commentatorsLayout_->count() - 1);
    commentatorsLayout_->addWidget(g);
    commentatorsLayout_->addItem(addButtonItem);

    updateSize();

    connect(removeButton, &QToolButton::released, [this, g] {
        for (int i = 0; i != commentatorsLayout_->count(); ++i)
        {
            QLayoutItem* item = commentatorsLayout_->itemAt(i);
            if (item->widget() == g)
            {
                item = commentatorsLayout_->takeAt(i);
                delete item->widget();
                delete item;
                updateSize();
                return;
            }
        }
    });
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Commentators::onAdoptMetaData(rfcommon::MetaData* mdata)
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Commentators::onOverwriteMetaData(rfcommon::MetaData* mdata)
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Commentators::onMetaDataCleared(rfcommon::MetaData* mdata)
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Commentators::onBracketTypeChangedUI(rfcommon::BracketType bracketType)
{

}

void MetaDataEditWidget_Commentators::onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_Commentators::onMetaDataTournamentDetailsChanged() {}
void MetaDataEditWidget_Commentators::onMetaDataEventDetailsChanged() {}
void MetaDataEditWidget_Commentators::onMetaDataCommentatorsChanged() {}
void MetaDataEditWidget_Commentators::onMetaDataGameDetailsChanged() {}
void MetaDataEditWidget_Commentators::onMetaDataPlayerDetailsChanged() {}
void MetaDataEditWidget_Commentators::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Commentators::onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
