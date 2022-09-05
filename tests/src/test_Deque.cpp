#include "rfcommon/Deque.hpp"
#include "gmock/gmock.h"

#define NAME deque

namespace {

template <typename T>
struct Entry
{
    Entry() {}
    Entry(const T& data) : data(data) {}

    Entry* next = (Entry*)0x6969;
    Entry* prev = (Entry*)0x420;
    T data;
};

}

using namespace rfcommon;
using namespace testing;

TEST(NAME, front_and_back_are_null_in_empty_deque)
{
    Deque<Entry<int>> d;
    ASSERT_THAT(d.count(), Eq(0));
    ASSERT_THAT(d.peekFront(), IsNull());
    ASSERT_THAT(d.peekback(), IsNull());
}

TEST(NAME, put_front_first_entry_correctly_sets_members)
{
    Entry<int> e1;
    Deque<Entry<int>> d;
    d.putFront(&e1);
    ASSERT_THAT(d.count(), Eq(1));
    ASSERT_THAT(d.peekFront(), Eq(&e1));
    ASSERT_THAT(d.peekback(), Eq(&e1));
    ASSERT_THAT(e1.prev, IsNull());
    ASSERT_THAT(e1.next, IsNull());
}

TEST(NAME, put_front_first_and_second_entry_correctly_sets_members)
{
    Entry<int> e1, e2;
    Deque<Entry<int>> d;
    d.putFront(&e1);
    d.putFront(&e2);
    ASSERT_THAT(d.count(), Eq(2));
    ASSERT_THAT(d.peekFront(), Eq(&e2));
    ASSERT_THAT(d.peekback(), Eq(&e1));
    ASSERT_THAT(e1.prev, IsNull());
    ASSERT_THAT(e1.next, Eq(&e2));
    ASSERT_THAT(e2.prev, Eq(&e1));
    ASSERT_THAT(e2.next, IsNull());
}

TEST(NAME, put_back_first_entry_correctly_sets_members)
{
    Entry<int> e1;
    Deque<Entry<int>> d;
    d.putBack(&e1);
    ASSERT_THAT(d.count(), Eq(1));
    ASSERT_THAT(d.peekFront(), Eq(&e1));
    ASSERT_THAT(d.peekback(), Eq(&e1));
    ASSERT_THAT(e1.prev, IsNull());
    ASSERT_THAT(e1.next, IsNull());
}

TEST(NAME, put_back_first_and_second_entry_correctly_sets_members)
{
    Entry<int> e1, e2;
    Deque<Entry<int>> d;
    d.putBack(&e1);
    d.putBack(&e2);
    ASSERT_THAT(d.count(), Eq(2));
    ASSERT_THAT(d.peekFront(), Eq(&e1));
    ASSERT_THAT(d.peekback(), Eq(&e2));
    ASSERT_THAT(e1.prev, Eq(&e2));
    ASSERT_THAT(e1.next, IsNull());
    ASSERT_THAT(e2.prev, IsNull());
    ASSERT_THAT(e2.next, Eq(&e1));
}

TEST(NAME, take_front_last_entry_correctly_resets_members)
{
    Entry<int> e1;
    Deque<Entry<int>> d;
    d.putFront(&e1);
    ASSERT_THAT(d.takeFront(), Eq(&e1));
    ASSERT_THAT(d.count(), Eq(0));
    ASSERT_THAT(d.peekFront(), IsNull());
    ASSERT_THAT(d.peekback(), IsNull());
}

TEST(NAME, take_front_last_two_entries_correctly_resets_members)
{
    Entry<int> e1, e2;
    Deque<Entry<int>> d;
    d.putFront(&e1);
    d.putFront(&e2);
    ASSERT_THAT(d.takeFront(), Eq(&e2));
    ASSERT_THAT(d.takeFront(), Eq(&e1));
    ASSERT_THAT(d.count(), Eq(0));
    ASSERT_THAT(d.peekFront(), IsNull());
    ASSERT_THAT(d.peekback(), IsNull());
}

TEST(NAME, take_front_second_last_entry_correctly_sets_members)
{
    Entry<int> e1, e2;
    Deque<Entry<int>> d;
    d.putFront(&e1);
    d.putFront(&e2);
    ASSERT_THAT(d.takeFront(), Eq(&e2));
    ASSERT_THAT(d.count(), Eq(1));
    ASSERT_THAT(d.peekFront(), Eq(&e1));
    ASSERT_THAT(d.peekback(), Eq(&e1));
    ASSERT_THAT(e1.prev, IsNull());
    ASSERT_THAT(e1.next, IsNull());
}

TEST(NAME, take_back_last_entry_correctly_resets_members)
{
    Entry<int> e1;
    Deque<Entry<int>> d;
    d.putBack(&e1);
    ASSERT_THAT(d.takeBack(), Eq(&e1));
    ASSERT_THAT(d.count(), Eq(0));
    ASSERT_THAT(d.peekFront(), IsNull());
    ASSERT_THAT(d.peekback(), IsNull());
}

TEST(NAME, take_back_last_two_entries_correctly_resets_members)
{
    Entry<int> e1, e2;
    Deque<Entry<int>> d;
    d.putBack(&e1);
    d.putBack(&e2);
    ASSERT_THAT(d.takeBack(), Eq(&e2));
    ASSERT_THAT(d.takeBack(), Eq(&e1));
    ASSERT_THAT(d.count(), Eq(0));
    ASSERT_THAT(d.peekFront(), IsNull());
    ASSERT_THAT(d.peekback(), IsNull());
}

TEST(NAME, take_back_second_last_entry_correctly_sets_members)
{
    Entry<int> e1, e2;
    Deque<Entry<int>> d;
    d.putBack(&e1);
    d.putBack(&e2);
    ASSERT_THAT(d.takeBack(), Eq(&e2));
    ASSERT_THAT(d.count(), Eq(1));
    ASSERT_THAT(d.peekFront(), Eq(&e1));
    ASSERT_THAT(d.peekback(), Eq(&e1));
    ASSERT_THAT(e1.prev, IsNull());
    ASSERT_THAT(e1.next, IsNull());
}

TEST(NAME, iterate)
{
    Entry<int> e1(2), e2(3);
    Deque<Entry<int>> d;
    d.putFront(&e1);
    d.putFront(&e2);
    auto it = d.begin();
    ASSERT_THAT(it->data, Eq(2));
    it++;
    ASSERT_THAT(it->data, Eq(3));
    it++;
    ASSERT_THAT(it, Eq(d.end()));
}
