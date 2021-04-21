#pragma once

#include <cstdint>

namespace uh {

class PluginInterface
{
public:
    uint32_t version() const { return 0; }

    template <typename T>
    bool registerFactory() { return false; }

    template <typename T>
    void unregisterFactory() {}
};

}
