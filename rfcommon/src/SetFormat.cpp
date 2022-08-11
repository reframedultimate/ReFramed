#include "rfcommon/SetFormat.hpp"
#include <cassert>
#include <exception>

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
#define X(type, shortstr, longstr) if (desc == shortstr || desc == longstr) return type;
        SET_FORMAT_LIST
#undef X
        return OTHER;
      }())
{
    if (type_ == OTHER)
        otherDesc_ = desc;
}

// ----------------------------------------------------------------------------
const char* SetFormat::shortDescription() const
{
    if (type_ == OTHER)
        return otherDesc_.cStr();

    switch (type_)
    {
#define X(type, shortstr, longstr) case type: return shortstr;
        SET_FORMAT_LIST
#undef X
        default: std::terminate();
    }
}

// ----------------------------------------------------------------------------
const char* SetFormat::longDescription() const
{
    if (type_ == OTHER)
        return otherDesc_.cStr();

    switch (type_)
    {
#define X(type, shortstr, longstr) case type: return longstr;
        SET_FORMAT_LIST
#undef X
        default: std::terminate();
    }
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
