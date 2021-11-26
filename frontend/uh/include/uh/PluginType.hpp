#pragma once

extern "C" {

enum class UHPluginType : unsigned char
{
    VISUALIZER    = 0x01,
    ANALYZER      = 0x02,
    REALTIME      = 0x04,
    STANDALONE    = 0x08
};

}

static inline UHPluginType operator&(const UHPluginType& lhs, const UHPluginType& rhs)
    { return static_cast<UHPluginType>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs)); }
static inline UHPluginType operator|(const UHPluginType& lhs, const UHPluginType& rhs)
    { return static_cast<UHPluginType>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs)); }

static inline bool operator!(const UHPluginType& type)
    { return static_cast<unsigned int>(type) == 0; }
