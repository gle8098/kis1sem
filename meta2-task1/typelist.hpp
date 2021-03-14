#pragma once

/* TypeList */

class NullType {};

template<typename T=NullType, typename ... U>
struct TypeList {
    using head = T;
    using tail = TypeList<U ...>;
};

template<typename T, typename Head, typename Tail>
struct TypeList<T, TypeList<Head, Tail>> {
    using head = T;
    using tail = TypeList<Head, Tail>;
};

template<typename T, typename Head>
struct TypeList<T, TypeList<Head>> {
    using head = T;
    using tail = TypeList<Head>;
};

template<typename T>
struct TypeList<T, TypeList<>> {
    using head = T;
    using tail = NullType;
};

template<typename T>
struct TypeList<T> {
    using head = T;
    using tail = NullType;
};

template<typename Head, typename Tail>
struct TypeList<TypeList<Head, Tail>> {
    using head = Head;
    using tail = Tail;
};

template<typename Head>
struct TypeList<TypeList<Head>> {
    using head = Head;
    using tail = NullType;
};

using EmptyList = TypeList<>;


/* Length */

template <typename TypeList>
struct Length {
    enum { value = Length<typename TypeList::tail>::value + 1 };
};

template <>
struct Length<NullType> {
    enum { value = 0 };
};

template <>
struct Length<TypeList<NullType>> {
    enum { value = 0 };
};


/* TypeAt */

template<class T, unsigned int i>
struct TypeAt {
    using Result = typename TypeAt<typename T::tail, i - 1>::Result;
};

template<class T>
struct TypeAt<T, 0> {
    using Result = typename T::head;
};


/* Append */

template<class TList, class T>
struct Append {
    using Result = TypeList<typename TList::head, typename Append<typename TList::tail, T>::Result>;
};

template<>
struct Append<NullType, NullType> {
    using Result = NullType;
};

template<class T>
struct Append<NullType, T> {
    using Result = TypeList<T>;
};

template<class T>
struct Append<EmptyList, T> {
    using Result = TypeList<T>;
};

template<class Head, class Tail>
struct Append<NullType, TypeList<Head, Tail>> {
    using Result = TypeList<Head, Tail>;
};


/* Erase */

template<class TList, class T>
struct Erase;

template<typename T>
struct Erase<NullType, T> {
    using Result = NullType;
};

template<class TList, class T>
struct Erase {
    using Result = TypeList<typename TList::head, typename Erase<typename TList::tail, T>::Result>;
};

template<class... Tail, class T>
struct Erase<TypeList<T, Tail...>, T> {
    using Result = TypeList<Tail...>;
};

template<class T>
struct Erase<TypeList<T>, T> {
    using Result = TypeList<>;
};
