#include "gmock/gmock.h"
#include "rfcommon/HashMap.hpp"

#define NAME hash_map

using namespace testing;
using namespace rfcommon;

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
    EXPECT_THAT(hm.erase(KEY1), IsTrue());
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
    EXPECT_THAT(hm.erase(KEY1), IsTrue());
    EXPECT_THAT(hm.erase(KEY1), IsFalse());
    ASSERT_THAT(hm.count(), Eq(0));
}

TEST(NAME, hash_collision_insert_ab_erase_ba)
{
    HashMap<int, float, ShittyHash> hm;
    EXPECT_THAT(hm.insertOrGet(KEY1, 5.6)->value(), FloatEq(5.6));
    EXPECT_THAT(hm.insertOrGet(KEY2, 3.4)->value(), FloatEq(3.4));
    EXPECT_THAT(hm.count(), Eq(2));
    EXPECT_THAT(hm.erase(KEY2), IsTrue());
    EXPECT_THAT(hm.erase(KEY1), IsTrue());
    EXPECT_THAT(hm.count(), Eq(0));
}

TEST(NAME, hash_collision_insert_ab_erase_ab)
{
    HashMap<int, float, ShittyHash> hm;
    EXPECT_THAT(hm.insertOrGet(KEY1, 5.6)->value(), FloatEq(5.6));
    EXPECT_THAT(hm.insertOrGet(KEY2, 3.4)->value(), FloatEq(3.4));
    EXPECT_THAT(hm.count(), Eq(2));
    EXPECT_THAT(hm.erase(KEY1), IsTrue());
    EXPECT_THAT(hm.erase(KEY2), IsTrue());
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
    EXPECT_THAT(hm.erase(KEY1), IsTrue());
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
    EXPECT_THAT(hm.erase(KEY2), IsTrue());
    auto it = hm.find(KEY1);
    ASSERT_THAT(it, Ne(hm.end()));
    EXPECT_THAT(it->key(), Eq(KEY1));
    EXPECT_THAT(it->value(), FloatEq(5.6));
}

TEST(NAME, rehash)
{
    HashMap<int, String, ShittyHash> hm(8);
    ASSERT_THAT(hm.insertIfNew(0, "Wolf"), Ne(hm.end()));
    ASSERT_THAT(hm.insertIfNew(1, "Ike"), Ne(hm.end()));
    ASSERT_THAT(hm.insertIfNew(2, "Snake"), Ne(hm.end()));
    ASSERT_THAT(hm.insertIfNew(3, "Wario"), Ne(hm.end()));

    const auto print = [&hm]() {
        auto result = Vector<String>::makeReserved(hm.count());
        for (const auto it : hm)
            result.push(it->value());
        puts("hashmap:");
        for (const auto& it : result)
            printf("  %s\n", it.cStr());
    };

    print();
    hm.rehash();
    print();

    ASSERT_THAT(hm.find(0), Ne(hm.end())); EXPECT_THAT(hm.find(0)->value(), Eq("Wolf"));
    ASSERT_THAT(hm.find(1), Ne(hm.end())); EXPECT_THAT(hm.find(1)->value(), Eq("Ike"));
    ASSERT_THAT(hm.find(2), Ne(hm.end())); EXPECT_THAT(hm.find(2)->value(), Eq("Snake"));
    ASSERT_THAT(hm.find(3), Ne(hm.end())); EXPECT_THAT(hm.find(3)->value(), Eq("Wario"));

}
