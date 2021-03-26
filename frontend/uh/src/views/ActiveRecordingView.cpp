#include "uh/views/ActiveRecordingView.hpp"
#include "uh/ui_ActiveRecordingView.h"

namespace uh {

// ----------------------------------------------------------------------------
ActiveRecordingView::ActiveRecordingView(QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::ActiveRecordingView)
{
    ui_->setupUi(this);
}

// ----------------------------------------------------------------------------
ActiveRecordingView::~ActiveRecordingView()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setWaitingForGame()
{
    ui_->stackedWidget->setCurrentWidget(ui_->page_waiting);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setActive()
{
    ui_->stackedWidget->setCurrentWidget(ui_->page_active);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setDisconnected()
{
    ui_->stackedWidget->setCurrentWidget(ui_->page_disconnected);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setTimeStarted(const QDateTime& date)
{
    ui_->label_date->setText(date.toString());
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setStageName(const QString& stage)
{
    ui_->label_stage->setText(stage);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setPlayerCount(int count)
{
    // Clear layout
    QLayoutItem* item;
    while ((item = ui_->groupBox_playerInfo->layout()->takeAt(0)) != nullptr)
    {
        if (item->layout() != nullptr)
            item->layout()->deleteLater();
        if (item->widget() != nullptr)
            item->widget()->deleteLater();
    }

    tags_.resize(count);
    fighterName_.resize(count);
    fighterStatus_.resize(count);
    fighterDamage_.resize(count);
    fighterStocks_.resize(count);

    for (int i = 0; i != count; ++i)
    {
        tags_[i] = new QGroupBox;
        fighterName_[i] = new QLabel();
        fighterStatus_[i] = new QLabel();
        fighterDamage_[i] = new QLabel();
        fighterStocks_[i] = new QLabel();

        QFormLayout* layout = new QFormLayout;
        layout->addRow(
            new QLabel("<html><head/><body><p><span style=\" font-weight:600;\">Fighter:</span></p></body></html>"),
            fighterName_[i]);
        layout->addRow(
            new QLabel("<html><head/><body><p><span style=\" font-weight:600;\">State:</span></p></body></html>"),
            fighterStatus_[i]);
        layout->addRow(
            new QLabel("<html><head/><body><p><span style=\" font-weight:600;\">Damage:</span></p></body></html>"),
            fighterDamage_[i]);
        layout->addRow(
            new QLabel("<html><head/><body><p><span style=\" font-weight:600;\">Stocks:</span></p></body></html>"),
            fighterStocks_[i]);

        tags_[i]->setLayout(layout);
        ui_->groupBox_playerInfo->layout()->addWidget(tags_[i]);
    }
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setPlayerTag(int index, const QString& tag)
{
    tags_[index]->setTitle(tag);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setPlayerFighterName(int index, const QString& fighterName)
{
    fighterName_[index]->setText(fighterName);
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setPlayerStatus(unsigned int frame, int index, unsigned int status)
{
    (void)frame;
    fighterStatus_[index]->setText(QString::number(status));
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setPlayerDamage(unsigned int frame, int index, float damage)
{
    (void)frame;
    fighterDamage_[index]->setText(QString::number(damage));
}

// ----------------------------------------------------------------------------
void ActiveRecordingView::setPlayerStockCount(unsigned int frame, int index, unsigned char stocks)
{
    (void)frame;
    fighterStocks_[index]->setText(QString::number(stocks));
}

}
