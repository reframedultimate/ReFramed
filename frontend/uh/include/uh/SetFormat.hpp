#pragma once

#include <string>

namespace uh {

class SetFormat
{
public:
    enum Type {
        FRIENDLIES,
        PRACTICE,
        BO3,
        BO5,
        BO7,
        FT5,
        FT10,
        OTHER
    };

    SetFormat(Type type, const std::string& otherDesc="");
    SetFormat(const std::string& description);

    Type type() const { return type_; }

    /*!
     * \brief Gets a string representation of the set's format.
     */
    std::string description() const;

private:
    Type type_;
    std::string otherDesc_;
};

}

