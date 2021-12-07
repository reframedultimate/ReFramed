#pragma once

#include "application/views/DataSetFilterWidget.hpp"

namespace Ui {
    class DataSetFilterWidget_Stage;
}

namespace uhapp {

class DataSetFilterWidget_Stage : public DataSetFilterWidget
{
    Q_OBJECT

public:
    explicit DataSetFilterWidget_Stage(QWidget* parent=nullptr);
    ~DataSetFilterWidget_Stage();

private:
    Ui::DataSetFilterWidget_Stage* ui_;
};

}
