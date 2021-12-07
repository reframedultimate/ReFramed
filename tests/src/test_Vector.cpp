#include "gmock/gmock.h"
#include "uh/Vector.hpp"

#define NAME vector

using namespace testing;
using namespace uh;

namespace {
#define CONSTRUCTED 5555
#define COPIED      6666
#define MOVED       7777
#define DESTRUCTED  8888
struct Obj
{
    Obj() : x(CONSTRUCTED) {}
    Obj(int x) : x(x) {}
    Obj(const Obj& other) : x(other.x) { *const_cast<int*>(&other.x) = COPIED; }
    Obj(Obj&& other) : x(other.x) { other.x = MOVED; }
    ~Obj() { x = DESTRUCTED; }

    int x;
};
}

TEST(NAME, default_constructor)
{
    Vector<Obj> v;
    ASSERT_THAT(v.begin(), Eq(v.end()));
    ASSERT_THAT(v.count(), Eq(0));
}

TEST(NAME, construct_with_size)
{
    Vector<Obj> v(2);
    EXPECT_THAT(v.count(), Eq(2));
    EXPECT_THAT(v[0].x, Eq(CONSTRUCTED));
    EXPECT_THAT(v[1].x, Eq(CONSTRUCTED));
}

TEST(NAME, copy_construct)
{
    Vector<Obj> v1;
    EXPECT_THAT(v1.push(Obj(1))->x, Eq(1));
    EXPECT_THAT(v1.push(Obj(2))->x, Eq(2));
    EXPECT_THAT(v1.push(Obj(3))->x, Eq(3));

    Vector<Obj> v2(v1);
    EXPECT_THAT(v1[0].x, Eq(COPIED));
    EXPECT_THAT(v1[1].x, Eq(COPIED));
    EXPECT_THAT(v1[2].x, Eq(COPIED));
    EXPECT_THAT(v2[0].x, Eq(1));
    EXPECT_THAT(v2[1].x, Eq(2));
    EXPECT_THAT(v2[2].x, Eq(3));
}

TEST(NAME, move_construct)
{
    Vector<Obj> v1;
    EXPECT_THAT(v1.push(Obj(1))->x, Eq(1));
    EXPECT_THAT(v1.push(Obj(2))->x, Eq(2));
    EXPECT_THAT(v1.push(Obj(3))->x, Eq(3));

    Vector<Obj> v2(std::move(v1));
    EXPECT_THAT(v1.count(), Eq(0));
    EXPECT_THAT(v2[0].x, Eq(1));
    EXPECT_THAT(v2[1].x, Eq(2));
    EXPECT_THAT(v2[2].x, Eq(3));
}

TEST(NAME, push_copy_elements)
{
    Vector<Obj> v;
    Obj o1(1);
    Obj o2(2);
    Obj o3(3);
    EXPECT_THAT(v.push(o1)->x, Eq(1));
    EXPECT_THAT(v.push(o2)->x, Eq(2));
    EXPECT_THAT(v.push(o3)->x, Eq(3));
    EXPECT_THAT(v[0].x, Eq(1));
    EXPECT_THAT(v[1].x, Eq(2));
    EXPECT_THAT(v[2].x, Eq(3));
    EXPECT_THAT(o1.x, Eq(COPIED));
    EXPECT_THAT(o2.x, Eq(COPIED));
    EXPECT_THAT(o3.x, Eq(COPIED));
}

TEST(NAME, push_move_elements)
{
    Vector<Obj> v;
    Obj o1(1);
    Obj o2(2);
    Obj o3(3);
    EXPECT_THAT(v.push(std::move(o1))->x, Eq(1));
    EXPECT_THAT(v.push(std::move(o2))->x, Eq(2));
    EXPECT_THAT(v.push(std::move(o3))->x, Eq(3));
    EXPECT_THAT(v[0].x, Eq(1));
    EXPECT_THAT(v[1].x, Eq(2));
    EXPECT_THAT(v[2].x, Eq(3));
    EXPECT_THAT(o1.x, Eq(MOVED));
    EXPECT_THAT(o2.x, Eq(MOVED));
    EXPECT_THAT(o3.x, Eq(MOVED));
}

