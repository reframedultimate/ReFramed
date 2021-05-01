#include "gmock/gmock.h"
#include "uh/HashMap.hpp"

#define NAME hash_map

using namespace testing;
using namespace uh;

static const int KEY1 = 1111;
static const int KEY2 = 2222;
static const int KEY3 = 3333;
static const int KEY4 = 4444;

struct ShittyHash
{
    typedef uint32_t HashType;
    uint32_t operator()(float value) { return 42; }
};

TEST(NAME, default_constructor)
{
    HashMap<int, float> hm;
    EXPECT_THAT(hm.count(), Eq(0));
    EXPECT_THAT(hm.begin(), Eq(hm.end()));
}

TEST(NAME, insert_increases_count)
{
    HashMap<int, float> hm;
    auto it = hm.insertOrGet(KEY1, 5.6);
    ASSERT_THAT(hm.count(), Eq(1));
    EXPECT_THAT(it->key(), Eq(KEY1));
    EXPECT_THAT(it->value(), FloatEq(5.6));
}

TEST(NAME, erase_decreases_count)
{
    HashMap<int, float> hm;
    hm.insertOrGet(KEY1, 5.6);
    ASSERT_THAT(hm.count(), Eq(1));
    EXPECT_THAT(hm.erase(KEY1), Eq(1));
    ASSERT_THAT(hm.count(), Eq(0));
}

TEST(NAME, insert_same_key_twice_doesnt_replace)
{
    HashMap<int, float> hm;
    auto it1 = hm.insertOrGet(KEY1, 5.6);
    auto it2 = hm.insertOrGet(KEY1, 8.2);
    ASSERT_THAT(hm.count(), Eq(1));
    EXPECT_THAT(it1, Eq(it2));
    EXPECT_THAT(it1->key(), Eq(KEY1));
    EXPECT_THAT(it1->value(), FloatEq(5.6));
    EXPECT_THAT(it2->key(), Eq(KEY1));
    EXPECT_THAT(it2->value(), FloatEq(5.6));
}

TEST(NAME, erase_same_key_twice_only_erases_once)
{
    HashMap<int, float> hm;
    hm.insertOrGet(KEY1, 5.6);
    ASSERT_THAT(hm.count(), Eq(1));
    EXPECT_THAT(hm.erase(KEY1), Eq(1));
    EXPECT_THAT(hm.erase(KEY1), Eq(0));
    ASSERT_THAT(hm.count(), Eq(0));
}

TEST(NAME, hash_collision_insert_ab_erase_ba)
{
    HashMap<int, float, ShittyHash> hm;
    EXPECT_THAT(hm.insertOrGet(KEY1, 5.6)->value(), FloatEq(5.6));
    EXPECT_THAT(hm.insertOrGet(KEY2, 3.4)->value(), FloatEq(3.4));
    EXPECT_THAT(hm.count(), Eq(2));
    EXPECT_THAT(hm.erase(KEY2), Eq(1));
    EXPECT_THAT(hm.erase(KEY1), Eq(1));
    EXPECT_THAT(hm.count(), Eq(0));
}

TEST(NAME, hash_collision_insert_ab_erase_ab)
{
    HashMap<int, float, ShittyHash> hm;
    EXPECT_THAT(hm.insertOrGet(KEY1, 5.6)->value(), FloatEq(5.6));
    EXPECT_THAT(hm.insertOrGet(KEY2, 3.4)->value(), FloatEq(3.4));
    EXPECT_THAT(hm.count(), Eq(2));
    EXPECT_THAT(hm.erase(KEY1), Eq(1));
    EXPECT_THAT(hm.erase(KEY2), Eq(1));
    EXPECT_THAT(hm.count(), Eq(0));
}

TEST(NAME, hash_collision_insert_ab_find_ab)
{
    HashMap<int, float, ShittyHash> hm;
    EXPECT_THAT(hm.insertOrGet(KEY1, 5.6)->value(), FloatEq(5.6));
    EXPECT_THAT(hm.insertOrGet(KEY2, 3.4)->value(), FloatEq(3.4));
    EXPECT_THAT(hm.count(), Eq(2));
    auto it1 = hm.find(KEY1);
    ASSERT_THAT(it1, Ne(hm.end()));
    EXPECT_THAT(it1->key(), Eq(KEY1));
    EXPECT_THAT(it1->value(), FloatEq(5.6));
    auto it2 = hm.find(KEY2);
    ASSERT_THAT(it2, Ne(hm.end()));
    EXPECT_THAT(it2->key(), Eq(KEY2));
    EXPECT_THAT(it2->value(), FloatEq(3.4));
}

TEST(NAME, hash_collision_insert_ab_erase_a_find_b)
{
    HashMap<int, float, ShittyHash> hm;
    EXPECT_THAT(hm.insertOrGet(KEY1, 5.6)->value(), FloatEq(5.6));
    EXPECT_THAT(hm.insertOrGet(KEY2, 3.4)->value(), FloatEq(3.4));
    EXPECT_THAT(hm.count(), Eq(2));
    EXPECT_THAT(hm.erase(KEY1), Eq(1));
    auto it = hm.find(KEY2);
    ASSERT_THAT(it, Ne(hm.end()));
    EXPECT_THAT(it->key(), Eq(KEY2));
    EXPECT_THAT(it->value(), FloatEq(3.4));
}

TEST(NAME, hash_collision_insert_ab_erase_b_find_a)
{
    HashMap<int, float, ShittyHash> hm;
    EXPECT_THAT(hm.insertOrGet(KEY1, 5.6)->value(), FloatEq(5.6));
    EXPECT_THAT(hm.insertOrGet(KEY2, 3.4)->value(), FloatEq(3.4));
    EXPECT_THAT(hm.count(), Eq(2));
    EXPECT_THAT(hm.erase(KEY2), Eq(1));
    auto it = hm.find(KEY1);
    ASSERT_THAT(it, Ne(hm.end()));
    EXPECT_THAT(it->key(), Eq(KEY1));
    EXPECT_THAT(it->value(), FloatEq(5.6));
}
