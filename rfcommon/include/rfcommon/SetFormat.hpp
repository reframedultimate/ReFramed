#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/String.hpp"

#define SET_FORMAT_LIST                      \
    X(FRIENDLIES, "Friendlies")              \
    X(PRACTICE,   "Practice")                \
    X(TRAINING,   "Training")                \
    X(COACHING,   "Coaching")                \
    X(BO3,        "Best of 3")               \
    X(BO5,        "Best of 5")               \
    X(BO7,        "Best of 7")               \
    X(FT5,        "First to 5")              \
    X(FT10,       "First to 10")             \
    X(BO3MM,      "Best of 3 Money Match")   \
    X(BO5MM,      "Best of 5 Money Match")   \
    X(BO7MM,      "Best of 7 Money Match")   \
    X(FT5MM,      "First to 5 Money Match")  \
    X(FT10MM,     "First to 10 Money Match") \
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
    int index() const { return static_cast<int>(type_); }

    /*!
     * \brief Gets a string representation of the set's format.
     */
    String description() const;

    bool operator==(const SetFormat& rhs);
    bool operator!=(const SetFormat& rhs);

private:
    Type type_;
    String otherDesc_;
};

}

