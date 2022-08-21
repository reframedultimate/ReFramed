#include "application/views/MainWindow.hpp"
#include "application/Util.hpp"

#include "rfcommon/init.h"
#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/Log.hpp"

#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
#include <QStandardPaths>

#include <iostream>

int processOptions(int argc, char** argv)
{
    NOPROFILE();

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
    RFCommonLib(const char* logPath) { result = rfcommon_init(logPath); }
    ~RFCommonLib() { rfcommon_deinit(); }

    int result;
};

int main(int argc, char** argv)
{
    NOPROFILE();

    processOptions(argc, argv);

#ifdef _WIN32
    QApplication::setStyle("fusion");
#endif
    QApplication app(argc, argv);

    QDir logPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if (logPath.exists("logs") == false)
        logPath.mkdir("logs");
    QByteArray logPathUtf8 = logPath.absoluteFilePath("logs").replace("/", QDir::separator()).toUtf8();
    RFCommonLib rfcommonLib(logPathUtf8.constData());
    if (rfcommonLib.result != 0)
    {
        QMessageBox::critical(nullptr, "Error", "Failed to initialize rfcommon library");
        return -1;
    }

    // Load hash40 strings. These are pretty much required for the
    // plugin API to work, and for user motion labels to work.
    rfcommon::Reference<rfcommon::Hash40Strings> hash40Strings;
    {
#if defined(_WIN32)
        const char* file = "share\\reframed\\data\\ParamLabels.csv";
#else
        const char* file = "share/reframed/data/ParamLabels.csv";
#endif
        hash40Strings = rfcommon::Hash40Strings::loadCSV(file);
        if (hash40Strings == nullptr)
        {
            QMessageBox::critical(nullptr,
                "Error", "Could not load file \"" + QString(file) + "\"\n\n"
                "This is an essential file and ReFramed cannot run without it. Maybe try downloading it from here?\n"
                "https://github.com/ultimate-research/param-labels");
            return -1;
        }
    }

    rfapp::MainWindow mainWindow(hash40Strings);
    
    // Make the main window as large as possible when not maximized
    mainWindow.setGeometry(rfapp::calculatePopupGeometryActiveScreen());

    mainWindow.showMaximized();
    rfcommon::Log::root()->info("Entering main loop");
    int result = app.exec();

    {
        FILE* fp = fopen("profile.dot", "w");
        if (fp)
            rfcommon::profiler->exportGraph(fp, rfcommon::Profiler::DOT);
    }

    return result;
}
