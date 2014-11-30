#pragma once
#ifndef __Function_hpp__
#define __Function_hpp__
#include <Config/Prerequisities.h>

enum class FunctionType : uint32_t {
	Vertex = 0x8B31,
	Geometry = 0x8DD9, 
	Fragment = 0x8B30,
	TessEval = 0x8E88, 
	TessCtl = 0x8E87,
	Compute = 0x91B9
};

class Function {
public:

	FunctionType functionType;
	
	std::string source;

	uint32_t handle;

	Function();
	~Function();
	
	bool compileShader();
};

#endif