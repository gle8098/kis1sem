#include <iostream>

#include "hierarchy.hpp"

struct Infantry {
    Infantry() {
        std::cout << "Infantry created" << std::endl;
    }
    ~Infantry() {
        std::cout << "Infantry destroyed" << std::endl;
    }
};
struct Archer {
    Archer() {
        std::cout << "Archer created" << std::endl;
    }
    ~Archer() {
        std::cout << "Archer destroyed" << std::endl;
    }
};
struct Cavalry {
    Cavalry() {
        std::cout << "Cavalry created" << std::endl;
    }
    ~Cavalry() {
        std::cout << "Cavalry destroyed" << std::endl;
    }
};


//////////////////////////////////////////////////////


template<class ConcreteT>
struct IAbstractFactoryUnit {
public:
    virtual ~IAbstractFactoryUnit() = default;

protected:
    virtual ConcreteT *create0(ConcreteT*) = 0;
};

template <class... T>
struct IArmyFactory : public GenScatterHierarchy<
        TypeList<T...>,
        IAbstractFactoryUnit> {
    using IAbstractFactoryUnit<T>::create0...;

    template<class U>
    U *create() {
        return create0((U*)nullptr);
    }
};

template<template<class, class> class FactoryUnit, class... T>
struct CArmyFactory : public GenLinearHierarchy<
        TypeList<T...>,
        FactoryUnit,
        IArmyFactory<T...>> {};


//////////////////////////////////////////////////////


template<class ConcreteProduct, class Base>
struct CFactoryUnit : public Base {
protected:
    ConcreteProduct *create0(ConcreteProduct*) override {
        return new ConcreteProduct;
    }
};

int main() {
    CArmyFactory<CFactoryUnit, Infantry, Archer, Cavalry> h;
    IArmyFactory<Infantry, Archer, Cavalry>& hp = h;
    auto t = hp.create<Archer>();
    delete t;

    return 0;
}
