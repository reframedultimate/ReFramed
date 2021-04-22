#pragma once

#include "application/views/DataSetFilterWidget.hpp"

namespace Ui {
    class DataSetFilterWidget_Date;
}

namespace uhapp {

class DataSetFilterWidget_Date : public DataSetFilterWidget
{
    Q_OBJECT

public:
    explicit DataSetFilterWidget_Date(QWidget* parent=nullptr);
    ~DataSetFilterWidget_Date();

private:
    Ui::DataSetFilterWidget_Date* ui_;
};

}
