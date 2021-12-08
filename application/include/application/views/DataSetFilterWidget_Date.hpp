#pragma once

#include "application/views/DataSetFilterWidget.hpp"

namespace Ui {
    class DataSetFilterWidget_Date;
}

namespace rfapp {

class DataSetFilterWidget_Date : public DataSetFilterWidget
{
    Q_OBJECT

public:
    explicit DataSetFilterWidget_Date(QWidget* parent=nullptr);
    ~DataSetFilterWidget_Date();

private slots:
    void onFromChanged(const QDateTime& dt);
    void onToChanged(const QDateTime& dt);

private:
    Ui::DataSetFilterWidget_Date* ui_;
};

}
