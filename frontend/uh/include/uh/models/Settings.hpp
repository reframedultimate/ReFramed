#pragma once

#include <QString>
#include <QDir>

namespace uh {

class Settings
{
public:
    Settings();
    ~Settings();

    QString connectIPAddress = "";
    uint16_t connectPort = 42069;
    QDir activeRecordingSavePath = QDir(".");

private:
    void writeSettings();
};

}
