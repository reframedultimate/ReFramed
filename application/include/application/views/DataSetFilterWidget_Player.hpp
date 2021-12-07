#pragma once

#include "application/views/DataSetFilterWidget.hpp"

namespace Ui {
    class DataSetFilterWidget_Player;
}

namespace uhapp {

class DataSetFilterWidget_Player : public DataSetFilterWidget
{
    Q_OBJECT

public:
    explicit DataSetFilterWidget_Player(QWidget* parent=nullptr);
    ~DataSetFilterWidget_Player();

private:
    Ui::DataSetFilterWidget_Player* ui_;
};

}
