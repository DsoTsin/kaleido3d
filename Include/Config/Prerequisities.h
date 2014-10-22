#pragma once
#ifndef __Prerequisities_H__
#define __Prerequisities_H__

#include "Config.h"

#include "Types.h"
#include <iostream>
#include <vector>
#include <list>
#include <thread>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>
#include <initializer_list>
#include <functional>
#include <cassert>

typedef std::string k3dString;

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
#endif
