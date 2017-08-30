#pragma once 
#include "CoreMinimal.h"

namespace k3d
{
template <class T>
class K3D_CORE_API Singleton
{
public:
    static T & Get()
    {
      static T instance;
      return instance;
    }
	
protected:
    Singleton(){}
};
}