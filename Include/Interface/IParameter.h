#pragma once
#include "IShader.h"

struct IParameterManager
{
	virtual void SetVector4(const char * paramName, void *vector4) = 0;
};