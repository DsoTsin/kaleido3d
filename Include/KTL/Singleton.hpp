#pragma once 
#include <Kaleido3D.h>

K3D_COMMON_NS
{
template <class T>
class K3D_API Singleton
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