#include "gmock/gmock.h"
#include "uh/String.hpp"

#define NAME string

using namespace testing;
using namespace uh;

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
