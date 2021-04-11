#include "uh/models/SetFormat.hpp"

namespace uh {

// ----------------------------------------------------------------------------
SetFormat::SetFormat(Type type, const QString& otherDesc)
    : type_(type)
    , otherDesc_(otherDesc)
{
}

// ----------------------------------------------------------------------------
SetFormat::SetFormat(const QString& desc)
    : type_([&desc]() -> Type {
        if (desc == "Friendlies") return FRIENDLIES;
        if (desc == "Practice")   return PRACTICE;
        if (desc == "Bo3")        return BO3;
        if (desc == "Bo5")        return BO5;
        if (desc == "Bo7")        return BO7;
        if (desc == "FT5")        return FT5;
        if (desc == "FT10")       return FT10;
        return OTHER;
      }())
{
    if (type_ == OTHER)
        otherDesc_ = desc;
}

// ----------------------------------------------------------------------------
QString SetFormat::description() const
{
    switch (type_)
    {
        case FRIENDLIES : return "Friendlies";
        case PRACTICE   : return "Practice";
        case BO3        : return "Bo3";
        case BO5        : return "Bo5";
        case BO7        : return "Bo7";
        case FT5        : return "FT5";
        case FT10       : return "FT10";
        case OTHER      : return otherDesc_;
    }

    assert(false);
    return "";
}

}
