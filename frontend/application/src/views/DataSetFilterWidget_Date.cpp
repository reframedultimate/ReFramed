#include "application/ui_DataSetFilterWidget_Date.h"
#include "application/views/DataSetFilterWidget_Date.hpp"
#include "uh/DataSetFilter_Date.hpp"

namespace uhapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_Date::DataSetFilterWidget_Date(QWidget* parent)
    : DataSetFilterWidget(new uh::DataSetFilter_Date, parent)
    , ui_(new Ui::DataSetFilterWidget_Date)
{
    ui_->setupUi(contentWidget());
    setTitle("Date");
    updateSize();
    setExpanded(true);

    uh::DataSetFilter_Date* f = static_cast<uh::DataSetFilter_Date*>(filter());
    QDateTime now = QDateTime::currentDateTime();
    f->setStartTimeMs(now.toMSecsSinceEpoch());
    f->setEndTimeMs(now.toMSecsSinceEpoch());
    ui_->from->setDateTime(now.addMonths(-1));
    ui_->to->setDateTime(now);

    connect(ui_->from, &QDateTimeEdit::dateTimeChanged, this, &DataSetFilterWidget_Date::onFromChanged);
    connect(ui_->to, &QDateTimeEdit::dateTimeChanged, this, &DataSetFilterWidget_Date::onToChanged);
}

// ----------------------------------------------------------------------------
DataSetFilterWidget_Date::~DataSetFilterWidget_Date()
{
    delete ui_;
}

// ----------------------------------------------------------------------------
void DataSetFilterWidget_Date::onFromChanged(const QDateTime& dt)
{
    uh::DataSetFilter_Date* f = static_cast<uh::DataSetFilter_Date*>(filter());
    f->setStartTimeMs(dt.toMSecsSinceEpoch());
}

// ----------------------------------------------------------------------------
void DataSetFilterWidget_Date::onToChanged(const QDateTime& dt)
{
    uh::DataSetFilter_Date* f = static_cast<uh::DataSetFilter_Date*>(filter());
    f->setEndTimeMs(dt.toMSecsSinceEpoch());
}

}
