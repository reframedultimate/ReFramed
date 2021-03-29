#include "uh/views/MainWindow.hpp"
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    uh::MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}
