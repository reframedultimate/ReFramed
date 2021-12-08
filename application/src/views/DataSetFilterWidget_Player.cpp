#include "application/ui_DataSetFilterWidget_Player.h"
#include "application/views/DataSetFilterWidget_Player.hpp"
#include "rfcommon/DataSetFilter_Player.hpp"

namespace rfapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_Player::DataSetFilterWidget_Player(QWidget* parent)
    : DataSetFilterWidget(new rfcommon::DataSetFilter_Player, parent)
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
