#pragma once

#include "application/views/DataSetFilterWidget.hpp"

namespace Ui {
    class DataSetFilterWidget_PlayerCount;
}

namespace uhapp {

class DataSetFilterWidget_PlayerCount : public DataSetFilterWidget
{
    Q_OBJECT

public:
    explicit DataSetFilterWidget_PlayerCount(QWidget* parent=nullptr);
    ~DataSetFilterWidget_PlayerCount();

private:
    Ui::DataSetFilterWidget_PlayerCount* ui_;
};

}
