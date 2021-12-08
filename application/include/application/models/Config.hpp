#pragma once

#include <QJsonObject>

namespace rfapp {

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
