#pragma once
#ifndef __kTypeTraits_hpp__
#define __kTypeTraits_hpp__

#define LINK2(a,b) a##_##b
#define LINK3(a,b,c) a##_##b##_##c

#define KTYPE_META_TEMPLATE(ClassName) \
  template <typename T>\
struct LINK2( is, ClassName ) \
{\
enum LINK3( is, ClassName, enum ) \
{\
  _YES = 0, \
  _NO = 1\
}; \
}

#define KTYPE_ISTYPE_TEMPLATE(T, ClassName)	\
  template <>\
struct LINK2( is, ClassName ) <T>\
{							\
enum LINK3( is, ClassName, enum )\
{\
  _YES = 1,\
  _NO = 0\
};\
}

KTYPE_META_TEMPLATE( float );
KTYPE_ISTYPE_TEMPLATE( float, float );
KTYPE_ISTYPE_TEMPLATE( double, float );
KTYPE_ISTYPE_TEMPLATE( long double, float );

template <typename T>
struct kType
{
  enum type_enum
  {
    is_float = is_float<T>::_YES,
  };
};

#endif