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

// ----------------------------------------------------------------------------
SetFormat descToSetFormat(const QString& desc)
{
    if (desc == "Friendlies") return SetFormat::FRIENDLIES;
    if (desc == "Practice")   return SetFormat::PRACTICE;
    if (desc == "Bo3")        return SetFormat::BO3;
    if (desc == "Bo5")        return SetFormat::BO5;
    if (desc == "Bo7")        return SetFormat::BO7;
    if (desc == "FT5")        return SetFormat::FT5;
    if (desc == "FT10")       return SetFormat::FT10;
    return SetFormat::OTHER;
}

}