TEST(NAME, insert_copy_single_without_realloc)
{
    Vector<Obj> v;
    v.reserve(5);
    Obj o = Obj(3);
    EXPECT_THAT(v.push(Obj(1))->x, Eq(1));
    EXPECT_THAT(v.push(Obj(2))->x, Eq(2));
    EXPECT_THAT(v.push(Obj(4))->x, Eq(4));
    EXPECT_THAT(v.push(Obj(5))->x, Eq(5));
    EXPECT_THAT(v.insert(2, o)->x, Eq(3));
    EXPECT_THAT(v[0].x, Eq(1));
    EXPECT_THAT(v[1].x, Eq(2));
    EXPECT_THAT(v[2].x, Eq(3));
    EXPECT_THAT(v[3].x, Eq(4));
    EXPECT_THAT(v[4].x, Eq(5));
    EXPECT_THAT(o.x, Eq(COPIED));
}

TEST(NAME, insert_move_single_without_realloc)
{
    Vector<Obj> v;
    v.reserve(5);
    Obj o = Obj(3);
    EXPECT_THAT(v.push(Obj(1))->x, Eq(1));
    EXPECT_THAT(v.push(Obj(2))->x, Eq(2));
    EXPECT_THAT(v.push(Obj(4))->x, Eq(4));
    EXPECT_THAT(v.push(Obj(5))->x, Eq(5));
    EXPECT_THAT(v.insert(2, std::move(o))->x, Eq(3));
    EXPECT_THAT(v[0].x, Eq(1));
    EXPECT_THAT(v[1].x, Eq(2));
    EXPECT_THAT(v[2].x, Eq(3));
    EXPECT_THAT(v[3].x, Eq(4));
    EXPECT_THAT(v[4].x, Eq(5));
    EXPECT_THAT(o.x, Eq(MOVED));
}

TEST(NAME, insert_copy_multiple_without_realloc)
{
    Vector<Obj> v;
    v.reserve(5);
    EXPECT_THAT(v.push(Obj(1))->x, Eq(1));
    EXPECT_THAT(v.push(Obj(5))->x, Eq(5));
    Obj toInsert[3] = {Obj(2), Obj(3), Obj(4)};
    v.insertCopy(1, toInsert, toInsert + 3);
    EXPECT_THAT(v[0].x, Eq(1));
    EXPECT_THAT(v[1].x, Eq(2));
    EXPECT_THAT(v[2].x, Eq(3));
    EXPECT_THAT(v[3].x, Eq(4));
    EXPECT_THAT(v[4].x, Eq(5));

    EXPECT_THAT(toInsert[0].x, Eq(COPIED));
    EXPECT_THAT(toInsert[1].x, Eq(COPIED));
    EXPECT_THAT(toInsert[2].x, Eq(COPIED));
}

TEST(NAME, insert_move_multiple_without_realloc)
{
    Vector<Obj> v;
    v.reserve(5);
    EXPECT_THAT(v.push(Obj(1))->x, Eq(1));
    EXPECT_THAT(v.push(Obj(5))->x, Eq(5));
    Obj toInsert[3] = {Obj(2), Obj(3), Obj(4)};
    v.insertMove(1, toInsert, toInsert + 3);
    EXPECT_THAT(v[0].x, Eq(1));
    EXPECT_THAT(v[1].x, Eq(2));
    EXPECT_THAT(v[2].x, Eq(3));
    EXPECT_THAT(v[3].x, Eq(4));
    EXPECT_THAT(v[4].x, Eq(5));

    EXPECT_THAT(toInsert[0].x, Eq(MOVED));
    EXPECT_THAT(toInsert[1].x, Eq(MOVED));
    EXPECT_THAT(toInsert[2].x, Eq(MOVED));
}

