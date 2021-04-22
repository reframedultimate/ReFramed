#pragma once

#include "application/views/DataSetFilterWidget.hpp"

namespace Ui {
    class DataSetFilterWidget_Game;
}

namespace uhapp {

class DataSetFilterWidget_Game : public DataSetFilterWidget
{
    Q_OBJECT

public:
    explicit DataSetFilterWidget_Game(QWidget* parent=nullptr);
    ~DataSetFilterWidget_Game();

private:
    Ui::DataSetFilterWidget_Game* ui_;
};

}
