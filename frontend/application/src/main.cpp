#include "application/views/MainWindow.hpp"
#include "uh/init.h"
#include <QApplication>
#include <QStyleFactory>
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
    processOptions(argc, argv);

#ifdef _WIN32
    QApplication::setStyle("fusion");
#endif
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
