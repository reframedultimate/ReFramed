#pragma once

#include <QWidget>

namespace Ui {
    class ConnectView;
}

namespace uh {

class ConnectView : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectView(QWidget* parent=nullptr, Qt::WindowFlags flags=Qt::Popup | Qt::Dialog);
    ~ConnectView();

private:
    Ui::ConnectView* ui_;
};

}
