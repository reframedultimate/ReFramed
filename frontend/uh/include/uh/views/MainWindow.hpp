#pragma once

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

namespace uh {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private slots:
    void onConnectActionTriggered();

private:
    Ui::MainWindow* ui_;
};

}

