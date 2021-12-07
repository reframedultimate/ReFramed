#include "application/ui_DataSetFilterWidget_Matchup.h"
#include "application/views/DataSetFilterWidget_Matchup.hpp"
#include "uh/DataSetFilter_Matchup.hpp"

namespace uhapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_Matchup::DataSetFilterWidget_Matchup(QWidget* parent)
    : DataSetFilterWidget(new uh::DataSetFilter_Matchup, parent)
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
