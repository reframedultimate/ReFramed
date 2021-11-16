#pragma once

extern "C" {

enum class UHPluginType : unsigned char
{
    VISUALIZER    = 0x01,
    ANALYZER      = 0x02,
    REALTIME = 0x04,
    STANDALONE    = 0x08
};

}
