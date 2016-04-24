#pragma once

#define KOBJECT_PROPERTY_GET(Property, Type) \
  Type Get##Property() const

#define KOBJECT_PROPERTY_SET(Property, Type) \
  void Set##Property( const Type & _Property ) 

#define KOBJECT_PROPERTY(Property, Type) \
  KOBJECT_PROPERTY_SET(Property, Type);\
  KOBJECT_PROPERTY_GET(Property, Type)

#define KOBJECT_CLASSNAME(className) \
  static const char * ClassName() {\
    static const char * name = #className ;\
    return name;\
  }