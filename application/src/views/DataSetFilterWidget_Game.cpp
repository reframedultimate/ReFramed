#include "application/ui_DataSetFilterWidget_Game.h"
#include "application/views/DataSetFilterWidget_Game.hpp"
#include "rfcommon/DataSetFilter_Game.hpp"
#include "rfcommon/SetFormat.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_Game::DataSetFilterWidget_Game(QWidget* parent)
    : DataSetFilterWidget(new rfcommon::DataSetFilter_Game, parent)
    , ui_(new Ui::DataSetFilterWidget_Game)
{
    ui_->setupUi(contentWidget());
    setTitle("Game");
    setExpanded(true);

    ui_->comboBox_format->addItem("Any");
#define X(name, shortstr, longstr) ui_->comboBox_format->addItem(longstr);
    SET_FORMAT_LIST
#undef X
    ui_->lineEdit_formatName->setVisible(false);
    ui_->label_formatName->setVisible(false);

    ui_->minLength->setTime(QTime(0, 0));
    ui_->maxLength->setTime(QTime(0, 8));

    onFormatComboBoxChanged(0);
    onMinLengthChanged(ui_->minLength->time());
    onMaxLengthChanged(ui_->maxLength->time());

    connect(ui_->comboBox_format, SIGNAL(currentIndexChanged(int)), this, SLOT(onFormatComboBoxChanged(int)));
    connect(ui_->lineEdit_formatName, &QLineEdit::textChanged, this, &DataSetFilterWidget_Game::onFormatDescChanged);
    connect(ui_->winner, &QLineEdit::textChanged, this, &DataSetFilterWidget_Game::onWinnerTextChanged);
    connect(ui_->minLength, &QTimeEdit::timeChanged, this, &DataSetFilterWidget_Game::onMinLengthChanged);
    connect(ui_->maxLength, &QTimeEdit::timeChanged, this, &DataSetFilterWidget_Game::onMaxLengthChanged);

    updateSize();
}

// ----------------------------------------------------------------------------
DataSetFilterWidget_Game::~DataSetFilterWidget_Game()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void DataSetFilterWidget_Game::onFormatComboBoxChanged(int index)
{
    rfcommon::DataSetFilter_Game* f = static_cast<rfcommon::DataSetFilter_Game*>(filter());

    if (index == 0)
    {
        f->setAnySetFormat(true);
    }
    else
    {
        rfcommon::SetFormat::Type type = static_cast<rfcommon::SetFormat::Type>(index - 1);
        f->setAnySetFormat(false);
        if (type == rfcommon::SetFormat::OTHER)
        {
            QByteArray ba = ui_->lineEdit_formatName->text().toUtf8();
            f->setSetFormat(rfcommon::SetFormat::makeOther(ba.constData()));
        }
        else
            f->setSetFormat(rfcommon::SetFormat::fromType(type));
    }

    // Handle UI changes
    if (index == 0)
    {
        ui_->lineEdit_formatName->setVisible(false);
        ui_->label_formatName->setVisible(false);
    }
    else
    {
        rfcommon::SetFormat::Type type = static_cast<rfcommon::SetFormat::Type>(index - 1);
        ui_->lineEdit_formatName->setVisible(type == rfcommon::SetFormat::OTHER);
        ui_->label_formatName->setVisible(type == rfcommon::SetFormat::OTHER);
    }
    updateSize();
}

// ----------------------------------------------------------------------------
void DataSetFilterWidget_Game::onFormatDescChanged(const QString& text)
{
    (void)text;
    onFormatComboBoxChanged(ui_->comboBox_format->currentIndex());
}

// ----------------------------------------------------------------------------
void DataSetFilterWidget_Game::onWinnerTextChanged(const QString& text)
{
    rfcommon::DataSetFilter_Game* f = static_cast<rfcommon::DataSetFilter_Game*>(filter());
    f->setWinner(text.toStdString().c_str());
}

// ----------------------------------------------------------------------------
void DataSetFilterWidget_Game::onMinLengthChanged(const QTime& time)
{
    rfcommon::DataSetFilter_Game* f = static_cast<rfcommon::DataSetFilter_Game*>(filter());
    f->setMinLengthMs(QTime(0, 0).msecsTo(time));
}

// ----------------------------------------------------------------------------
void DataSetFilterWidget_Game::onMaxLengthChanged(const QTime& time)
{
    rfcommon::DataSetFilter_Game* f = static_cast<rfcommon::DataSetFilter_Game*>(filter());
    f->setMaxLengthMs(QTime(0, 0).msecsTo(time));
}

}
