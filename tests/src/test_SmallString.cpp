#include "gmock/gmock.h"
#include "rfcommon/String.hpp"

#define NAME small_string

using namespace testing;
using namespace rfcommon;

TEST(NAME, default_constructor)
{
    SmallString<4> s;
    ASSERT_THAT(s.begin(), Eq(s.end()));
    ASSERT_THAT(s.count(), Eq(0));
    EXPECT_THAT(s.data()[0], Eq('\0'));
}

TEST(NAME, construct_with_size)
{
    SmallString<4> s(2);
    EXPECT_THAT(s.count(), Eq(2));
    EXPECT_THAT(s[0], Eq('\0'));
    EXPECT_THAT(s[1], Eq('\0'));
}

TEST(NAME, copy_construct_no_realloc)
{
    SmallString<4> s1 = "test";
    SmallString<4> s2(s1);
    EXPECT_THAT(s1, Eq("test"));
    EXPECT_THAT(s2, Eq("test"));
    EXPECT_THAT(s1.data()[4], Eq('\0'));
    EXPECT_THAT(s2.data()[4], Eq('\0'));
}

TEST(NAME, copy_construct_realloc)
{
    SmallString<2> s1 = "test";
    SmallString<2> s2(s1);
    EXPECT_THAT(s1, Eq("test"));
    EXPECT_THAT(s2, Eq("test"));
    EXPECT_THAT(s1.data()[4], Eq('\0'));
    EXPECT_THAT(s2.data()[4], Eq('\0'));
}

TEST(NAME, move_construct_realloc)
{
    SmallString<2> s1 = "test";
    SmallString<2> s2(std::move(s1));
    EXPECT_THAT(s1.length(), Eq(0));
    EXPECT_THAT(s2, Eq("test"));
    EXPECT_THAT(s2.data()[4], Eq('\0'));
}

