#include "gmock/gmock.h"
#include "rfcommon/String.hpp"

#define NAME string

using namespace testing;
using namespace rfcommon;

TEST(NAME, default_constructor)
{
    String s;
    ASSERT_THAT(s.begin(), Eq(s.end()));
    ASSERT_THAT(s.count(), Eq(0));
    EXPECT_THAT(s.data()[0], Eq('\0'));
}

TEST(NAME, construct_with_size)
{
    String s(2);
    EXPECT_THAT(s.count(), Eq(2));
    EXPECT_THAT(s[0], Eq('\0'));
    EXPECT_THAT(s[1], Eq('\0'));
}

TEST(NAME, copy_construct_no_realloc)
{
    String s1 = "test";
    String s2(s1);
    EXPECT_THAT(s1, Eq("test"));
    EXPECT_THAT(s2, Eq("test"));
    EXPECT_THAT(s1.data()[4], Eq('\0'));
    EXPECT_THAT(s2.data()[4], Eq('\0'));
}

TEST(NAME, copy_construct_realloc)
{
    String s1 = "test";
    String s2(s1);
    EXPECT_THAT(s1, Eq("test"));
    EXPECT_THAT(s2, Eq("test"));
    EXPECT_THAT(s1.data()[4], Eq('\0'));
    EXPECT_THAT(s2.data()[4], Eq('\0'));
}

TEST(NAME, move_construct_realloc)
{
    String s1 = "test";
    String s2(std::move(s1));
    EXPECT_THAT(s1.length(), Eq(0));
    EXPECT_THAT(s2, Eq("test"));
    EXPECT_THAT(s2.data()[4], Eq('\0'));
}

TEST(NAME, concat_strings)
{
    String s = String("this") + " " + String("is") + " a test";
    EXPECT_THAT(s, Eq("this is a test"));
    EXPECT_THAT(s.data()[14], Eq('\0'));
}

TEST(NAME, move_construct_stringvec)
{
    SmallVector<SmallString<15>, 2> sv1;
    sv1.push("Player 1");
    sv1.push("Player 2");

    ASSERT_THAT(sv1[0].length(), Eq(8));
    ASSERT_THAT(sv1[1].length(), Eq(8));
    ASSERT_THAT(sv1[0].cStr(), StrEq("Player 1"));
    ASSERT_THAT(sv1[1].cStr(), StrEq("Player 2"));

    SmallVector<SmallString<15>, 2> sv2(std::move(sv1));

    ASSERT_THAT(sv2[0].length(), Eq(8));
    ASSERT_THAT(sv2[1].length(), Eq(8));
    ASSERT_THAT(sv2[0].cStr(), StrEq("Player 1"));
    ASSERT_THAT(sv2[1].cStr(), StrEq("Player 2"));
}
