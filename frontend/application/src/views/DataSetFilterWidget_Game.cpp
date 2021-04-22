#include "application/ui_DataSetFilterWidget_Game.h"
#include "application/views/DataSetFilterWidget_Game.hpp"

namespace uhapp {

// ----------------------------------------------------------------------------
DataSetFilterWidget_Game::DataSetFilterWidget_Game(QWidget* parent)
    : DataSetFilterWidget(nullptr, parent)
    , ui_(new Ui::DataSetFilterWidget_Game)
{
    ui_->setupUi(contentWidget());
    setTitle("Game");
    updateSize();
    setExpanded(true);
}

// ----------------------------------------------------------------------------
DataSetFilterWidget_Game::~DataSetFilterWidget_Game()
{
    delete ui_;
}

}