TEST(NAME, insert_copy_single_with_realloc)
{
    Vector<Obj> v;
    v.reserve(4);
    Obj o = Obj(3);
    EXPECT_THAT(v.push(Obj(1))->x, Eq(1));
    EXPECT_THAT(v.push(Obj(2))->x, Eq(2));
    EXPECT_THAT(v.push(Obj(4))->x, Eq(4));
    EXPECT_THAT(v.push(Obj(5))->x, Eq(5));
    EXPECT_THAT(v.insert(2, o)->x, Eq(3));
    EXPECT_THAT(v[0].x, Eq(1));
    EXPECT_THAT(v[1].x, Eq(2));
    EXPECT_THAT(v[2].x, Eq(3));
    EXPECT_THAT(v[3].x, Eq(4));
    EXPECT_THAT(v[4].x, Eq(5));
    EXPECT_THAT(o.x, Eq(COPIED));
}

TEST(NAME, insert_move_single_with_realloc)
{
    Vector<Obj> v;
    v.reserve(4);
    Obj o = Obj(3);
    EXPECT_THAT(v.push(Obj(1))->x, Eq(1));
    EXPECT_THAT(v.push(Obj(2))->x, Eq(2));
    EXPECT_THAT(v.push(Obj(4))->x, Eq(4));
    EXPECT_THAT(v.push(Obj(5))->x, Eq(5));
    EXPECT_THAT(v.insert(2, std::move(o))->x, Eq(3));
    EXPECT_THAT(v[0].x, Eq(1));
    EXPECT_THAT(v[1].x, Eq(2));
    EXPECT_THAT(v[2].x, Eq(3));
    EXPECT_THAT(v[3].x, Eq(4));
    EXPECT_THAT(v[4].x, Eq(5));
    EXPECT_THAT(o.x, Eq(MOVED));
}

TEST(NAME, insert_copy_multiple_with_realloc)
{
    Vector<Obj> v;
    v.reserve(4);
    EXPECT_THAT(v.push(Obj(1))->x, Eq(1));
    EXPECT_THAT(v.push(Obj(5))->x, Eq(5));
    Obj toInsert[3] = {Obj(2), Obj(3), Obj(4)};
    v.insertCopy(1, toInsert, toInsert + 3);
    EXPECT_THAT(v[0].x, Eq(1));
    EXPECT_THAT(v[1].x, Eq(2));
    EXPECT_THAT(v[2].x, Eq(3));
    EXPECT_THAT(v[3].x, Eq(4));
    EXPECT_THAT(v[4].x, Eq(5));

    EXPECT_THAT(toInsert[0].x, Eq(COPIED));
    EXPECT_THAT(toInsert[1].x, Eq(COPIED));
    EXPECT_THAT(toInsert[2].x, Eq(COPIED));
}

TEST(NAME, insert_move_multiple_with_realloc)
{
    Vector<Obj> v;
    v.reserve(4);
    EXPECT_THAT(v.push(Obj(1))->x, Eq(1));
    EXPECT_THAT(v.push(Obj(5))->x, Eq(5));
    Obj toInsert[3] = {Obj(2), Obj(3), Obj(4)};
    v.insertMove(1, toInsert, toInsert + 3);
    EXPECT_THAT(v[0].x, Eq(1));
    EXPECT_THAT(v[1].x, Eq(2));
    EXPECT_THAT(v[2].x, Eq(3));
    EXPECT_THAT(v[3].x, Eq(4));
    EXPECT_THAT(v[4].x, Eq(5));

    EXPECT_THAT(toInsert[0].x, Eq(MOVED));
    EXPECT_THAT(toInsert[1].x, Eq(MOVED));
    EXPECT_THAT(toInsert[2].x, Eq(MOVED));
}

TEST(NAME, iterate_begin_end)
{
    Vector<Obj> v;
    EXPECT_THAT(v.push(Obj(1))->x, Eq(1));
    EXPECT_THAT(v.push(Obj(2))->x, Eq(2));
    EXPECT_THAT(v.push(Obj(3))->x, Eq(3));
    int i = 1;
    for (const auto& o : v)
        EXPECT_THAT(o.x, Eq(i++));
}

TEST(NAME, front_and_back_is_correct)
{
    Vector<Obj> v;
    EXPECT_THAT(v.push(Obj(1))->x, Eq(1));
    EXPECT_THAT(v.push(Obj(2))->x, Eq(2));
    EXPECT_THAT(v.push(Obj(3))->x, Eq(3));
    EXPECT_THAT(v.front().x, Eq(1));
    EXPECT_THAT(v.back().x, Eq(3));
}
