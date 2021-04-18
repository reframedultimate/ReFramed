#pragma once

#include <QString>

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

    SetFormat(Type type, const QString& otherDesc="");
    SetFormat(const QString& description);

    Type type() const { return type_; }

    /*!
     * \brief Gets a string representation of the set's format.
     */
    QString description() const;

private:
    Type type_;
    QString otherDesc_;
};

}

