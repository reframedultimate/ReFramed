#include "uh/models/SetFormat.hpp"

namespace uh {

// ----------------------------------------------------------------------------
QString setFormatDesc(SetFormat format, const QString& otherDesc)
{
    switch (format)
    {
        case SetFormat::FRIENDLIES : return "Friendlies";
        case SetFormat::PRACTICE   : return "Practice";
        case SetFormat::BO3        : return "Bo3";
        case SetFormat::BO5        : return "Bo5";
        case SetFormat::BO7        : return "Bo7";
        case SetFormat::FT5        : return "FT5";
        case SetFormat::FT10       : return "FT10";
        case SetFormat::OTHER      : return otherDesc;
    }

    return "";
}

}
