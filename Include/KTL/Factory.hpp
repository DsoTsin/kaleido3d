#ifndef __Factory_hpp__
#define __Factory_hpp__
#pragma once
#include <assert.h>

template <class Type> class FactoryBase {
protected:
  explicit FactoryBase(int type);
  virtual ~FactoryBase();

  virtual Type *Create() const = 0;
  template <class Type> friend class Factory ;
private:
  int m_Type;
};

template <class Base, class Type> class FactoryCreator : public FactoryBase<Base> {
public:
  explicit FactoryCreator(int type) : FactoryBase<Base>(type) {}
  virtual ~FactoryCreator() {}

  virtual Base *Create() const {
    return new Type;
  }
};

template <class Type> class Factory {
protected:
  enum {
    TYPE_MAX = 256
  };

  Factory() {}
  ~Factory() {}

  static Type * Create(int type) {
    assert(type >= 0 && type < TYPE_MAX && "Factory::Create: not supported type!");
    assert(m_Creators[type] == 0 && "Factory::addCreator(): creator is already defined ! ");
    return m_Creators[type]->Create();
  }

private:

    template <class Class> friend class FactoryBase;

    static void addCreator(int type, FactoryBase<Type> *creator) {
        assert(type >= 0 && type < TYPE_MAX && "Factory::addCreator(): not supported type !");
        assert(m_Creators[type] == 0 && "Factory::addCreator(): creator is already defined !");
        m_Creators[type] = creator;
    }

    static void removeCreator(int type) {
        assert(type >= 0 && type < TYPE_MAX && "Factory::removeCreator(): not supported type !");
        assert(m_Creators[type] != 0 && "Factory::removeCreator(): creator is NULL ! ");
        m_Creators[type] = 0;
    }

    static FactoryBase<Type> *m_Creators[TYPE_MAX];
};

template <class Type> FactoryBase<Type> *Factory<Type>::m_Creators[Factory<Type>::TYPE_MAX] = { 0, };

template <class Type>
FactoryBase<Type>::FactoryBase(int type) : m_Type(type) {
    Factory<Type>::addCreator(type,this);
}

template <class Type>
FactoryBase<Type>::~FactoryBase() {
    Factory<Type>::removeCreator(m_Type);
}
#endif
