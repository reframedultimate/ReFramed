#include "application/views/MainWindow.hpp"
#include "uh/init.h"
#include <QApplication>

int main(int argc, char** argv)
{
    int result;
    QApplication app(argc, argv);
    uhapp::MainWindow mainWindow;

    if (uh_init() != 0)
        goto init_uh_library_failed;

    mainWindow.show();
    result = app.exec();

    uh_deinit();
    return result;

    init_uh_library_failed: return -1;
}
