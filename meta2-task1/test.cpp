#include <iostream>
#include <gtest/gtest.h>
#include "typelist.hpp"

class A {};
class B {};
class C {};

TEST(TypeList, Length) {
    EXPECT_EQ(Length<EmptyList>::value, 0);

    auto value = Length<TypeList<A, B, C>>::value;
    EXPECT_EQ(value, 3);
}

TEST(TypeList, TypeAt) {
    using List1 = TypeList<A, B, C>;
    EXPECT_STREQ(typeid(TypeAt<List1, 0>::Result).name(), typeid(A).name());
    EXPECT_STREQ(typeid(TypeAt<List1, 1>::Result).name(), typeid(B).name());
    EXPECT_STREQ(typeid(TypeAt<List1, 2>::Result).name(), typeid(C).name());

    using List2 = TypeList<A, TypeList< B, TypeList<C> >>;
    EXPECT_STREQ(typeid(TypeAt<List2, 0>::Result).name(), typeid(A).name());
    EXPECT_STREQ(typeid(TypeAt<List2, 1>::Result).name(), typeid(B).name());
    EXPECT_STREQ(typeid(TypeAt<List2, 2>::Result).name(), typeid(C).name());
}

TEST(TypeList, Append) {
    using List1 = TypeList<A, B>;
    using List2 = Append<List1, C>::Result;
    EXPECT_STREQ(typeid(TypeAt<List2, 0>::Result).name(), typeid(A).name());
    EXPECT_STREQ(typeid(TypeAt<List2, 1>::Result).name(), typeid(B).name());
    EXPECT_STREQ(typeid(TypeAt<List2, 2>::Result).name(), typeid(C).name());
    EXPECT_EQ(Length<List2>::value, 3);

    using List3 = TypeList<>;
    using List4 = Append<List3, A>::Result;
    EXPECT_STREQ(typeid(TypeAt<List4, 0>::Result).name(), typeid(A).name());
    EXPECT_EQ(Length<List4>::value, 1);

    using List5 = Append<List2, List4>::Result;
    EXPECT_STREQ(typeid(TypeAt<List5, 0>::Result).name(), typeid(A).name());
    EXPECT_STREQ(typeid(TypeAt<List5, 1>::Result).name(), typeid(B).name());
    EXPECT_STREQ(typeid(TypeAt<List5, 2>::Result).name(), typeid(C).name());
    EXPECT_STREQ(typeid(TypeAt<List5, 3>::Result).name(), typeid(A).name());
    EXPECT_EQ(Length<List5>::value, 4);
}

TEST(TypeList, Erase) {
    using List1 = TypeList<A, B, C>;

    using List2 = Erase<List1, B>::Result;
    EXPECT_STREQ(typeid(TypeAt<List2, 0>::Result).name(), typeid(A).name());
    EXPECT_STREQ(typeid(TypeAt<List2, 1>::Result).name(), typeid(C).name());
    EXPECT_EQ(Length<List2>::value, 2);

    using List3 = Erase<List1, C>::Result;
    EXPECT_STREQ(typeid(TypeAt<List3, 0>::Result).name(), typeid(A).name());
    EXPECT_STREQ(typeid(TypeAt<List3, 1>::Result).name(), typeid(B).name());
    EXPECT_EQ(Length<List3>::value, 2);

    using List4 = Erase<List1, A>::Result;
    EXPECT_STREQ(typeid(TypeAt<List4, 0>::Result).name(), typeid(B).name());
    EXPECT_STREQ(typeid(TypeAt<List4, 1>::Result).name(), typeid(C).name());
    EXPECT_EQ(Length<List4>::value, 2);
}
