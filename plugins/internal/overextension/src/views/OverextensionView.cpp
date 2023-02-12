#include "ui_OverextensionView.h"
#include "overextension/views/OverextensionView.hpp"
#include "overextension/models/OverextensionModel.hpp"

#include "rfcommon/UserMotionLabels.hpp"

#include <QVBoxLayout>
#include <QPlainTextEdit>

// ----------------------------------------------------------------------------
OverextensionView::OverextensionView(OverextensionModel* model, rfcommon::UserMotionLabels* userLabels)
    : ui_(new Ui::OverextensionView)
    , model_(model)
    , userLabels_(userLabels)
    , text_(new QPlainTextEdit)
{
    ui_->setupUi(this);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(text_);
    ui_->tab->setLayout(l);

    connect(ui_->comboBox_player, qOverload<int>(&QComboBox::currentIndexChanged), this, &OverextensionView::onPOVChanged);
    connect(ui_->spinBox_escapeOption, qOverload<int>(&QSpinBox::valueChanged), this, &OverextensionView::onEarliestEscapeOptionChanged);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
OverextensionView::~OverextensionView()
{
    delete ui_;

    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void OverextensionView::onPOVChanged(int index)
{
    if (index >= 0)
    {
        model_->setCurrentFighter(index);
        lastFighterPOV_ = QString::fromUtf8(model_->fighterName(index).cStr());
    }
}

// ----------------------------------------------------------------------------
void OverextensionView::onEarliestEscapeOptionChanged(int value)
{
    if (model_->fighterCount() > 0)
        model_->setOpponentEarliestEscape(model_->currentFighter(), value);
}

// ----------------------------------------------------------------------------
void OverextensionView::onPlayersChanged()
{
    QSignalBlocker blockPOVComboBox(ui_->comboBox_player);
    QSignalBlocker blockEscapeFrames(ui_->spinBox_escapeOption);

    ui_->comboBox_player->clear();

    for (int i = 0; i != model_->fighterCount(); ++i)
    {
        QString name = QString::fromUtf8(model_->playerName(i).cStr());
        QString fighter = QString::fromUtf8(model_->fighterName(i).cStr());
        ui_->comboBox_player->addItem(name + " (" + fighter + ")");
        if (lastFighterPOV_ == fighter)
        {
            model_->setCurrentFighter(i);
            ui_->spinBox_escapeOption->setValue(model_->opponentEarliestEscape(i));
        }
    }
}

// ----------------------------------------------------------------------------
void OverextensionView::onDataChanged()
{
    text_->clear();

    int fighterIdx = ui_->comboBox_player->currentIndex();
    if (fighterIdx < 0)
        return;

    text_->insertPlainText("true combos: " + QString::number(model_->moveCount(fighterIdx, OverextensionModel::TRUE_COMBO)) + "\n");
    text_->insertPlainText("combos: " + QString::number(model_->moveCount(fighterIdx, OverextensionModel::COMBO)) + "\n");
    text_->insertPlainText("winning: " + QString::number(model_->moveCount(fighterIdx, OverextensionModel::WINNING_OVEREXTENSION)) + "\n");
    text_->insertPlainText("trading: " + QString::number(model_->moveCount(fighterIdx, OverextensionModel::LOSING_OVEREXTENSION)) + "\n");
    text_->insertPlainText("losing: " + QString::number(model_->moveCount(fighterIdx, OverextensionModel::TRADING_OVEREXTENSION)) + "\n");

    auto fighterID = model_->fighterID(fighterIdx);
    for (auto category : {
         OverextensionModel::COMBO,
         OverextensionModel::WINNING_OVEREXTENSION,
         OverextensionModel::LOSING_OVEREXTENSION,
         OverextensionModel::TRADING_OVEREXTENSION
    }){
        text_->insertPlainText("\n" + QString::fromUtf8(OverextensionModel::categoryName(category)) + "\n");
        for (int i = 0; i != model_->moveCount(fighterIdx, category); ++i)
        {
            auto beforeMove = model_->moveBefore(fighterIdx, i, category);
            auto afterMove = model_->moveAfter(fighterIdx, i, category);
            auto beforeName = QString::fromUtf8(userLabels_->toStringHighestLayer(fighterID, beforeMove));
            auto afterName = QString::fromUtf8(userLabels_->toStringHighestLayer(fighterID, afterMove));
            auto frameStart = QString::number(model_->moveStart(fighterIdx, i, category).index());
            auto frameEnd = QString::number(model_->moveEnd(fighterIdx, i, category).index());
            auto gap = QString::number(model_->moveGap(fighterIdx, i, category));
            text_->insertPlainText("  " + beforeName + " -> " + afterName + " (" + gap + ") (" + frameStart + "-" + frameEnd + ")\n");
        }
    }
}

// ----------------------------------------------------------------------------
void OverextensionView::onCurrentFighterChanged(int fighterIdx)
{
    QSignalBlocker blockPOVComboBox(ui_->comboBox_player);
    QSignalBlocker blockEscapeFrames(ui_->spinBox_escapeOption);

    ui_->comboBox_player->setCurrentIndex(fighterIdx);
    ui_->spinBox_escapeOption->setValue(model_->opponentEarliestEscape(fighterIdx));
}
