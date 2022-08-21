#include "application/ui_DataSetFilterWidget_Date.h"
#include "application/views/DataSetFilterWidget_Date.hpp"
#include "rfcommon/DataSetFilter_Date.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_Date::DataSetFilterWidget_Date(QWidget* parent)
    : DataSetFilterWidget(new rfcommon::DataSetFilter_Date, parent)
    , ui_(new Ui::DataSetFilterWidget_Date)
{
    ui_->setupUi(contentWidget());
    setTitle("Date");
    updateSize();
    setExpanded(true);

    QDateTime now = QDateTime::currentDateTime();
    ui_->from->setDateTime(now.addMonths(-1));
    ui_->to->setDateTime(now);

    onFromChanged(ui_->from->dateTime());
    onToChanged(ui_->to->dateTime());

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
    PROFILE(DataSetFilterWidget_Date, onFromChanged);

    rfcommon::DataSetFilter_Date* f = static_cast<rfcommon::DataSetFilter_Date*>(filter());
    f->setStartTimeMs(dt.toMSecsSinceEpoch());
}

// ----------------------------------------------------------------------------
void DataSetFilterWidget_Date::onToChanged(const QDateTime& dt)
{
    PROFILE(DataSetFilterWidget_Date, onToChanged);

    rfcommon::DataSetFilter_Date* f = static_cast<rfcommon::DataSetFilter_Date*>(filter());
    f->setEndTimeMs(dt.toMSecsSinceEpoch());
}

}
