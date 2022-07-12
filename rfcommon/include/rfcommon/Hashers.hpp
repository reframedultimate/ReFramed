#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/String.hpp"
#include <cstdint>

namespace rfcommon {

static inline uint32_t hash32_jenkins_oaat(const void* key, int len)
{
    uint32_t hash = 0;
    for(int i = 0; i != len; ++i)
    {
        hash += *(static_cast<const uint8_t*>(key) + i);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 1);
    hash += (hash << 15);
    return hash;
}

static inline uint32_t hash32_combine(uint32_t lhs, uint32_t rhs)
{
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
}

template <typename T, typename H=uint32_t>
struct Hasher
{
    typedef H HashType;
};

template <>
struct Hasher<uint8_t, uint32_t>
{
    typedef uint32_t HashType;
    HashType operator()(uint8_t value) const {
        return hash32_combine(
            static_cast<uint32_t>(value) << 0,
            hash32_combine(
                static_cast<uint32_t>(value) << 8,
                hash32_combine(
                    static_cast<uint32_t>(value) << 16,
                    static_cast<uint32_t>(value) << 24
                )
            )
        );
    }
};

template <>
struct Hasher<uint16_t, uint32_t>
{
    typedef uint32_t HashType;
    HashType operator()(uint16_t value) const {
        return hash32_combine(
            static_cast<HashType>(value) << 0,
            static_cast<HashType>(value) << 16
        );
    }
};

template <>
struct Hasher<uint32_t, uint32_t>
{
    typedef uint32_t HashType;
    HashType operator()(uint32_t value) const {
        return hash32_jenkins_oaat(&value, 4);
    }
};

template <>
struct Hasher<uint64_t, uint32_t>
{
    typedef uint32_t HashType;
    HashType operator()(uint64_t value) const {
        return hash32_jenkins_oaat(&value, 8);
    }
};

template <>
struct Hasher<int, uint32_t>
{
    typedef uint32_t HashType;
    uint32_t operator()(int value) const {
        return hash32_jenkins_oaat(&value, 4);
    }
};

template <int N>
struct Hasher<SmallString<N>, uint32_t>
{
    typedef uint32_t HashType;
    uint32_t operator()(const SmallString<N>& s) const {
        return hash32_jenkins_oaat(s.data(), s.count());
    }
};

template <typename P>
struct Hasher<P*, uint32_t>
{
    typedef uint32_t HashType;
    uint32_t operator()(P* p) const {
        return hash32_combine(
            static_cast<uint32_t>(reinterpret_cast<size_t>(p) / sizeof(P*)),
            static_cast<uint32_t>((reinterpret_cast<size_t>(p) / sizeof(P*)) >> 32)
        );
    }
};

}
