#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/String.hpp"

#define EVENT_TYPE_LIST                  \
    X(SINGLES,    "Singles Bracket")     \
    X(DOUBLES,    "Doubles Bracket")     \
    X(SIDE,       "Side Bracket")        \
    X(AMATEURS,   "Amateurs Bracket")    \
    X(MONEYMATCH, "Money Match")         \
    X(PRACTICE,   "Practice")            \
    X(FRIENDLIES, "Friendlies")          \
    X(OTHER,      "Other")

namespace rfcommon {

class RFCOMMON_PUBLIC_API EventType
{
public:
    enum Type {
#define X(name, str) name,
        EVENT_TYPE_LIST
#undef X
    };

    static EventType makeOther(const char* description);
    static EventType fromDescription(const char* description);
    static EventType fromType(Type type);
    static EventType fromIndex(int index);

    Type type() const { return type_; }
    int index() const { return static_cast<int>(type_); }

    /*!
     * \brief Gets a string representation of the event type
     */
    const char* description() const;

    bool operator==(const EventType& rhs) const;
    bool operator!=(const EventType& rhs) const;

private:
    EventType(Type type, const char* otherDesc);
    EventType(const char* description);

private:
    Type type_;
    String otherDesc_;
};

}
