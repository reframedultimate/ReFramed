#include "gmock/gmock.h"
#include "rfcommon/Reference.hpp"
#include "rfcommon/RefCounted.hpp"

#define NAME Reference

using namespace testing;

namespace {
class Obj : public rfcommon::RefCounted
{
public:
};
}

TEST(NAME, default_refcounted_constructor_has_refcount_0)
{
    Obj* o = new Obj;
    EXPECT_THAT(o->refs(), Eq(0));
    delete o;
}

TEST(NAME, default_reference_constructor_has_refcount_0)
{
    rfcommon::Reference<Obj> ref;
    ASSERT_THAT(ref.refs(), Eq(0));
}

TEST(NAME, construct_from_raw_pointer)
{
    rfcommon::Reference<Obj> ref(new Obj);
    ASSERT_THAT(ref.refs(), Eq(1));

    Obj* o = ref.get();
    o->incRef();
    ASSERT_THAT(o->refs(), Eq(2));
    ref.drop();
    ASSERT_THAT(o->refs(), Eq(1));
    o->decRef();
}

TEST(NAME, copy_construct)
{
    Obj* o = new Obj;
    o->incRef();

    rfcommon::Reference<Obj> ref1(o);
    rfcommon::Reference<Obj> ref2(ref1);

    ASSERT_THAT(ref1.refs(), Eq(3));
    ASSERT_THAT(ref2.refs(), Eq(3));
    ASSERT_THAT(o->refs(), Eq(3));

    ref1.drop();
    ref2.drop();
    ASSERT_THAT(o->refs(), Eq(1));
    o->decRef();
}

TEST(NAME, assign_raw_pointer)
{
    Obj* o1 = new Obj;
    Obj* o2 = new Obj;
    o1->incRef();
    o2->incRef();

    rfcommon::Reference<Obj> ref = o1;
    ASSERT_THAT(o1->refs(), Eq(2));
    ASSERT_THAT(o2->refs(), Eq(1));
    ref = o2;
    ASSERT_THAT(o1->refs(), Eq(1));
    ASSERT_THAT(o2->refs(), Eq(2));

    ref.drop();
    ASSERT_THAT(o1->refs(), Eq(1));

    o1->decRef();
    o2->decRef();
}

TEST(NAME, assign_operator)
{
    Obj* o1 = new Obj;
    Obj* o2 = new Obj;
    o1->incRef();
    o2->incRef();

    rfcommon::Reference<Obj> ref1 = o1;
    rfcommon::Reference<Obj> ref2 = o2;
    ASSERT_THAT(o1->refs(), Eq(2));
    ASSERT_THAT(o2->refs(), Eq(2));

    ref1 = ref2;
    ASSERT_THAT(o1->refs(), Eq(1));
    ASSERT_THAT(o2->refs(), Eq(3));

    ref1.drop();
    ref2.drop();
    ASSERT_THAT(o1->refs(), Eq(1));
    ASSERT_THAT(o2->refs(), Eq(1));

    o1->decRef();
    o2->decRef();
}

TEST(NAME, move_construct)
{
    Obj* o1 = new Obj;
    Obj* o2 = new Obj;
    o1->incRef();
    o2->incRef();

    rfcommon::Reference<Obj> ref2 = o2;
    ASSERT_THAT(o1->refs(), Eq(1));
    ASSERT_THAT(o2->refs(), Eq(2));

    rfcommon::Reference<Obj> ref1(std::move(ref2));
    ASSERT_THAT(o1->refs(), Eq(1));
    ASSERT_THAT(o2->refs(), Eq(2));

    ref1.drop();
    ASSERT_THAT(o1->refs(), Eq(1));
    ASSERT_THAT(o2->refs(), Eq(1));

    o1->decRef();
    o2->decRef();
}

TEST(NAME, move_assign)
{
    Obj* o1 = new Obj;
    Obj* o2 = new Obj;
    o1->incRef();
    o2->incRef();

    rfcommon::Reference<Obj> ref1 = o1;
    rfcommon::Reference<Obj> ref2 = o2;
    ASSERT_THAT(o1->refs(), Eq(2));
    ASSERT_THAT(o2->refs(), Eq(2));

    ref1 = std::move(ref2);
    ASSERT_THAT(o1->refs(), Eq(1));
    ASSERT_THAT(o2->refs(), Eq(2));

    ref1.drop();
    ASSERT_THAT(o1->refs(), Eq(1));
    ASSERT_THAT(o2->refs(), Eq(1));

    o1->decRef();
    o2->decRef();
}
