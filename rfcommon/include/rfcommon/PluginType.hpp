#pragma once

extern "C" {

enum class RFPluginType : unsigned char
{
    UI            = 0x01,
    REALTIME      = 0x02,
    REPLAY        = 0x04,
    VISUALIZER    = 0x08,
    STANDALONE    = 0x10,

    ALL           = 0xFF
};

}

static inline RFPluginType operator&(const RFPluginType& lhs, const RFPluginType& rhs)
    { return static_cast<RFPluginType>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs)); }
static inline RFPluginType operator|(const RFPluginType& lhs, const RFPluginType& rhs)
    { return static_cast<RFPluginType>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs)); }

static inline bool operator!(const RFPluginType& type)
    { return static_cast<unsigned int>(type) == 0; }
