#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/String.hpp"

#define SET_FORMAT_LIST          \
    X(FRIENDLIES, "Friendlies")  \
    X(PRACTICE,   "Practice")    \
    X(BO3,        "Best of 3")   \
    X(BO5,        "Best of 5")   \
    X(BO7,        "Best of 7")   \
    X(FT5,        "First to 5")  \
    X(FT10,       "First to 10") \
    X(OTHER,      "Other")

namespace rfcommon {

class RFCOMMON_PUBLIC_API SetFormat
{
public:
    enum Type {
#define X(name, str) name,
        SET_FORMAT_LIST
#undef X
    };

    SetFormat(Type type, const String& otherDesc="");
    SetFormat(const String& description);

    Type type() const { return type_; }

    /*!
     * \brief Gets a string representation of the set's format.
     */
    String description() const;

private:
    Type type_;
    String otherDesc_;
};

}

