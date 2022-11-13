#include "rfcommon/EventType.hpp"
#include "rfcommon/Profiler.hpp"
#include <cstring>

namespace rfcommon {

// ----------------------------------------------------------------------------
EventType::EventType(Type type, const char* otherDesc)
    : type_(type)
    , otherDesc_(otherDesc)
{
}

// ----------------------------------------------------------------------------
EventType::EventType(const char* desc)
    : type_([&desc]() -> Type {
#define X(type, str) if (strcmp(desc, str) == 0) return type;
        EVENT_TYPE_LIST
#undef X
        return OTHER;
      }())
{
    if (type_ == OTHER)
        otherDesc_ = desc;
}

EventType EventType::makeOther(const char* description) { return EventType(OTHER, description); }
EventType EventType::fromDescription(const char* description) { return EventType(description); }
EventType EventType::fromType(Type type) { assert(type != OTHER); return EventType(type, ""); }
EventType EventType::fromIndex(int index) { assert(index >= 0 && index < OTHER); return EventType(static_cast<Type>(index), ""); }

// ----------------------------------------------------------------------------
const char* EventType::description() const
{
    PROFILE(EventType, description);

    if (type_ == OTHER)
        return otherDesc_.cStr();

    switch (type_)
    {
#define X(type, str) case type: return str;
        EVENT_TYPE_LIST
#undef X
        default: std::terminate();
    }
}

// ----------------------------------------------------------------------------
bool EventType::operator==(const EventType& rhs) const
{
    if (type_ == OTHER)
        return type_ == rhs.type_ && otherDesc_ == rhs.otherDesc_;
    return type_ == rhs.type_;
}

// ----------------------------------------------------------------------------
bool EventType::operator!=(const EventType& rhs) const
{
    return !operator==(rhs);
}

}
