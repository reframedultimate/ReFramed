#pragma once

#include "application/views/DataSetFilterWidget.hpp"

namespace Ui {
    class DataSetFilterWidget_Matchup;
}

namespace uhapp {

class DataSetFilterWidget_Matchup : public DataSetFilterWidget
{
    Q_OBJECT

public:
    explicit DataSetFilterWidget_Matchup(QWidget* parent=nullptr);
    ~DataSetFilterWidget_Matchup();

private:
    Ui::DataSetFilterWidget_Matchup* ui_;
};

}
