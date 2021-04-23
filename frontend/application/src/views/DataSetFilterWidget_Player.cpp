#include "application/ui_DataSetFilterWidget_Player.h"
#include "application/views/DataSetFilterWidget_Player.hpp"
#include "uh/DataSetFilter_Player.hpp"

namespace uhapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_Player::DataSetFilterWidget_Player(QWidget* parent)
    : DataSetFilterWidget(new uh::DataSetFilter_Player, parent)
    , ui_(new Ui::DataSetFilterWidget_Player)
{
    ui_->setupUi(contentWidget());
    setTitle("Player");
    updateSize();
    setExpanded(true);
}

// ----------------------------------------------------------------------------
DataSetFilterWidget_Player::~DataSetFilterWidget_Player()
{
    delete ui_;
}

}
