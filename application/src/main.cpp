#include "application/views/MainWindow.hpp"
#include "application/Util.hpp"

#include "rfcommon/init.h"
#include "rfcommon/Hash40Strings.hpp"

#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>

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

struct RFCommonLib
{
    RFCommonLib() { result = rfcommon_init(); }
    ~RFCommonLib() { rfcommon_deinit(); }

    int result;
};

int main(int argc, char** argv)
{
    processOptions(argc, argv);

#ifdef _WIN32
    QApplication::setStyle("fusion");
#endif
    QApplication app(argc, argv);

    RFCommonLib rfcommonLib;
    if (rfcommonLib.result != 0)
    {
        QMessageBox::critical(nullptr, "Error", "Failed to initialize rfcommon library");
        return -1;
    }

    // Load hash40 strings. These are pretty much required for the
    // plugin API to work, and for user motion labels to work.
    std::unique_ptr<rfcommon::Hash40Strings> hash40Strings;
    {
#if defined(_WIN32)
        const char* file = "share\\reframed\\data\\ParamLabels.csv";
#else
        const char* file = "share/reframed/data/ParamLabels.csv";
#endif
        hash40Strings.reset(rfcommon::Hash40Strings::loadCSV(file));
        if (hash40Strings == nullptr)
        {
            QMessageBox::critical(nullptr,
                "Error", "Could not load file \"" + QString(file) + "\"\n\n"
                "This is an essential file and ReFramed cannot run without it. Maybe try downloading it from here?\n"
                "https://github.com/ultimate-research/param-labels");
            return -1;
        }
    }

    rfapp::MainWindow mainWindow(std::move(hash40Strings));
    
    // Make the main window as large as possible when not maximized
    mainWindow.setGeometry(rfapp::calculatePopupGeometryActiveScreen());

    mainWindow.showMaximized();
    return app.exec();
}
