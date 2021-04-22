#include "application/ui_DataSetFilterWidget_Player.h"
#include "application/views/DataSetFilterWidget_Player.hpp"

namespace uhapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_Player::DataSetFilterWidget_Player(QWidget* parent)
    : DataSetFilterWidget(nullptr, parent)
    , ui_(new Ui::DataSetFilterWidget_Player)
{
    ui_->setupUi(contentWidget());
    updateSize();
    setExpanded(true);
}

// ----------------------------------------------------------------------------
DataSetFilterWidget_Player::~DataSetFilterWidget_Player()
{
    delete ui_;
}

}
