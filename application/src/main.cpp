#include "application/views/MainWindow.hpp"
#include "application/models/Config.hpp"
#include "application/Util.hpp"

#include "rfcommon/init.h"
#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/Utf8.hpp"

#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTextStream>

#include <iostream>

struct RFCommonLib
{
    RFCommonLib(const char* logPath) { result = rfcommon_init(logPath); }
    ~RFCommonLib() { rfcommon_deinit(); }

    int result;
};

int main(int argc, char** argv)
{
    NOPROFILE();

#ifdef _WIN32
    QApplication::setStyle("fusion");
#endif

    QApplication app(argc, argv);
    QDir appLocalDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);

#if defined(RFCOMMON_LOGGING)
    if (appLocalDir.exists("logs") == false)
        appLocalDir.mkdir("logs");
    QByteArray logPathUtf8 = appLocalDir.absoluteFilePath("logs").replace("/", QDir::separator()).toUtf8();
    const char* logPathUtf8CStr = logPathUtf8.constData();
#else
    const char* logPathUtf8CStr = "";
#endif

    RFCommonLib rfcommonLib(logPathUtf8CStr);
    if (rfcommonLib.result != 0)
    {
        QMessageBox::critical(nullptr, "Error", "Failed to initialize rfcommon library");
        return -1;
    }

    // We have to load the config here in order to set the theme on the whole
    // application. Doing it in the MainWindow increases startup time by about 1
    // second
    std::unique_ptr<rfapp::Config> config(new rfapp::Config);
    if (config->root["theme"] == "darkstyle")
    {
        QFile f(":/qdarkstyle/dark/darkstyle.qss");
        f.open(QIODevice::ReadOnly);
        if (f.isOpen())
        {
            QTextStream ts(&f);
            qApp->setStyleSheet(ts.readAll());
        }
        QIcon::setThemeName("feather-dark");
    }
    else
    {
        QIcon::setThemeName("feather-light");
    }

    // Load hash40 strings. These are pretty much required for the
    // plugin API to work, and for user motion labels to work.
    rfcommon::Reference<rfcommon::Hash40Strings> hash40Strings;
    {
#if defined(_WIN32)
        //const char* file = "share\\reframed\\data\\motion\\ParamLabels.csv";
        const char* file = "share\\reframed\\data\\motion\\ParamLabels.dat";
#else
        //const char* file = "share/reframed/data/motion/ParamLabels.csv";
        const char* file = "share/reframed/data/motion/ParamLabels.dat";
#endif
        hash40Strings = rfcommon::Hash40Strings::loadBinary(file);
        if (hash40Strings == nullptr)
        {
            QMessageBox::critical(nullptr,
                "Error", "Could not load file \"" + QString(file) + "\"\n\n"
                "This is an essential file and ReFramed cannot run without it. Maybe try downloading it from here?\n"
                "https://github.com/ultimate-research/param-labels");
            return -1;
        }
    }

    rfcommon::Reference<rfcommon::MotionLabels> motionLabels(
                new rfcommon::MotionLabels(appLocalDir.absoluteFilePath("motionLabels.dat").replace("/", QDir::separator()).toUtf8().constData()));
    motionLabels->importLayers(appLocalDir.absoluteFilePath("motion/1_Specific.json").replace("/", QDir::separator()).toUtf8().constData());
    motionLabels->importLayers(appLocalDir.absoluteFilePath("motion/2_General.json").replace("/", QDir::separator()).toUtf8().constData());
    motionLabels->importLayers(appLocalDir.absoluteFilePath("motion/3_English.json").replace("/", QDir::separator()).toUtf8().constData());
    motionLabels->importLayers(appLocalDir.absoluteFilePath("motion/unlabeled.json").replace("/", QDir::separator()).toUtf8().constData());

    rfapp::MainWindow mainWindow(std::move(config), hash40Strings, motionLabels);

    // Make the main window as large as possible when not maximized
    mainWindow.setGeometry(rfapp::calculatePopupGeometryActiveScreen());

    mainWindow.showMaximized();
    rfcommon::Log::root()->notice("Entering main loop");
    int result = app.exec();

#if defined(RFCOMMON_PROFILER)
    {
        NOPROFILE();

        FILE* fp = rfcommon::utf8_fopen_wb("profile.dot", sizeof("profile.dot"));
        if (fp)
            rfcommon::profiler->exportGraph(fp, rfcommon::Profiler::DOT);
    }
#endif

    return result;
}
