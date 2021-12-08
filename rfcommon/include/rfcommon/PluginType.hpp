#pragma once

extern "C" {

enum class RFPluginType : unsigned char
{
    VISUALIZER    = 0x01,
    ANALYZER      = 0x02,
    REALTIME      = 0x04,
    STANDALONE    = 0x08
};

}

static inline RFPluginType operator&(const RFPluginType& lhs, const RFPluginType& rhs)
    { return static_cast<RFPluginType>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs)); }
static inline RFPluginType operator|(const RFPluginType& lhs, const RFPluginType& rhs)
    { return static_cast<RFPluginType>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs)); }

static inline bool operator!(const RFPluginType& type)
    { return static_cast<unsigned int>(type) == 0; }
