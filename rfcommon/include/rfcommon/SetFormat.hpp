#pragma once

#include "rfcommon/config.hpp"

#define SET_FORMAT_LIST                  \
    X(BO3,        "Bo3",  "Best of 3")   \
    X(BO5,        "Bo5",  "Best of 5")   \
    X(BO7,        "Bo7",  "Best of 7")   \
    X(FT5,        "FT5",  "First to 5")  \
    X(FT10,       "FT10", "First to 10") \
    X(FREE,       "Free", "Free Play")

namespace rfcommon {

class RFCOMMON_PUBLIC_API SetFormat
{
public:
    enum Type {
#define X(name, shortstr, longstr) name,
        SET_FORMAT_LIST
#undef X
    };

    static SetFormat fromDescription(const char* description);
    static SetFormat fromType(Type type);
    static SetFormat fromIndex(int index);

    Type type() const { return type_; }
    int index() const { return static_cast<int>(type_); }

    /*!
     * \brief Gets a string representation of the set's format.
     */
    const char* shortDescription() const;
    const char* longDescription() const;

    bool operator==(const SetFormat& rhs) const;
    bool operator!=(const SetFormat& rhs) const;

private:
    SetFormat(Type type);
    SetFormat(const char* desc);

private:
    Type type_;
};

}
