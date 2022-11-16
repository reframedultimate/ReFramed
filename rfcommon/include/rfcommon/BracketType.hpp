#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/String.hpp"

#define BRACKET_TYPE_LIST                  \
    X(SINGLES,    "Singles Bracket")     \
    X(DOUBLES,    "Doubles Bracket")     \
    X(SIDE,       "Side Bracket")        \
    X(AMATEURS,   "Amateurs Bracket")    \
    X(MONEYMATCH, "Money Match")         \
    X(PRACTICE,   "Practice")            \
    X(FRIENDLIES, "Friendlies")          \
    X(OTHER,      "Other")

namespace rfcommon {

class RFCOMMON_PUBLIC_API BracketType
{
public:
    enum Type {
#define X(name, str) name,
        BRACKET_TYPE_LIST
#undef X
    };

    static BracketType makeOther(const char* description);
    static BracketType fromDescription(const char* description);
    static BracketType fromType(Type type);
    static BracketType fromIndex(int index);

    Type type() const { return type_; }
    int index() const { return static_cast<int>(type_); }

    /*!
     * \brief Gets a string representation of the event type
     */
    const char* description() const;

    bool operator==(const BracketType& rhs) const;
    bool operator!=(const BracketType& rhs) const;

private:
    BracketType(Type type, const char* otherDesc);
    BracketType(const char* description);

private:
    Type type_;
    String otherDesc_;
};

}
