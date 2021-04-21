#pragma once

#include <QJsonObject>

namespace uhapp {

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
