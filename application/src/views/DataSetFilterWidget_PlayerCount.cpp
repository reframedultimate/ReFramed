#include "application/ui_DataSetFilterWidget_PlayerCount.h"
#include "application/views/DataSetFilterWidget_PlayerCount.hpp"
#include "rfcommon/DataSetFilter_PlayerCount.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_PlayerCount::DataSetFilterWidget_PlayerCount(QWidget* parent)
    : DataSetFilterWidget(new rfcommon::DataSetFilter_PlayerCount, parent)
    , ui_(new Ui::DataSetFilterWidget_PlayerCount)
{
    ui_->setupUi(contentWidget());
    setTitle("PlayerCount");
    updateSize();
    setExpanded(true);
}

// ----------------------------------------------------------------------------
DataSetFilterWidget_PlayerCount::~DataSetFilterWidget_PlayerCount()
{
    delete ui_;
}

}
