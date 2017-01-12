#pragma once 
#include <Kaleido3D.h>

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
