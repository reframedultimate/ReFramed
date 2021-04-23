#include "application/ui_DataSetFilterWidget_Stage.h"
#include "application/views/DataSetFilterWidget_Stage.hpp"
#include "uh/DataSetFilter_Stage.hpp"

namespace uhapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_Stage::DataSetFilterWidget_Stage(QWidget* parent)
    : DataSetFilterWidget(new uh::DataSetFilter_Stage, parent)
    , ui_(new Ui::DataSetFilterWidget_Stage)
{
    ui_->setupUi(contentWidget());
    setTitle("Stage");
    updateSize();
    setExpanded(true);
}

// ----------------------------------------------------------------------------
DataSetFilterWidget_Stage::~DataSetFilterWidget_Stage()
{
    delete ui_;
}

}
