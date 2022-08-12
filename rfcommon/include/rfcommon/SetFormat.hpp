#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/String.hpp"

#define SET_FORMAT_LIST                                 \
    X(FRIENDLIES, "Friendlies", "Friendlies")           \
    X(PRACTICE,   "Practice", "Practice")               \
    X(TRAINING,   "Training", "Training")               \
    X(COACHING,   "Coaching", "Coaching")               \
    X(BO3,        "Bo3", "Best of 3")                   \
    X(BO5,        "Bo5", "Best of 5")                   \
    X(BO7,        "Bo7", "Best of 7")                   \
    X(FT5,        "FT5", "First to 5")                  \
    X(FT10,       "FT10", "First to 10")                \
    X(BO3MM,      "Bo3 MM", "Best of 3 Money Match")    \
    X(BO5MM,      "Bo5 MM", "Best of 5 Money Match")    \
    X(BO7MM,      "Bo7 MM", "Best of 7 Money Match")    \
    X(FT5MM,      "FT5 MM", "First to 5 Money Match")   \
    X(FT10MM,     "FT10 MM", "First to 10 Money Match") \
    X(OTHER,      "Other", "Other")

namespace rfcommon {

class RFCOMMON_PUBLIC_API SetFormat
{
public:
    enum Type {
#define X(name, shortstr, longstr) name,
        SET_FORMAT_LIST
#undef X
    };

    static SetFormat makeOther(const char* description);
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

    bool operator==(const SetFormat& rhs);
    bool operator!=(const SetFormat& rhs);

private:
    SetFormat(Type type, const char* otherDesc);
    SetFormat(const char* description);

private:
    Type type_;
    String otherDesc_;
};

}
