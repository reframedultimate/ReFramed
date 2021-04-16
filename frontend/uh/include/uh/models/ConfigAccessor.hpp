#pragma once

class QJsonObject;
class QStringList;

namespace uh {

class Config;

class ConfigAccessor
{
protected:
    ConfigAccessor(Config* config);
    QJsonObject& getConfig() const;
    void saveConfig() const;

private:
    Config* config_;
};

}
