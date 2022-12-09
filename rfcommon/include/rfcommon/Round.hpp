#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/SessionNumber.hpp"
#include "rfcommon/String.hpp"
#include <cstdint>

#define ROUND_TYPES_LIST                                \
    X(WINNERS_ROUND,   "WR", "Winners Round")           \
    X(WINNERS_QUARTER, "WQF", "Winners Quarter Finals") \
    X(WINNERS_SEMI,    "WSF", "Winners Semi Finals")    \
    X(WINNERS_FINALS,  "WF", "Winners Finals")          \
    X(LOSERS_ROUND,    "LR", "Losers Round")            \
    X(LOSERS_QUARTER,  "LQF", "Losers Quarter Finals")  \
    X(LOSERS_SEMI,     "LSF", "Losers Semi Finals")     \
    X(LOSERS_FINALS,   "LF", "Losers Finals")           \
    X(GRAND_FINALS,    "GF", "Grand Finals")            \
    X(POOLS,           "Pools", "Pools")                \
    X(FREE,            "", "")

namespace rfcommon {

class RFCOMMON_PUBLIC_API Round
{
public:
    enum Type
    {
#define X(name, shortstr, longstr) name,
        ROUND_TYPES_LIST
#undef X
    };

    static Round makeFree();
    static Round fromType(Type type, SessionNumber number=rfcommon::SessionNumber::fromValue(1));
    static Round fromIndex(int index, SessionNumber number=rfcommon::SessionNumber::fromValue(1));
    static Round fromSessionNumber(SessionNumber sessionNumber);
    static Round fromDescription(const char* desc);

    ~Round();

    Type type() const { return type_; }
    int index() const { return static_cast<int>(type_); }
    SessionNumber number() const { return number_; }
    String shortDescription() const;
    String longDescription() const;

    bool operator==(const Round& rhs) const;
    bool operator!=(const Round& rhs) const;

private:
    Round(Type type, SessionNumber number);
    Round(const char* desc);

private:
    Type type_;
    SessionNumber number_;
};

}
