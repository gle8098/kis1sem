#pragma once

#include "typelist.hpp"


template<class TList, template<class> class Unit>
class GenScatterHierarchy;

template<class T1, class T2, class... T3, template<class> class Unit>
class GenScatterHierarchy<TypeList<T1, T2, T3...>, Unit> : public Unit<T1>,
                                                           public GenScatterHierarchy<TypeList<T2, T3...>, Unit> {
public:
    using TList = TypeList<T1, T2, T3...>;
    using LeftBase = GenScatterHierarchy<T1, Unit>;
    using RightBase = GenScatterHierarchy<TypeList<T2, T3...>, Unit>;
};

template<class AtomicType, template<class> class Unit>
class GenScatterHierarchy<TypeList<AtomicType>, Unit> : public Unit<AtomicType> {
public:
    using LeftBase = Unit<AtomicType>;
};

template<template<class> class Unit>
class GenScatterHierarchy<NullType, Unit> {};


template<class TList, template<class AtomicType, class base> class Unit, class Root = NullType>
class GenLinearHierarchy;

template<class T1, class T2, class... T3, template<class, class> class Unit, class Root>
class GenLinearHierarchy<TypeList<T1, T2, T3...>, Unit, Root> : public Unit<T1, GenLinearHierarchy<TypeList<T2, T3...>, Unit, Root>> {};

template<class AtomicType, template<class, class> class Unit, class Root>
class GenLinearHierarchy<TypeList<AtomicType>, Unit, Root> : public Unit<AtomicType, Root> {};
