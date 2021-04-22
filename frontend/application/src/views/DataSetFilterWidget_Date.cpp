#include "application/ui_DataSetFilterWidget_Date.h"
#include "application/views/DataSetFilterWidget_Date.hpp"

namespace uhapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_Date::DataSetFilterWidget_Date(QWidget* parent)
    : DataSetFilterWidget(nullptr, parent)
    , ui_(new Ui::DataSetFilterWidget_Date)
{
    ui_->setupUi(contentWidget());
    setTitle("Date");
    updateSize();
    setExpanded(true);
}

// ----------------------------------------------------------------------------
DataSetFilterWidget_Date::~DataSetFilterWidget_Date()
{
    delete ui_;
}

}
