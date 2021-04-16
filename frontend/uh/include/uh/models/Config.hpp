#pragma once

#include <QJsonObject>

namespace uh {

class Config
{
public:
    Config();
    ~Config();

    void load();
    void save();

    QJsonObject root;
};

}
