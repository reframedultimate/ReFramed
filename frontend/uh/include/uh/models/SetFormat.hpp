#pragma once

#include <QString>

namespace uh {

enum class SetFormat : unsigned char
{
    FRIENDLIES,
    PRACTICE,
    BO3,
    BO5,
    BO7,
    FT5,
    FT10,
    OTHER
};

/*!
 * \brief Gets a string representation of the set's format.
 */
QString setFormatDesc(SetFormat format, const QString& otherDesc);

}

