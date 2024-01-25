#include "application/views/MainWindow.hpp"
#include "application/models/Config.hpp"
#include "application/Util.hpp"

#include "rfcommon/init.h"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MappedFile.hpp"
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

    // Copy some files over to the config directory if necessary
    if (appLocalDir.exists("mappingInfo.json") == false)
        QFile::copy("./share/reframed/data/mappingInfo.json", appLocalDir.absoluteFilePath("mappingInfo.json"));

    rfcommon::Reference<rfcommon::MotionLabels> motionLabels(
                new rfcommon::MotionLabels(appLocalDir.absoluteFilePath("motionLabels.dat").replace("/", QDir::separator()).toUtf8().constData()));
    if (motionLabels->layerCount() == 0)
    {
        rfcommon::MappedFile file;
        if (file.open(appLocalDir.absoluteFilePath("mappingInfo.json").replace("/", QDir::separator()).toUtf8().constData()))
        {
            rfcommon::Reference<rfcommon::MappingInfo> map = rfcommon::MappingInfo::load(file.address(), file.size());
            if (map.notNull())
            {
                motionLabels->importGoogleDocCSV(
                    QDir("share/reframed/data").absoluteFilePath("ReFramed User Labels.csv")
                        .replace("/", QDir::separator()).toUtf8().constData(), map);
                motionLabels->save();
            }
        }
    }

    rfapp::MainWindow mainWindow(std::move(config), motionLabels);

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
