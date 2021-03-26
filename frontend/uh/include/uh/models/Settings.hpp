#pragma once

#include <QString>
#include <QSharedData>

namespace uh {

class Settings : public QSharedData
{
public:
    Settings();
    ~Settings();

    QString connectIPAddress = "";
    uint16_t connectPort = 42069;

private:
    void writeSettings();
};

}
