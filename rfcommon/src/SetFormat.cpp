#include "rfcommon/SetFormat.hpp"
#include <cassert>

namespace rfcommon {

// ----------------------------------------------------------------------------
SetFormat::SetFormat(Type type, const String& otherDesc)
    : type_(type)
    , otherDesc_(otherDesc)
{
}

// ----------------------------------------------------------------------------
SetFormat::SetFormat(const String& desc)
    : type_([&desc]() -> Type {
#define X(type, str) if (desc == str) return type;
        SET_FORMAT_LIST
#undef X
        return OTHER;
      }())
{
    if (type_ == OTHER)
        otherDesc_ = desc;
}

// ----------------------------------------------------------------------------
String SetFormat::description() const
{
    if (type_ == OTHER)
        return otherDesc_;

#define X(type, str) if (type_ == type) return str;
    SET_FORMAT_LIST
#undef X

    std::terminate();
}

// ----------------------------------------------------------------------------
bool SetFormat::operator==(const SetFormat& rhs)
{
    if (type_ == OTHER)
        return type_ == rhs.type_ && otherDesc_ == rhs.otherDesc_;
    return type_ == rhs.type_;
}

// ----------------------------------------------------------------------------
bool SetFormat::operator!=(const SetFormat& rhs)
{
    return !operator==(rhs);
}

}
