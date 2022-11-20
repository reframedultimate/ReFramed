#include "application/models/MetaDataEditModel.hpp"
#include "application/widgets/MetaDataEditWidget_Event.hpp"

#include "rfcommon/BracketType.hpp"
#include "rfcommon/GameMetaData.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QGroupBox>
#include <QComboBox>

namespace rfapp {

// ----------------------------------------------------------------------------
MetaDataEditWidget_Event::MetaDataEditWidget_Event(MetaDataEditModel* model, QWidget* parent)
    : MetaDataEditWidget(model, parent)
    , label_bracketURL_(new QLabel("Bracket URL:"))
    , lineEdit_bracketURL_(new QLineEdit)
    , lineEdit_otherBracketType_(new QLineEdit)
{
    setTitle("Event");

    QComboBox* bracketType = new QComboBox;
#define X(type, name) bracketType->addItem(name);
    BRACKET_TYPE_LIST
#undef X
    bracketType->setCurrentIndex(rfcommon::BracketType::FRIENDLIES);

    label_bracketURL_->setVisible(false);
    lineEdit_bracketURL_->setVisible(false);
    lineEdit_otherBracketType_->setVisible(false);

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Bracket Type:", bracketType);
    layout->addWidget(lineEdit_otherBracketType_);
    layout->addRow(label_bracketURL_, lineEdit_bracketURL_);

    contentWidget()->setLayout(layout);
    updateSize();

    connect(bracketType, qOverload<int>(&QComboBox::currentIndexChanged), this, &MetaDataEditWidget_Event::onComboBoxBracketTypeChanged);
}

// ----------------------------------------------------------------------------
MetaDataEditWidget_Event::~MetaDataEditWidget_Event()
{}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Event::onComboBoxBracketTypeChanged(int index)
{
    rfcommon::BracketType type = index == rfcommon::BracketType::OTHER ?
                rfcommon::BracketType::makeOther(lineEdit_otherBracketType_->text().toUtf8().constData()) :
                rfcommon::BracketType::fromIndex(index);

    switch (type.type())
    {
        case rfcommon::BracketType::SINGLES:
        case rfcommon::BracketType::DOUBLES:
        case rfcommon::BracketType::AMATEURS:
        case rfcommon::BracketType::SIDE:
            label_bracketURL_->setVisible(true);
            lineEdit_bracketURL_->setVisible(true);
            break;

        case rfcommon::BracketType::MONEYMATCH:
        case rfcommon::BracketType::PRACTICE:
        case rfcommon::BracketType::FRIENDLIES:
        case rfcommon::BracketType::OTHER:
            label_bracketURL_->setVisible(false);
            lineEdit_bracketURL_->setVisible(false);
            break;
    }

    lineEdit_otherBracketType_->setVisible(index == rfcommon::BracketType::OTHER);
    updateSize();

    model_->dispatcher.dispatch(&MetaDataEditListener::onBracketTypeChangedUI, type);
}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Event::onAdoptMetaData(const MappingInfoList& map, const MetaDataList& mdata)
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Event::onOverwriteMetaData(const MappingInfoList& map, const MetaDataList& mdata)
{

}

// ----------------------------------------------------------------------------
void MetaDataEditWidget_Event::onMetaDataCleared(const MappingInfoList& map, const MetaDataList& mdata) {}
void MetaDataEditWidget_Event::onBracketTypeChangedUI(rfcommon::BracketType bracketType) {}
void MetaDataEditWidget_Event::onMetaDataTimeChanged(rfcommon::TimeStamp timeStarted, rfcommon::TimeStamp timeEnded) {}
void MetaDataEditWidget_Event::onMetaDataTournamentDetailsChanged() {}
void MetaDataEditWidget_Event::onMetaDataEventDetailsChanged() {}
void MetaDataEditWidget_Event::onMetaDataCommentatorsChanged() {}
void MetaDataEditWidget_Event::onMetaDataGameDetailsChanged() {}
void MetaDataEditWidget_Event::onMetaDataPlayerDetailsChanged() {}
void MetaDataEditWidget_Event::onMetaDataWinnerChanged(int winnerPlayerIdx) {}
void MetaDataEditWidget_Event::onMetaDataTrainingSessionNumberChanged(rfcommon::SessionNumber number) {}

}
