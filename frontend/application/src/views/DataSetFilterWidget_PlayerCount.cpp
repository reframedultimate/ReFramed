#include "application/ui_DataSetFilterWidget_PlayerCount.h"
#include "application/views/DataSetFilterWidget_PlayerCount.hpp"

namespace uhapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_PlayerCount::DataSetFilterWidget_PlayerCount(QWidget* parent)
    : DataSetFilterWidget(nullptr, parent)
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
