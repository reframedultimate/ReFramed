#include "application/views/MainWindow.hpp"
#include "rfcommon/init.h"
#include <QApplication>
#include <QStyleFactory>
#include <QCursor>
#include <QScreen>
#include <iostream>

int processOptions(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--styles") == 0)
        {
            std::cout << "Available styles:" << std::endl;
            for (const auto& name : QStyleFactory::keys())
                std::cout << "  " << name.toStdString() << std::endl;
        }

        if (strcmp(argv[i], "--style") == 0)
        {
            if (++i >= argc)
                return -1;
            if (QApplication::setStyle(argv[i]) == nullptr)
            {
                std::cout << "Failed to set style \"" << argv[i] << "\"" << std::endl;
                return -1;
            }
        }
    }

    return 0;
}

int main(int argc, char** argv)
{
    int result;
    if (rfcommon_init() != 0)
        return -1;

    processOptions(argc, argv);

#ifdef _WIN32
    QApplication::setStyle("fusion");
#endif
    QApplication app(argc, argv);
    rfapp::MainWindow mainWindow;    
    
    // Make the main window as large as possible when not maximized
    QScreen* screen = QApplication::screenAt(QCursor::pos());
    if (screen == nullptr)
        screen = QApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    int width = screenRect.width() * 3 / 4;
    int height = screenRect.height() * 3 / 4;
    int x = (screenRect.width() - width) / 2 + screenRect.x();
    int y = (screenRect.height() - height) / 2 + screenRect.y();
    mainWindow.setGeometry(x, y, width, height);

    mainWindow.showMaximized();
    result = app.exec();

    rfcommon_deinit();
    return result;
}