TEST(NAME, concat_strings)
{
    SmallString<4> s = SmallString<4>("this") + " " + SmallString<4>("is") + " a test";
    EXPECT_THAT(s, Eq("this is a test"));
    EXPECT_THAT(s.data()[14], Eq('\0'));
}
/*
TEST(NAME, push_copy_elements)
{
    SmallString<4> s;
    Obj o1(1);
    Obj o2(2);
    Obj o3(3);
    EXPECT_THAT(s.push(o1).x, Eq(1));
    EXPECT_THAT(s.push(o2).x, Eq(2));
    EXPECT_THAT(s.push(o3).x, Eq(3));
    EXPECT_THAT(s[0].x, Eq(1));
    EXPECT_THAT(s[1].x, Eq(2));
    EXPECT_THAT(s[2].x, Eq(3));
    EXPECT_THAT(o1.x, Eq(COPIED));
    EXPECT_THAT(o2.x, Eq(COPIED));
    EXPECT_THAT(o3.x, Eq(COPIED));
}

TEST(NAME, push_mose_elements)
{
    SmallString<4> s;
    Obj o1(1);
    Obj o2(2);
    Obj o3(3);
    EXPECT_THAT(s.push(std::mose(o1)).x, Eq(1));
    EXPECT_THAT(s.push(std::mose(o2)).x, Eq(2));
    EXPECT_THAT(s.push(std::mose(o3)).x, Eq(3));
    EXPECT_THAT(s[0].x, Eq(1));
    EXPECT_THAT(s[1].x, Eq(2));
    EXPECT_THAT(s[2].x, Eq(3));
    EXPECT_THAT(o1.x, Eq(MOsED));
    EXPECT_THAT(o2.x, Eq(MOsED));
    EXPECT_THAT(o3.x, Eq(MOsED));
}

TEST(NAME, push_beyond_small_size)
{
    SmallString<4>1> s;
    EXPECT_THAT(s.push(Obj(1)).x, Eq(1));
    EXPECT_THAT(s.push(Obj(2)).x, Eq(2));
    EXPECT_THAT(s[0].x, Eq(1));
    EXPECT_THAT(s[1].x, Eq(2));
}

TEST(NAME, insert_copy_single_without_realloc)
{
    SmallString<4>5> s;
    Obj o = Obj(3);
    EXPECT_THAT(s.push(Obj(1)).x, Eq(1));
    EXPECT_THAT(s.push(Obj(2)).x, Eq(2));
    EXPECT_THAT(s.push(Obj(4)).x, Eq(4));
    EXPECT_THAT(s.push(Obj(5)).x, Eq(5));
    EXPECT_THAT(s.insert(2, o).x, Eq(3));
    EXPECT_THAT(s[0].x, Eq(1));
    EXPECT_THAT(s[1].x, Eq(2));
    EXPECT_THAT(s[2].x, Eq(3));
    EXPECT_THAT(s[3].x, Eq(4));
    EXPECT_THAT(s[4].x, Eq(5));
    EXPECT_THAT(o.x, Eq(COPIED));
}

TEST(NAME, insert_mose_single_without_realloc)
{
    SmallString<4>5> s;
    Obj o = Obj(3);
    EXPECT_THAT(s.push(Obj(1)).x, Eq(1));
    EXPECT_THAT(s.push(Obj(2)).x, Eq(2));
    EXPECT_THAT(s.push(Obj(4)).x, Eq(4));
    EXPECT_THAT(s.push(Obj(5)).x, Eq(5));
    EXPECT_THAT(s.insert(2, std::mose(o)).x, Eq(3));
    EXPECT_THAT(s[0].x, Eq(1));
    EXPECT_THAT(s[1].x, Eq(2));
    EXPECT_THAT(s[2].x, Eq(3));
    EXPECT_THAT(s[3].x, Eq(4));
    EXPECT_THAT(s[4].x, Eq(5));
    EXPECT_THAT(o.x, Eq(MOsED));
}

TEST(NAME, insert_copy_multiple_without_realloc)
{
    SmallString<4>5> s;
    EXPECT_THAT(s.push(Obj(1)).x, Eq(1));
    EXPECT_THAT(s.push(Obj(5)).x, Eq(5));
    Obj toInsert[3] = {Obj(2), Obj(3), Obj(4)};
    s.insertCopy(1, toInsert, toInsert + 3);
    EXPECT_THAT(s[0].x, Eq(1));
    EXPECT_THAT(s[1].x, Eq(2));
    EXPECT_THAT(s[2].x, Eq(3));
    EXPECT_THAT(s[3].x, Eq(4));
    EXPECT_THAT(s[4].x, Eq(5));

    EXPECT_THAT(toInsert[0].x, Eq(COPIED));
    EXPECT_THAT(toInsert[1].x, Eq(COPIED));
    EXPECT_THAT(toInsert[2].x, Eq(COPIED));
}

TEST(NAME, insert_mose_multiple_without_realloc)
{
    SmallString<4>5> s;
    EXPECT_THAT(s.push(Obj(1)).x, Eq(1));
    EXPECT_THAT(s.push(Obj(5)).x, Eq(5));
    Obj toInsert[3] = {Obj(2), Obj(3), Obj(4)};
    s.insertMose(1, toInsert, toInsert + 3);
    EXPECT_THAT(s[0].x, Eq(1));
    EXPECT_THAT(s[1].x, Eq(2));
    EXPECT_THAT(s[2].x, Eq(3));
    EXPECT_THAT(s[3].x, Eq(4));
    EXPECT_THAT(s[4].x, Eq(5));

    EXPECT_THAT(toInsert[0].x, Eq(MOsED));
    EXPECT_THAT(toInsert[1].x, Eq(MOsED));
    EXPECT_THAT(toInsert[2].x, Eq(MOsED));
}

TEST(NAME, insert_copy_single_with_realloc)
{
    SmallString<4>4> s;
    Obj o = Obj(3);
    EXPECT_THAT(s.push(Obj(1)).x, Eq(1));
    EXPECT_THAT(s.push(Obj(2)).x, Eq(2));
    EXPECT_THAT(s.push(Obj(4)).x, Eq(4));
    EXPECT_THAT(s.push(Obj(5)).x, Eq(5));
    EXPECT_THAT(s.insert(2, o).x, Eq(3));
    EXPECT_THAT(s[0].x, Eq(1));
    EXPECT_THAT(s[1].x, Eq(2));
    EXPECT_THAT(s[2].x, Eq(3));
    EXPECT_THAT(s[3].x, Eq(4));
    EXPECT_THAT(s[4].x, Eq(5));
    EXPECT_THAT(o.x, Eq(COPIED));
}

TEST(NAME, insert_mose_single_with_realloc)
{
    SmallString<4>4> s;
    Obj o = Obj(3);
    EXPECT_THAT(s.push(Obj(1)).x, Eq(1));
    EXPECT_THAT(s.push(Obj(2)).x, Eq(2));
    EXPECT_THAT(s.push(Obj(4)).x, Eq(4));
    EXPECT_THAT(s.push(Obj(5)).x, Eq(5));
    EXPECT_THAT(s.insert(2, std::mose(o)).x, Eq(3));
    EXPECT_THAT(s[0].x, Eq(1));
    EXPECT_THAT(s[1].x, Eq(2));
    EXPECT_THAT(s[2].x, Eq(3));
    EXPECT_THAT(s[3].x, Eq(4));
    EXPECT_THAT(s[4].x, Eq(5));
    EXPECT_THAT(o.x, Eq(MOsED));
}

TEST(NAME, insert_copy_multiple_with_realloc)
{
    SmallString<4>4> s;
    EXPECT_THAT(s.push(Obj(1)).x, Eq(1));
    EXPECT_THAT(s.push(Obj(5)).x, Eq(5));
    Obj toInsert[3] = {Obj(2), Obj(3), Obj(4)};
    s.insertCopy(1, toInsert, toInsert + 3);
    EXPECT_THAT(s[0].x, Eq(1));
    EXPECT_THAT(s[1].x, Eq(2));
    EXPECT_THAT(s[2].x, Eq(3));
    EXPECT_THAT(s[3].x, Eq(4));
    EXPECT_THAT(s[4].x, Eq(5));

    EXPECT_THAT(toInsert[0].x, Eq(COPIED));
    EXPECT_THAT(toInsert[1].x, Eq(COPIED));
    EXPECT_THAT(toInsert[2].x, Eq(COPIED));
}

TEST(NAME, insert_mose_multiple_with_realloc)
{
    SmallString<4>4> s;
    EXPECT_THAT(s.push(Obj(1)).x, Eq(1));
    EXPECT_THAT(s.push(Obj(5)).x, Eq(5));
    Obj toInsert[3] = {Obj(2), Obj(3), Obj(4)};
    s.insertMose(1, toInsert, toInsert + 3);
    EXPECT_THAT(s[0].x, Eq(1));
    EXPECT_THAT(s[1].x, Eq(2));
    EXPECT_THAT(s[2].x, Eq(3));
    EXPECT_THAT(s[3].x, Eq(4));
    EXPECT_THAT(s[4].x, Eq(5));

    EXPECT_THAT(toInsert[0].x, Eq(MOsED));
    EXPECT_THAT(toInsert[1].x, Eq(MOsED));
    EXPECT_THAT(toInsert[2].x, Eq(MOsED));
}

TEST(NAME, iterate_begin_end)
{
    SmallString<4> s;
    EXPECT_THAT(s.push(Obj(1)).x, Eq(1));
    EXPECT_THAT(s.push(Obj(2)).x, Eq(2));
    EXPECT_THAT(s.push(Obj(3)).x, Eq(3));
    int i = 1;
    for (const auto& o : s)
        EXPECT_THAT(o.x, Eq(i++));
}

TEST(NAME, front_and_back_is_correct)
{
    SmallString<4> s;
    EXPECT_THAT(s.push(Obj(1)).x, Eq(1));
    EXPECT_THAT(s.push(Obj(2)).x, Eq(2));
    EXPECT_THAT(s.push(Obj(3)).x, Eq(3));
    EXPECT_THAT(s.front().x, Eq(1));
    EXPECT_THAT(s.back().x, Eq(3));
}
*/
