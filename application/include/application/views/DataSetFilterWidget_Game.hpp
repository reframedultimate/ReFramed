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

private slots:
    void onFormatComboBoxChanged(int index);
    void onFormatDescChanged(const QString& text);
    void onWinnerTextChanged(const QString& text);
    void onMinLengthChanged(const QTime& time);
    void onMaxLengthChanged(const QTime& time);

private:
    Ui::DataSetFilterWidget_Game* ui_;
};

}
