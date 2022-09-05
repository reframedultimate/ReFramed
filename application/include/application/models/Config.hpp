#pragma once

#include "nlohmann/json.hpp"

namespace rfapp {

class Config
{
public:
    Config();
    ~Config();

    void load();
    void save();

    nlohmann::json root;
};

}
