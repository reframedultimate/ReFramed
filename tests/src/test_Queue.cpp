#include "rfcommon/Queue.hpp"
#include "gmock/gmock.h"

#define NAME queue

namespace {

template <typename T>
struct Entry
{
    Entry() {}
    Entry(const T& d) : data(d) {}

    Entry* next = (Entry*)0x6969;
    T data;
};

}

using namespace rfcommon;
using namespace testing;

TEST(NAME, front_and_back_are_null_in_empty_deque)
{
    Queue<Entry<int>> d;
    ASSERT_THAT(d.count(), Eq(0));
    ASSERT_THAT(d.peekFront(), IsNull());
    ASSERT_THAT(d.peekback(), IsNull());
}

TEST(NAME, put_front_first_entry_correctly_sets_members)
{
    Entry<int> e1;
    Queue<Entry<int>> d;
    d.putFront(&e1);
    ASSERT_THAT(d.count(), Eq(1));
    ASSERT_THAT(d.peekFront(), Eq(&e1));
    ASSERT_THAT(d.peekback(), Eq(&e1));
    ASSERT_THAT(e1.next, IsNull());
}

TEST(NAME, put_front_first_and_second_entry_correctly_sets_members)
{
    Entry<int> e1, e2;
    Queue<Entry<int>> d;
    d.putFront(&e1);
    d.putFront(&e2);
    ASSERT_THAT(d.count(), Eq(2));
    ASSERT_THAT(d.peekFront(), Eq(&e2));
    ASSERT_THAT(d.peekback(), Eq(&e1));
    ASSERT_THAT(e1.next, Eq(&e2));
    ASSERT_THAT(e2.next, IsNull());
}

TEST(NAME, take_back_last_entry_correctly_resets_members)
{
    Entry<int> e1;
    Queue<Entry<int>> d;
    d.putFront(&e1);
    ASSERT_THAT(d.takeBack(), Eq(&e1));
    ASSERT_THAT(d.count(), Eq(0));
    ASSERT_THAT(d.peekFront(), IsNull());
    ASSERT_THAT(d.peekback(), IsNull());
}

TEST(NAME, take_back_last_two_entries_correctly_resets_members)
{
    Entry<int> e1, e2;
    Queue<Entry<int>> d;
    d.putFront(&e1);
    d.putFront(&e2);
    ASSERT_THAT(d.takeBack(), Eq(&e1));
    ASSERT_THAT(d.takeBack(), Eq(&e2));
    ASSERT_THAT(d.count(), Eq(0));
    ASSERT_THAT(d.peekFront(), IsNull());
    ASSERT_THAT(d.peekback(), IsNull());
}

TEST(NAME, take_back_second_last_entry_correctly_sets_members)
{
    Entry<int> e1, e2;
    Queue<Entry<int>> d;
    d.putFront(&e1);
    d.putFront(&e2);
    ASSERT_THAT(d.takeBack(), Eq(&e1));
    ASSERT_THAT(d.count(), Eq(1));
    ASSERT_THAT(d.peekFront(), Eq(&e2));
    ASSERT_THAT(d.peekback(), Eq(&e2));
    ASSERT_THAT(e2.next, IsNull());
}

TEST(NAME, iterate)
{
    Entry<int> e1(2);
    Entry<int> e2(3);
    Queue<Entry<int>> d;
    d.putFront(&e1);
    d.putFront(&e2);
    auto it = d.begin();
    ASSERT_THAT(it->data, Eq(2));
    it++;
    ASSERT_THAT(it->data, Eq(3));
    it++;
    ASSERT_THAT(it, Eq(d.end()));
}
