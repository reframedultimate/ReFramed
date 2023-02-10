#include "ui_OverextensionView.h"
#include "overextension/views/OverextensionView.hpp"
#include "overextension/models/OverextensionModel.hpp"

#include <QVBoxLayout>
#include <QPlainTextEdit>

// ----------------------------------------------------------------------------
OverextensionView::OverextensionView(OverextensionModel* model)
    : ui_(new Ui::OverextensionView)
    , model_(model)
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
    onDataChanged();
    
    if (index >= 0)
    {
        lastFighterPOV_ = QString::fromUtf8(model_->fighterName(index).cStr());

        QSignalBlocker block(ui_->spinBox_escapeOption);
        ui_->spinBox_escapeOption->setValue(model_->opponentEarliestEscape(index));
    }
}

// ----------------------------------------------------------------------------
void OverextensionView::onEarliestEscapeOptionChanged(int value)
{
    int fighterIdx = ui_->comboBox_player->currentIndex();
    if (fighterIdx < 0)
        return;

    model_->setOpponentEarliestEscape(fighterIdx, value);
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
            ui_->comboBox_player->setCurrentIndex(i);
    }

    int fighterIdx = ui_->comboBox_player->currentIndex();
    if (fighterIdx >= 0)
        ui_->spinBox_escapeOption->setValue(model_->opponentEarliestEscape(fighterIdx));
}

// ----------------------------------------------------------------------------
void OverextensionView::onDataChanged()
{
    text_->clear();

    int fighterIdx = ui_->comboBox_player->currentIndex();
    if (fighterIdx < 0)
        return;

    text_->insertPlainText("total: " + QString::number(model_->numTotal(fighterIdx)) + "\n");
    text_->insertPlainText("true combos: " + QString::number(model_->numTrueCombos(fighterIdx)) + "\n");
    text_->insertPlainText("combos: " + QString::number(model_->numCombos(fighterIdx)) + "\n");
    text_->insertPlainText("winning: " + QString::number(model_->numWinningOverextensions(fighterIdx)) + "\n");
    text_->insertPlainText("trading: " + QString::number(model_->numTradingOverextensions(fighterIdx)) + "\n");
    text_->insertPlainText("losing: " + QString::number(model_->numLosingOverextensions(fighterIdx)) + "\n\n");


}
