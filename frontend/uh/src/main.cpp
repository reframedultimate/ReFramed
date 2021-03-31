#include "uh/views/MainWindow.hpp"
#include "uh/platform/tcp_socket.h"
#include <QApplication>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    uh::MainWindow mainWindow;

    if (tcp_socket_global_init() != 0)
        goto init_sockets_failed;
    
    mainWindow.show();
    int result = app.exec();

    tcp_socket_global_deinit();
    return result;

    init_sockets_failed: return -1;
}
