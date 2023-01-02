#pragma once

#include "nlohmann/json.hpp"

namespace nlohmann {
    template<template<typename, typename, typename...> class ObjectType,
             template<typename, typename...> class ArrayType,
             class StringType, class BooleanType, class NumberIntegerType,
             class NumberUnsignedType, class NumberFloatType,
             template<typename> class AllocatorType,
             template<typename, typename = void> class JSONSerializer,
             class BinaryType>
    class basic_json;
}

namespace rfapp {

class Config;

class ConfigAccessor
{
protected:
    ConfigAccessor(Config* config);
    Config* config() const;
    nlohmann::basic_json<>& configRoot() const;
    void saveConfig() const;

private:
    Config* config_;
};

}
