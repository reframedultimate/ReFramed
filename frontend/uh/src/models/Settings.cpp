#include "uh/models/Settings.hpp"

#include <QDir>
#include <QStandardPaths>

namespace uh {

// ----------------------------------------------------------------------------
Settings::Settings()
{
    QString pathStr = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir path(pathStr);
    if(path.exists() == false)
        return;

}

// ----------------------------------------------------------------------------
Settings::~Settings()
{
}

}
