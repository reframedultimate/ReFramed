#include "application/ui_DataSetFilterWidget_Matchup.h"
#include "application/views/DataSetFilterWidget_Matchup.hpp"
#include "rfcommon/DataSetFilter_Matchup.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_Matchup::DataSetFilterWidget_Matchup(QWidget* parent)
    : DataSetFilterWidget(new rfcommon::DataSetFilter_Matchup, parent)
    , ui_(new Ui::DataSetFilterWidget_Matchup)
{
    ui_->setupUi(contentWidget());
    setTitle("Matchup");
    updateSize();
    setExpanded(true);
}

// ----------------------------------------------------------------------------
DataSetFilterWidget_Matchup::~DataSetFilterWidget_Matchup()
{
    delete ui_;
}

}
